#pragma once

#include "scenefiles.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gpu {

enum class CameraType : std::uint32_t {
  PERSPECTIVE = 0,
  ORTHOGONAL = 1
};

enum class PigmentType : std::uint32_t {
  UNIFORM = 0,
  CHECKERED = 1
};

enum class BRDFType : std::uint32_t {
  DIFFUSIVE = 0,
  SPECULAR = 1
};

/// @brief Affine transform packed as three rows of four floats.
/// @details Interpreted as a 3x3 linear part plus a translation column.
struct alignas(16) Affine3x4 {
  float m[3][4];
};

/// @brief GPU-facing sphere record for flattened scene storage.
struct alignas(16) Sphere {
  Affine3x4 local_to_world;
  Affine3x4 world_to_local;
  std::uint32_t material_idx;
  std::uint32_t _padding0;
  std::uint32_t _padding1;
  std::uint32_t _padding2;
};

/// @brief GPU-facing plane record for flattened scene storage.
struct alignas(16) Plane {
  Affine3x4 local_to_world;
  Affine3x4 world_to_local;
  std::uint32_t material_idx;
  std::uint32_t _padding0;
  std::uint32_t _padding1;
  std::uint32_t _padding2;
};

/// @brief GPU-facing camera record.
struct alignas(16) Camera {
  Affine3x4 camera_to_world;
  float distance;
  float asp_ratio;
  CameraType camera_type;
  std::uint32_t _padding0;
};

/// @brief Inline pigment payload stored directly inside a material record.
/// @details Image pigments should reference external texture data through resource_idx.
/// @details Checkered pigments use int_parameter as n_intervals and the two color fields as payload.
struct alignas(16) InlinePigment {
  PigmentType pigment_type;
  std::uint32_t resource_idx;
  std::int32_t int_parameter;
  std::uint32_t _padding0;
  float color1[4];
  float color2[4];
};

/// @brief GPU-facing material record.
struct alignas(16) Material {
  BRDFType brdf_type;
  std::uint32_t _padding0;
  std::uint32_t _padding1;
  std::uint32_t _padding2;

  InlinePigment brdf_pigment;
  InlinePigment emitted_pigment;
};

/// @brief Rendering configuration uploaded alongside the scene.
struct alignas(16) RenderConfig {
  std::uint32_t width;
  std::uint32_t height;
  std::uint32_t samples_per_pixel;
  std::uint32_t max_depth;

  std::uint32_t russian_roulette_lim;
  std::uint32_t frame_idx; // Useful later to decorrelate RNG across progressive accumulation passes.
  std::uint32_t _padding0;
  std::uint32_t _padding1;
};

/// @brief Heap-owned flattened scene ready to be uploaded to GPU buffers.
struct Scene {
  struct SceneCounts {
    std::uint32_t spheres; 
    std::uint32_t planes;
    std::uint32_t materials;
    std::uint32_t _padding0;
  };

  std::vector<Sphere> spheres;
  std::vector<Plane> planes;
  std::vector<Material> materials;
  SceneCounts counts;
  Camera camera;
};

inline Material lower_material(const ::Material &material);

inline Camera lower_camera(const ::Camera &camera);

inline Sphere lower_sphere(const ::Sphere &sphere,
                           const std::unordered_map<const ::Material *, std::uint32_t> &material_to_idx);

inline Plane lower_plane(const ::Plane &plane,
                         const std::unordered_map<const ::Material *, std::uint32_t> &material_to_idx);

inline Affine3x4 lower_transformation(const ::Transformation &transformation);

inline InlinePigment lower_pigment(const ::Pigment &pigment);

/// @brief Lower a parsed CPU-side scene to the flattened GPU representation.
inline Scene lower_scene(const ::Scene &scene) {
  std::vector<Material> lowered_materials;
  std::unordered_map<const ::Material *, std::uint32_t> material_to_idx;

  lowered_materials.reserve(scene.materials.size());
  material_to_idx.reserve(scene.materials.size());

  for (const auto &[name, material] : scene.materials) {
    (void)name;
    const std::uint32_t idx = static_cast<std::uint32_t>(lowered_materials.size());
    lowered_materials.push_back(lower_material(material));
    material_to_idx.emplace(&material, idx);
  }

  if (!scene.camera) {
    throw std::runtime_error("Got null camera while lowering to GPU backend");
  }
  Camera lowered_camera = lower_camera(*scene.camera);

  std::vector<Sphere> lowered_spheres;
  std::vector<Plane> lowered_planes;

  for (const auto &object_ptr : scene.world.objects) {
    if (const auto *sphere_ptr = dynamic_cast<const ::Sphere *>(object_ptr.get())) {
      lowered_spheres.push_back(lower_sphere(*sphere_ptr, material_to_idx));
    } else if (const auto *plane_ptr = dynamic_cast<const ::Plane *>(object_ptr.get())) {
      lowered_planes.push_back(lower_plane(*plane_ptr, material_to_idx));
    } else {
      throw std::runtime_error("Got unsupported shape while lowering to GPU backend");
    }
  }

  // TODO guard these narrowing conversions explicitly if we ever want to support
  // scenes large enough to exceed the uint32_t GPU ABI counts.
  Scene::SceneCounts lowered_counts{
    .spheres = static_cast<std::uint32_t>(lowered_spheres.size()),
    .planes = static_cast<std::uint32_t>(lowered_planes.size()),
    .materials = static_cast<std::uint32_t>(lowered_materials.size()),
    ._padding0 = 0u,
  };

  return Scene{
    .spheres = std::move(lowered_spheres),
    .planes = std::move(lowered_planes),
    .materials = std::move(lowered_materials),
    .counts = lowered_counts,
    .camera = lowered_camera,
  };
}

inline Material lower_material(const ::Material &material) {
  const InlinePigment brdf_pigment = lower_pigment(*material.brdf->pigment);
  const InlinePigment emitted_pigment = lower_pigment(*material.emitted_radiance);

  BRDFType brdf_type;
  if (dynamic_cast<const DiffusiveBRDF *>(material.brdf.get())) {
    brdf_type = BRDFType::DIFFUSIVE;
  } else if (dynamic_cast<const SpecularBRDF *>(material.brdf.get())) {
    brdf_type = BRDFType::SPECULAR;
  } else {
    throw std::runtime_error("Got unsupported BRDF type while lowering to GPU backend");
  }

  Material lowered{};
  lowered.brdf_type = brdf_type;
  lowered.brdf_pigment = brdf_pigment;
  lowered.emitted_pigment = emitted_pigment;
  return lowered;
}

inline Camera lower_camera(const ::Camera &camera) {
  if (!camera.asp_ratio.has_value()) {
    throw std::runtime_error("Got camera with undefined aspect ratio while lowering to GPU backend");
  }

  Camera lowered{};
  lowered.camera_to_world = lower_transformation(camera.transformation);
  lowered.asp_ratio = *camera.asp_ratio;

  if (const auto *camera_ptr = dynamic_cast<const PerspectiveCamera *>(&camera)) {
    lowered.camera_type = CameraType::PERSPECTIVE;
    lowered.distance = camera_ptr->distance;
  } else if (dynamic_cast<const OrthogonalCamera *>(&camera)) {
    lowered.camera_type = CameraType::ORTHOGONAL;
  } else {
    throw std::runtime_error("Got unknown camera type while lowering to GPU backend");
  }

  return lowered;
}

inline Sphere lower_sphere(const ::Sphere &sphere,
                           const std::unordered_map<const ::Material *, std::uint32_t> &material_to_idx) {
  const auto it = material_to_idx.find(&sphere.material);
  if (it == material_to_idx.end()) {
    throw std::runtime_error("Got unknown material while lowering sphere to GPU backend");
  }

  Sphere lowered{};
  lowered.local_to_world = lower_transformation(sphere.transformation);
  lowered.world_to_local = lower_transformation(sphere.transformation.inverse());
  lowered.material_idx = it->second;
  return lowered;
}

inline Plane lower_plane(const ::Plane &plane,
                         const std::unordered_map<const ::Material *, std::uint32_t> &material_to_idx) {
  const auto it = material_to_idx.find(&plane.material);
  if (it == material_to_idx.end()) {
    throw std::runtime_error("Got unknown material while lowering plane to GPU backend");
  }

  Plane lowered{};
  lowered.local_to_world = lower_transformation(plane.transformation);
  lowered.world_to_local = lower_transformation(plane.transformation.inverse());
  lowered.material_idx = it->second;
  return lowered;
}

inline Affine3x4 lower_transformation(const ::Transformation &transformation) {
  Affine3x4 lowered{};

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      lowered.m[i][j] = transformation.hom_matrix.linear_part[i][j];
    }
  }

  lowered.m[0][3] = transformation.hom_matrix.translation_vec.x;
  lowered.m[1][3] = transformation.hom_matrix.translation_vec.y;
  lowered.m[2][3] = transformation.hom_matrix.translation_vec.z;

  return lowered;
}

inline InlinePigment lower_pigment(const ::Pigment &pigment) {
  InlinePigment lowered{};

  if (const auto *pigment_ptr = dynamic_cast<const UniformPigment *>(&pigment)) {
    lowered.pigment_type = PigmentType::UNIFORM;
    lowered.color1[0] = pigment_ptr->color.r;
    lowered.color1[1] = pigment_ptr->color.g;
    lowered.color1[2] = pigment_ptr->color.b;
    lowered.color1[3] = 1.f;
  } else if (const auto *pigment_ptr = dynamic_cast<const CheckeredPigment *>(&pigment)) {
    lowered.pigment_type = PigmentType::CHECKERED;
    lowered.int_parameter = pigment_ptr->n_intervals;

    lowered.color1[0] = pigment_ptr->color1.r;
    lowered.color1[1] = pigment_ptr->color1.g;
    lowered.color1[2] = pigment_ptr->color1.b;
    lowered.color1[3] = 1.f;

    lowered.color2[0] = pigment_ptr->color2.r;
    lowered.color2[1] = pigment_ptr->color2.g;
    lowered.color2[2] = pigment_ptr->color2.b;
    lowered.color2[3] = 1.f;
  } else {
    throw std::runtime_error("Got unsupported pigment type while lowering to GPU backend");
  }

  return lowered;
}

static_assert(std::is_standard_layout_v<CameraType>);
static_assert(std::is_trivially_copyable_v<CameraType>);
static_assert(sizeof(CameraType) == 4);
static_assert(alignof(CameraType) == 4);

static_assert(std::is_standard_layout_v<PigmentType>);
static_assert(std::is_trivially_copyable_v<PigmentType>);
static_assert(sizeof(PigmentType) == 4);
static_assert(alignof(PigmentType) == 4);

static_assert(std::is_standard_layout_v<BRDFType>);
static_assert(std::is_trivially_copyable_v<BRDFType>);
static_assert(sizeof(BRDFType) == 4);
static_assert(alignof(BRDFType) == 4);

static_assert(std::is_standard_layout_v<Affine3x4>);
static_assert(std::is_trivially_copyable_v<Affine3x4>);
static_assert(sizeof(Affine3x4) == 48);
static_assert(alignof(Affine3x4) == 16);

static_assert(std::is_standard_layout_v<Sphere>);
static_assert(std::is_trivially_copyable_v<Sphere>);
static_assert(sizeof(Sphere) == 112);
static_assert(alignof(Sphere) == 16);
static_assert(offsetof(Sphere, local_to_world) == 0);
static_assert(offsetof(Sphere, world_to_local) == 48);
static_assert(offsetof(Sphere, material_idx) == 96);

static_assert(std::is_standard_layout_v<Plane>);
static_assert(std::is_trivially_copyable_v<Plane>);
static_assert(sizeof(Plane) == 112);
static_assert(alignof(Plane) == 16);
static_assert(offsetof(Plane, local_to_world) == 0);
static_assert(offsetof(Plane, world_to_local) == 48);
static_assert(offsetof(Plane, material_idx) == 96);

static_assert(std::is_standard_layout_v<Camera>);
static_assert(std::is_trivially_copyable_v<Camera>);
static_assert(sizeof(Camera) == 64);
static_assert(alignof(Camera) == 16);
static_assert(offsetof(Camera, camera_to_world) == 0);
static_assert(offsetof(Camera, distance) == 48);

static_assert(std::is_standard_layout_v<InlinePigment>);
static_assert(std::is_trivially_copyable_v<InlinePigment>);
static_assert(sizeof(InlinePigment) == 48);
static_assert(alignof(InlinePigment) == 16);
static_assert(offsetof(InlinePigment, pigment_type) == 0);
static_assert(offsetof(InlinePigment, color1) == 16);
static_assert(offsetof(InlinePigment, color2) == 32);

static_assert(std::is_standard_layout_v<Material>);
static_assert(std::is_trivially_copyable_v<Material>);
static_assert(sizeof(Material) == 112);
static_assert(alignof(Material) == 16);
static_assert(offsetof(Material, brdf_type) == 0);
static_assert(offsetof(Material, brdf_pigment) == 16);
static_assert(offsetof(Material, emitted_pigment) == 64);

static_assert(std::is_standard_layout_v<RenderConfig>);
static_assert(std::is_trivially_copyable_v<RenderConfig>);
static_assert(sizeof(RenderConfig) == 32);
static_assert(alignof(RenderConfig) == 16);

} // namespace gpu
