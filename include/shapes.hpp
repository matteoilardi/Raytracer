// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR SHAPES AND WORLD OBJECTS -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include "cameras.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "materials.hpp"

#include <cmath>
#include <memory>
#include <numbers>
#include <optional>
// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class HitRecord;
class Object;
class Shape;
class Sphere;
class Plane;
class PointLightSource;
class World;


//-------------------------------------------------------------------------------------------------------------
// --------------UTILS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Merges two ordered arrays into one, preserving order
template<typename T, typename Compare>
std::vector<T> merge_ordered_vectors(const std::vector<T> &v1, const std::vector<T> &v2, Compare cmp) noexcept {
  std::vector<T> result;
  result.reserve(v1.size() + v2.size());

  auto it1 = v1.begin();
  auto it2 = v2.begin();

  while (it1 != v1.end() && it2 != v2.end()) {
    if (cmp(*it1, *it2)) result.push_back(*it1++);
    else result.push_back(*it2++);
  }

  result.insert(result.end(), it1, v1.end());
  result.insert(result.end(), it2, v2.end());

  return result;
}

//-------------------------------------------------------------------------------------------------------------
// -----------HIT RECORD CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Store information about the intersection of a ray with an object
class HitRecord {
public:
  //-------Properties--------

  const Shape *shape;  // shape that was hit (required in order to trace back to the material that was hit)
  Point world_point;   // 3D coordinates of the intersection point in real world
  Normal normal;       // normal to the surface at the intersection point
  Vec2d surface_point; // 2D coordinates on the surface
  Ray ray;             // the ray that actually hit the shape
  float t;             // distance from the origin of the ray to the intersection point

  //-----------Constructors-----------
  /// Default constructor
  HitRecord() : world_point{}, normal{}, surface_point{}, ray{}, t{0.f} {};

  /// Constructor with parameters
  HitRecord(const Shape *shape, Point world_point, Normal normal, Vec2d surface_point, const Ray &ray, float t) noexcept
      : shape{shape}, world_point{world_point}, normal{normal}, surface_point{surface_point}, ray{ray}, t{t} {}

  //------------Methods-----------
  bool is_close(const HitRecord &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
    return (this->shape == other.shape) && world_point.is_close(other.world_point, error_tolerance) &&
           normal.is_close(other.normal, error_tolerance) && surface_point.is_close(other.surface_point, error_tolerance) &&
           ray.is_close(other.ray, error_tolerance) && are_close(t, other.t, error_tolerance);
  }

  void transform_in_place(const Transformation &transformation) noexcept { 
    world_point = transformation * world_point;
    normal = transformation * normal;
    ray = ray.transform(transformation); 
  }
};


// ------------------------------------------------------------------------------------------------------------
// -----------OBJECT CLASS (abstract) ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Object class is the base class for all shapes in the scene
class Object {
public:
  
  //-------Properties--------
  ///@brief transformation describing the actual position of the shape in the world reference frame
  Transformation transformation;

  //-----------Constructors-----------
  /// @brief Constructor with parameters
  /// @param tranformation to apply to the standard shape 
  /// @param material properties of the shape (pigment and brdf) as a function of (u, v)
  Object(const Transformation &transformation) : transformation{transformation} {}

  virtual ~Object() {}

  //--------------------Methods----------------------
  /// @brief Finds the closest intersection of a given ray with the shape
  /// @param incoming ray (possibly) hitting the shape
  /// @details This method is required to calculate and return (inside HitRecord) a normalized normal
  virtual std::optional<HitRecord> ray_intersection(const Ray &ray_world_frame) const noexcept = 0;

  /// @brief Finds all intersections of a given ray with the shape, returns vector ordered by increasing t
  /// @param incoming ray (possibly) hitting the shape
  virtual std::vector<HitRecord> all_ray_intersections(const Ray &ray_world_frame) const noexcept = 0; 

  /// @brief Check if a point lies inside the shape
  virtual bool is_point_inside(Point point_world_frame) const noexcept = 0;

};

// ------------------------------------------------------------------------------------------------------------
// -----------SHAPE CLASS (abstract) ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Base class for geometrical objects
class Shape : public Object {
public:
  //-------Properties--------
  ///@brief properties (pigment and brdf) of the shape as a function of (u, v)
  const Material &material;

  //-----------Constructors-----------

  /// @brief Constructor with parameters
  /// @param tranformation to apply to the standard shape 
  /// @param material 
  Shape(const Transformation &transformation, const Material &material) : Object{transformation}, material{material} {}

  virtual ~Shape() {}

  //--------------------Methods----------------------

protected:
  /// @brief Calculates normal to the *standard* shape at a given point in the standard shape's reference frame
  virtual Normal calculate_normal(Point point) const noexcept = 0;

  /// @brief Calculates (u, v) surface coordinates at a given point in the standard sphere's reference frame
  virtual Vec2d calculate_uv(Point point) const noexcept = 0;

  /// @brief Flip the normal to the surface so that it has negative scalar product with the hitting ray
  /// @param normal to the surface
  /// @param incoming ray hitting the shape
  static Normal enforce_correct_normal_orientation(Normal normal, const Ray &ray) noexcept {
    // The normal and the ray must have opposite directions, if this is false (dot product>0), flip the normal
    float sign = std::copysignf(1.0f, -(normal * ray.direction));
    return normal * sign;
  }

  /// @brief Build HitRecord given a solution to ray-sphere intersection equation
  /// @param ray in the sphere's reference frame
  /// @param ray's t at intersection
  /// @param ray in the world's reference frame
  HitRecord make_hit(const Ray &ray, float t, const Ray& ray_world_frame) const noexcept {
    // Find the corresponding hitting point in the *standard* sphere's reference frame
    Point hit_point = ray.at(t);
    
    // Compute the normal to the surface at the intersection point in the *standard* sphere's reference frame
    Normal normal = calculate_normal(hit_point);
    normal = enforce_correct_normal_orientation(normal, ray);

    // Compute the 2D surface coordinates of the intersection point (same in both reference frames)
    Vec2d uv = calculate_uv(hit_point);

    // Transform the intersection point parameters back to the world's reference frame
    return HitRecord{this, transformation*hit_point, transformation*normal, uv, ray_world_frame, t};
  }
};

//-------------------------------------------------------------------------------------------------------------
// -----------SPHERE CLASS ------------------
// ------------------------------------------------------------------------------------------------------------

class Sphere : public Shape {
public:
  //-------Properties--------
  // By default, the sphere is centered at the origin and has a radius of 1 so we need no properties here, all the info is
  // specified in the transformation (that can take to an ellipsoid centered anywhere)

  //-----------Constructors-----------

  /// Constructor with parameters
  Sphere(const Transformation &transformation, const Material &material) : Shape{transformation, material} {};

  //--------------------Methods----------------------

  std::optional<HitRecord> ray_intersection(const Ray &ray_world_frame) const noexcept override {
    //  Important note: unless otherwise specified, every geometrical object in the body of this method is in the
    //  reference frame of the *standard* sphere

    // Transform the ray to the *standard* sphere's reference frame
    Ray ray = ray_world_frame.transform(transformation.inverse());

    auto [t1, t2] = solve_ray_sphere(ray);
    
    // Return if no hit is found
    if (!t1.has_value()) return std::nullopt;
    
    // Build HitRecord for the first hit
    return make_hit(ray, *t1, ray_world_frame);
  }

  std::vector<HitRecord> all_ray_intersections(const Ray &ray_world_frame) const noexcept override {
    //  Important note: unless otherwise specified, every geometrical object in the body of this method is in the
    //  reference frame of the *standard* sphere

    // Transform the ray to the *standard* sphere's reference frame
    Ray ray = ray_world_frame.transform(transformation.inverse());

    auto ts = solve_ray_sphere(ray);
   
    std::vector<HitRecord> valid_hits;
    
    // Build HitRecords for valid hits
    for (auto t : ts) {
      if (t && *t > ray.tmin && *t < ray.tmax) {
        valid_hits.push_back(make_hit(ray, *t, ray_world_frame));
      }
    }

    return valid_hits;
  }

  /// @brief Check if a point lies inside the shape
  bool is_point_inside(Point point_world_frame) const noexcept override {
    // Transform the ray to the *standard* sphere's reference frame
    Point point = transformation.inverse() * point_world_frame;

    return point.to_vector().squared_norm() < 1.f;
  }

private:
  /// @brief Calculates normal to the *standard* shape at a given point (in the standard shape's reference frame)
  Normal calculate_normal(Point point) const noexcept override {
    return Normal{point.x, point.y, point.z}; // The normal to the sphere is just the vector from the origin
  }

  /// @brief Calculate (u, v) coordinates of given point (i. e. spherical coordinates)
  Vec2d calculate_uv(Point point) const noexcept override {
    float u = atan2f(point.y, point.x) / (2.f * std::numbers::pi_v<float>); // atan2 is the arctangent
    if (u < 0.f) {
      u = u + 1.f;
    } // This is necessary in order to have v in range (0, 1] because the output of atan2 is in range (-pi, pi]
    float v = std::acos(point.z) / std::numbers::pi_v<float>;
    return Vec2d{u, v}; // We follow the convention (u, v) = (phi, theta)
  }

  /// @brief Calculate the t's corrseponding to valid ray intersections with the standard sphere (two at most)
  static std::array<std::optional<float>, 2> solve_ray_sphere(const Ray &ray) noexcept {
    
    // Compute the discriminant of the 2nd degree equation in t for ray-sphere intersection (cfr. slides 8b 29-31),
    // return {nullopt, mullopt} if the ray doesn't intersect the sphere.
    Vec O = ray.origin.to_vector();
    
    float a = ray.direction.squared_norm();
    float b = O * ray.direction;
    float c = O.squared_norm() - 1.f;

    float reduced_disc = b*b - a * c;
    if (reduced_disc <= 0.f) {
      return {std::nullopt, std::nullopt};
    }

    float t1 = (-b - std::sqrt(reduced_disc)) / a;
    float t2 = (-b + std::sqrt(reduced_disc)) / a;
    
    if (t1 > t2) {
      std::swap(t1, t2);
    }

    // Pick valid solutions and order them by increasing things (first intersection first)
    if (t1 < ray.tmin) {
      if (t2 > ray.tmin && t2 < ray.tmax) return {t2, std::nullopt};
      else return {std::nullopt, std::nullopt};
    }

    if (t2 < ray.tmax) return {t1, t2};
    else return {t1, std::nullopt};
  }

};

//-------------------------------------------------------------------------------------------------------------
// -----------PLANE CLASS ------------------
// ------------------------------------------------------------------------------------------------------------
class Plane : public Shape {
public:
  //-------Properties--------
  // By default the plane is at the origin and normal to the z axis, so we need no futher properties here, all the info is in the
  // transformation

  //-----------Constructors-----------

  /// @brief Constructor with parameters
  Plane(const Transformation &transformation, const Material &material) : Shape{transformation, material} {}

  //--------------------Methods----------------------

  std::optional<HitRecord> ray_intersection(const Ray &ray_world_frame) const noexcept override {
    // Important note: unless otherwise specified, every geometrical object in the body of this method is in the
    // reference frame of the *standard* plane

    // Transform the ray to the plane reference frame
    Ray ray = ray_world_frame.transform(transformation.inverse());

    // Return null if the ray is parallel to the plane
    if (are_close(ray.direction.z, 0.f)) {
      return std::nullopt;
    }

    // 3. Compute t at the intersection point and return null if t < 0, compute the intersection point
    float t_hit = -ray.origin.z / ray.direction.z;
    if (t_hit < ray.tmin || t_hit > ray.tmax) {
      return std::nullopt;
    }

    return make_hit(ray, t_hit, ray_world_frame);
  }


  std::vector<HitRecord> all_ray_intersections(const Ray &ray_world_frame) const noexcept override {
    std::vector<HitRecord> hits;
    auto hit = ray_intersection(ray_world_frame);

    if (hit.has_value()) hits.push_back(*hit);
    return hits;
  }

  /// @brief Check if a point lies inside the shape
  bool is_point_inside(Point point_world_frame) const noexcept override {
    // Transform the ray to the *standard* sphere's reference frame
    Point point = transformation.inverse() * point_world_frame;

    return point.z < 0.f; // By our convention, the lower half space is the interior of the plane
  }

private:
  /// @brief Calculates normal to the *standard* shape at a given point (in the standard shape's reference frame)
  Normal calculate_normal(Point point) const noexcept override { return VEC_Z.to_normal(); }

  /// @brief Calculate (u, v) coordinates of given point (periodic parametrization)
  Vec2d calculate_uv(Point point) const noexcept override {
    return Vec2d{point.x - std::floor(point.x), point.y - std::floor(point.y)};
  }

};


//-------------------------------------------------------------------------------------------------------------
// -----------CSGObject CLASS ------------------
// ------------------------------------------------------------------------------------------------------------

class CSGObject : public Object {
public:
  //-------Properties--------
  std::unique_ptr<Object> object1;
  std::unique_ptr<Object> object2;
  Transformation transformation;

  enum class Operation { UNION, INTERSECTION, DIFFERENCE, FUSION };
  Operation operation;

  //-------Constructors--------
  CSGObject() = delete;
  // A CSGObject should be built from ground up starting from Shapes in order to make sense. Deleting the default
  // constructor makes invalid states unrepresentable.

  CSGObject(std::unique_ptr<Object> object1, std::unique_ptr<Object> object2, Operation operation, const Transformation& transformation) noexcept :
    Object{transformation}, object1{std::move(object1)}, object2{std::move(object2)}, operation{operation}  {}

  //--------Methods---------
  /// @brief Finds the closest intersection of a given ray with the shape
  /// @param incoming ray (possibly) hitting the shape
  /// @details This method is required to calculate and return (inside HitRecord) a normalized normal
  std::optional<HitRecord> ray_intersection(const Ray &ray_world_frame) const noexcept override {
    auto valid_hits = all_ray_intersections(ray_world_frame); // Ordered by increasing t
    if (valid_hits.empty()) return std::nullopt;
    return valid_hits.front(); 
  }

  /// @brief Finds all intersections of a given ray with the shape
  /// @param incoming ray (possibly) hitting the shape
  std::vector<HitRecord> all_ray_intersections(const Ray &ray_world_frame) const noexcept override {
    // Transoform ray to the children objects' refrence frame 
    Ray ray = ray_world_frame.transform(transformation.inverse());

    // Calculate all intersections with child objects
    auto hits1 = object1->all_ray_intersections(ray);
    auto hits2 = object2->all_ray_intersections(ray);

    // Merge only valid hits in a single vector, preserving order by increasing t
    std::vector<HitRecord> valid_hits1;
    valid_hits1.reserve(hits1.size());
    for (const auto &hit : hits1)
      if (hit_on_obj1_valid(hit.world_point)) valid_hits1.push_back(hit);

    std::vector<HitRecord> valid_hits2;
    valid_hits2.reserve(hits2.size());
    for (const auto &hit : hits2)
      if (hit_on_obj2_valid(hit.world_point)) valid_hits2.push_back(hit);

    auto valid_hits =  merge_ordered_vectors(valid_hits1, valid_hits2, [](const HitRecord &hit1, const HitRecord &hit2) {
          return hit1.t < hit2.t;
        });

    // Transform the hits back to the world's reference frame
    for (auto &hit : valid_hits) { hit.transform_in_place(transformation); }
    return valid_hits;
  } 

  /// @brief Check if a point lies inside the shape
  bool is_point_inside(Point point_world_frame) const noexcept override {

    // Transform the ray to the children objects' reference frame
    Point point = transformation.inverse() * point_world_frame;

    const bool inside1 = object1->is_point_inside(point_world_frame);
    const bool inside2 = object2->is_point_inside(point_world_frame);

    switch (operation) {
      case Operation::UNION:
        return inside1 || inside2;
      case Operation::INTERSECTION:
        return inside1 && inside2;
      case Operation::DIFFERENCE:
        return inside1 && !inside2;
      case Operation::FUSION:
        return inside1 || inside2;
    }
    std::unreachable();
  }


private:

  bool hit_on_obj1_valid(Point hit_point) const noexcept {
    const bool inside2 = object2->is_point_inside(hit_point);

    switch (operation) {
      case Operation::UNION:
        return true;
      case Operation::INTERSECTION:
        return inside2;
      case Operation::DIFFERENCE:
        return !inside2;
       case Operation::FUSION:
        return !inside2;
    }
    std::unreachable();
  }

  bool hit_on_obj2_valid(Point hit_point) const noexcept {
    const bool inside1 = object1->is_point_inside(hit_point);

    switch (operation) {
      case Operation::UNION:
        return true;
      case Operation::INTERSECTION:
        return inside1;
      case Operation::DIFFERENCE:
        return inside1;
       case Operation::FUSION:
        return !inside1;
    }
    std::unreachable();
  }
};


//-------------------------------------------------------------------------------------------------------------
// ----------- POINT LIGHT SOURCE CLASS ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Point-like light source, used in point light tracing
class PointLightSource {
public:
  // ------- Properties --------
  Point point;
  Color color;
  float emission_radius; // fictitious radius r of the light source, used to compute solid angle rescaling at distance
                         // d: (r/d)^2

  // ------- Constructors --------
  /// @brief Constructor with arguments
  /// @param position of the light source
  /// @param color of the light
  /// @param fictitious radius, used to compute solid angle rescaling with distance
  PointLightSource(Point point = Point(), Color color = WHITE, float emission_radius = 0.f)
      : point{point}, color{color}, emission_radius{emission_radius} {}
};



//-------------------------------------------------------------------------------------------------------------
// -----------WORLD CLASS ------------------
// ------------------------------------------------------------------------------------------------------------
/// @brief World class contains all the objects in the scene (and light sources for pointlight tracing) and trace rays against
/// them
class World {
public:
  // ------- Properties --------

  /// @brief vector containing the shapes in the scene
  std::vector<std::unique_ptr<Object>> objects;
  /// @brief vector containing the light sources in the scene (use for illumination in point-light tracing)
  std::vector<std::unique_ptr<PointLightSource>> light_sources;

  // ----------- Constructors -----------

  /// @brief Default constructor
  World() = default;

  // -------------------- Methods ----------------------

  /// @brief Add a shape to the scene
  /// @param object shape to add
  void add_object(std::unique_ptr<Object> &&object) { objects.push_back(std::move(object)); }

  /// @brief Add a point light source to the scene
  /// @param point light source to add
  void add_light_source(std::unique_ptr<PointLightSource> &&light_source) { light_sources.push_back(std::move(light_source)); }

  /// @brief Finds the closest intersection of a ray with the objects in the scene
  /// @param ray to be traced through the world
  /// @return closest intersection info (or std::nullopt if no hit)
  std::optional<HitRecord> ray_intersection(const Ray &ray) const noexcept {
    std::optional<HitRecord> closest_hit; // closest hit found (if any)

    // Loop through all objects in World
    for (const auto &object : objects) {
      std::optional<HitRecord> hit = object->ray_intersection(ray); // Try finding intersection with current object

      // If there's a valid hit and it's closer than any previous hit, update closest_hit
      // (note there's no need to check hit->t > 0 since we already do it inside ray_intersection method of Objects)
      if (hit.has_value() && hit->t < (closest_hit.has_value() ? closest_hit->t : infinity)) {
        closest_hit = hit;
      }
    }
    return closest_hit; // Return the closest hit (or std::nullopt if none found)
  }

  /// @brief Finds the first intersection of a ray with objects in the world list (in iteration order, not necessarily the
  /// closest) used to speed up on_off rendering of images
  /// @param ray to be traced through the world
  /// @return first intersection info (or std::nullopt if no hit is found)
  std::optional<HitRecord> on_off_ray_intersection(const Ray &ray) const noexcept {
    std::optional<HitRecord> first_hit; // First hit found in the list (if any)

    // Loop through all objects in World
    for (const auto &object : objects) {
      first_hit = object->ray_intersection(ray); // Try intersecting with this object
      if (first_hit.has_value()) {
        return first_hit; // Return the first hit found
      }
    }
    return first_hit; // Return the first hit (nullopt if none is found)
  }

  /// @brief Compute ray connecting a viewer's point to a point on the surface of an object if the latter is visible
  /// @param viewer point
  /// @param surface point
  /// @param normal at the surface
  std::optional<Vec> offset_if_visible(Point viewer_point, Point surface_point, Normal normal_at_surface) const {
    Vec in_dir = surface_point - viewer_point;
    Ray in_ray{viewer_point, in_dir};

    // Return null if the ray comes from inside the object
    if (in_dir * normal_at_surface > 0.f) {
      return std::nullopt;
    }

    // Note that the algorithm doesn't consider the case where the point light source is visible via a specular
    // reflection. Loop over the objects in the World and return nullopt if one of them sits before surface_point 
    // (i. e. if the hit with ray in_dir has t < 1).
    for (const auto &object : objects) {
      std::optional<HitRecord> hit = object->ray_intersection(in_ray);
      if (hit.has_value() && hit->t < 1.f && !hit->world_point.is_close(surface_point)) {
        return std::nullopt;
      }
    }
    return in_dir;
  }
};
