#include <metal_stdlib>
#include "geometry.metalh"
#include "random.metalh"
#include "shapes.metalh"
#include "materials.metalh"
#include "cameras.metalh"
using namespace metal;

struct SceneCounts {
  uint spheres;
  uint planes;
  uint materials;
  uint _padding0;
};

struct OnOffCfg {
  uint _padding0;
  uint _padding1;
  uint _padding2;
  uint _padding3;
};

struct FlatCfg {
  float4 background_color;
};

struct PathCfg {
  uint samples_per_pixel;
  uint max_depth;
  uint russian_roulette_limit;
  uint _padding0;

  ulong random_seed;
  uint _padding1;
  uint _padding2;

  float4 background_color;
};

HitRecord world_intersect_ray(Ray ray,
                              device const Sphere *spheres,
                              device const Plane *planes, 
                              constant SceneCounts *scene_counts) {
  float spheres_min_t = INFINITY;
  float planes_min_t = INFINITY;
  uint spheres_min_idx;
  uint planes_min_idx;

  for (uint i = 0; i < scene_counts->spheres; ++i) {
    float t = intersect_sphere_t(spheres[i].world_to_local, ray);
    if (t < spheres_min_t) {
      spheres_min_t = t;
      spheres_min_idx = i;
    }
  }

  for (uint i = 0; i < scene_counts->planes; ++i) {
    float t = intersect_plane_t(planes[i].world_to_local, ray);
    if (t < planes_min_t) {
      planes_min_t = t;
      planes_min_idx = i;
    }
  }

  if (spheres_min_t == INFINITY && planes_min_t == INFINITY) {
    HitRecord hit;
    hit.t = INFINITY;
    hit.point = float3(INFINITY, INFINITY, INFINITY);
    hit.normal = float3(INFINITY, INFINITY, INFINITY);
    hit.uv = SurfaceCoords{INFINITY, INFINITY};
    hit.material_idx = uint(-1);
    return hit;
  }

  bool sphere_wins = spheres_min_t < planes_min_t;
  float t = sphere_wins ? spheres_min_t : planes_min_t;
  uint idx = sphere_wins ? spheres_min_idx : planes_min_idx;
  ShapeType shape_type = sphere_wins ? ShapeType::SPHERE : ShapeType::PLANE;
  Affine3x4 shape_world_to_local{};
  uint material_idx{};

  switch (shape_type) {
  case ShapeType::SPHERE:
    shape_world_to_local = spheres[idx].world_to_local;
    material_idx = spheres[idx].material_idx;
    break;
  case ShapeType::PLANE:
    shape_world_to_local = planes[idx].world_to_local;
    material_idx = planes[idx].material_idx;
    break;
  }

  return build_intersection(ray, t, shape_type, shape_world_to_local, material_idx);
}

kernel void on_off_tracing(texture2d<float, access::write> output [[texture(0)]],
                           device const Sphere *spheres [[buffer(0)]],
                           device const Plane *planes [[buffer(1)]],
                           device const Material *materials [[buffer(2)]],
                           constant SceneCounts *scene_counts [[buffer(3)]], 
                           constant Camera *camera [[buffer(4)]],
                           device const OnOffCfg *cfg [[buffer(5)]],
                           uint2 gid [[thread_position_in_grid]]) {

  (void)cfg;

  uint width = output.get_width();
  uint height = output.get_height();
  if (gid.x >= width || gid.y >= height) {
    return;
  }
  
  Ray ray = cast_first_ray(*camera, gid.x, gid.y, width, height);
  float t;

  for (uint i = 0; i < scene_counts->spheres; ++i) {
    t = intersect_sphere_t(spheres[i].world_to_local, ray);
    if (t < INFINITY) {
      output.write(float4(1.f, 1.f, 1.f, 1.f), gid);
      return;
    }
  }

  for (uint i = 0; i < scene_counts->planes; ++i) {
    t = intersect_plane_t(planes[i].world_to_local, ray);
    if (t < INFINITY) {
      output.write(float4(1.f, 1.f, 1.f, 1.f), gid);
      return;
    }
  }

  output.write(float4(0.f, 0.f, 0.f, 1.f), gid);
}

kernel void flat_tracing(texture2d<float, access::write> output [[texture(0)]],
                         device const Sphere *spheres [[buffer(0)]],
                         device const Plane *planes [[buffer(1)]],
                         device const Material *materials [[buffer(2)]],
                         constant SceneCounts *scene_counts [[buffer(3)]], 
                         constant Camera *camera [[buffer(4)]],
                         device const FlatCfg *cfg [[buffer(5)]],
                         uint2 gid [[thread_position_in_grid]]) {

  uint width = output.get_width();
  uint height = output.get_height();
  if (gid.x >= width || gid.y >= height) {
    return;
  }
  
  Ray ray = cast_first_ray(*camera, gid.x, gid.y, width, height);
  HitRecord hit = world_intersect_ray(ray, spheres, planes, scene_counts);

  if (hit.t == INFINITY) {
    output.write(cfg->background_color, gid);
    return;
  }

  InlinePigment brdf_pigment = materials[hit.material_idx].brdf_pigment;
  InlinePigment emitted_pigment = materials[hit.material_idx].emitted_pigment;
  float3 color = pigment_eval(brdf_pigment, hit.uv) + pigment_eval(emitted_pigment, hit.uv);
  output.write(float4(color, 1.f), gid);
}


kernel void path_tracing(texture2d<float, access::write> output [[texture(0)]],
                         device const Sphere *spheres [[buffer(0)]],
                         device const Plane *planes [[buffer(1)]],
                         device const Material *materials [[buffer(2)]],
                         constant SceneCounts *scene_counts [[buffer(3)]], 
                         constant Camera *camera [[buffer(4)]],
                         device const PathCfg *cfg [[buffer(5)]],
                         uint2 gid [[thread_position_in_grid]]) {

  uint width = output.get_width();
  uint height = output.get_height();
  if (gid.x >= width || gid.y >= height) {
    return;
  }

  // Initialize random number generator
  PCGState pcg;
  ulong seq_number = ((ulong)gid.x << 32) | (ulong)gid.y; 
  pcg_init(pcg, cfg->random_seed, seq_number);
  
  float3 acc_radiance = float3(0.f, 0.f, 0.f);

  for (uint sample = 0; sample < cfg->samples_per_pixel; ++sample) {
    
    // Cast first ray
    Ray ray = cast_first_ray(*camera, gid.x, gid.y, width, height);

    float3 sample_radiance = float3(0.f, 0.f, 0.f);
    float3 sample_factor = float3(1.f, 1.f, 1.f);

    // Bouncing loop
    while (true) {
      
      HitRecord hit = world_intersect_ray(ray, spheres, planes, scene_counts);

      if (hit.t == INFINITY) {
        sample_radiance += sample_factor * cfg->background_color.xyz;
        break;
      }

      InlinePigment brdf_pigment = materials[hit.material_idx].brdf_pigment;
      InlinePigment emitted_pigment = materials[hit.material_idx].emitted_pigment;
      sample_radiance += sample_factor * pigment_eval(emitted_pigment, hit.uv);
      float3 reflected_color = pigment_eval(brdf_pigment, hit.uv);
      sample_factor *= reflected_color; 
      
      float hit_lum = max(max(reflected_color.x, reflected_color.y), reflected_color.z);
      if (ray.depth >= cfg->max_depth || hit_lum <= 0.f) {
        break;
      }

      if (ray.depth > cfg->russian_roulette_limit) {
        float q = max(1.f - hit_lum, 0.05f);
        if (pcg_random_float(pcg) > q) {
          sample_factor *= 1.f / (1.f - q);          
        } else {
          break;
        }
      }

      ray = brdf_scatter_ray(materials[hit.material_idx].brdf_type,
                             pcg, ray.direction, hit.point, hit.normal, ray.depth + 1);
    }
    
    acc_radiance += 1.f / cfg->samples_per_pixel * sample_radiance;
  }

  output.write(float4(acc_radiance, 1.f), gid);
}
