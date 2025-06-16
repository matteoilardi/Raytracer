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
#include <memory> // library for smart pointers
#include <numbers>
#include <optional> // library for nullable types in C++

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class HitRecord;
class Shape;
class Sphere;
class Plane;
class PointLightSource;
class World;

//-------------------------------------------------------------------------------------------------------------
// -----------HIT RECORD CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief HitRecord class is used to store information on the intersection of a ray with a shape
class HitRecord {
public:
  //-------Properties--------

  std::shared_ptr<const Shape> shape; // shape that was hit (required in order to trace back to the material that was hit)
  Point world_point;                  // 3D coordinates of the intersection point in real world
  Normal normal;                      // normal to the surface at the intersection point
  Vec2d surface_point;                // 2D coordinates on the surface
  Ray ray;                            // the ray that actually hit the shape
  float t;                            // distance from the origin of the ray to the intersection point

  //-----------Constructors-----------
  /// Default constructor
  HitRecord() : world_point(Point()), normal(Normal()), surface_point(Vec2d()), ray(Ray()), t(0.) {};

  /// Constructor with parameters
  HitRecord(std::shared_ptr<const Shape> shape, Point world_point, Normal normal, Vec2d surface_point, Ray ray, float t)
      : shape(shape), world_point(world_point), normal(normal), surface_point(surface_point), ray(ray), t(t) {};

  //------------Methods-----------
  bool is_close(HitRecord other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return (this->shape == other.shape) && world_point.is_close(other.world_point, error_tolerance) &&
           normal.is_close(other.normal, error_tolerance) && surface_point.is_close(other.surface_point, error_tolerance) &&
           ray.is_close(other.ray, error_tolerance) && are_close(t, other.t, error_tolerance);
  }
};

// ------------------------------------------------------------------------------------------------------------
// -----------SHAPE CLASS (abstract) ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Shape class is the base class for all shapes in the scene
class Shape : public std::enable_shared_from_this<Shape> {
  // Shape must inherit from this template class in order for its methods to return a shared_ptr to itself
  // via the shared_from_this() method built into the std::enable_shared_from_this<Shape> class
  // this is needed in the ray_intersection method to return a shared_ptr to the shape that was hit
public:
  //-------Properties--------
  ///@brief transformation describing the actual position of the shape in the world reference frame
  Transformation transformation;
  ///@brief properties (pigment and brdf) of the shape as a function of (u, v)
  std::shared_ptr<Material> material;

  //-----------Constructors-----------
  /// Default constructor
  Shape() : transformation(Transformation()) { material = std::make_shared<Material>(); };

  /// Constructor with parameters
  /// @param tranformation taking you to the shape reference frame
  /// @param material properties of the shape (pigment and brdf) as a function of (u, v)
  Shape(Transformation transformation, std::shared_ptr<Material> material = nullptr)
      : transformation(transformation), material(material) {
    if (!this->material) {
      material = std::make_shared<Material>(); // if no material is provided, set default material with both diffusive BRDF black
                                               // and uniform pigment black
    }
  };

  /// virtual destructor to ensure proper cleanup of derived classes
  virtual ~Shape() {}

  //--------------------Methods----------------------

  ///@brief virtual method that implements the intersection of a given ray with the shape considered
  ///@param ray incoming ray hitting the shape
  virtual std::optional<HitRecord> ray_intersection(Ray ray) const = 0;

  ///@brief flip the normal to the surface so that it has negative scalar product with the hitting ray
  ///@param normal normal to the surface
  ///@param ray incoming ray hitting the shape
  Normal enforce_correct_normal_orientation(Normal normal, Ray ray) const {
    if (normal * ray.direction > 0) {
      return -normal; // the normal and the ray must have opposite directions, if this is false (dot product>0), flip
                      // the normal
    } else {
      return normal;
    }
  }
};

//-------------------------------------------------------------------------------------------------------------
// -----------SPHERE CLASS ------------------
// ------------------------------------------------------------------------------------------------------------

class Sphere : public Shape {
public:
  //-------Properties--------
  // by default, the sphere is centered at the origin and has a radius of 1
  // so we need no properties here, all the info is specified in the transformation (that can take to an ellipsoid
  // centered anywhere)

  //-----------Constructors-----------
  /// Default constructor
  Sphere() : Shape() {};

  /// Constructor with parameters
  Sphere(Transformation transformation, std::shared_ptr<Material> material = nullptr) : Shape(transformation, material) {};

  //--------------------Methods----------------------

  ///@brief implementation of virtual method that returns the information of the intersection with a given ray
  ///@param ray incoming ray hitting the shape
  virtual std::optional<HitRecord> ray_intersection(Ray ray_world_frame) const override {
    //  Important note: unless otherwise specified, every geometrical object in the body of this method is in the
    //  reference frame of the *standard* sphere

    // 1. Transform the ray to the *standard* sphere's reference frame
    Ray ray = ray_world_frame.transform(transformation.inverse());

    // 2. Compute the discriminant of the 2nd degree equation in slides 8b 29-31, return null if the ray is tangent (as
    // if there were no intersections)
    Vec O = ray.origin.to_vector();
    float reduced_discriminant = std::powf(O * ray.direction, 2) - ray.direction.squared_norm() * (O.squared_norm() - 1.f);
    if (reduced_discriminant == 0.f) {
      return std::nullopt;
    }

    // 3. Choose the smallest solution t > 0, which corresponds to the first hitting point of the ray (remember
    // t_first_hit is the same in both reference frames)
    float t_first_hit;
    float t1 = (-O * ray.direction - std::sqrt(reduced_discriminant)) / ray.direction.squared_norm();
    if (t1 > ray.tmin && t1 < ray.tmax) {
      t_first_hit = t1;
    } else {
      float t2 = (-O * ray.direction + std::sqrt(reduced_discriminant)) / ray.direction.squared_norm();
      if (t2 > ray.tmin && t2 < ray.tmax) {
        t_first_hit = t2;
      } else {
        return std::nullopt;
      }
    }

    // 4. Find the corresponding hitting point in the *standard* sphere's reference frame
    Point hit_point = ray.at(t_first_hit);

    // 5. Compute the normal to the surface at the intersection point in the *standard* sphere's reference frame
    Normal normal = Normal(hit_point.x, hit_point.y, hit_point.z); // normal to the sphere is just the vector from the origin
    normal = enforce_correct_normal_orientation(normal, ray);      // enforce normal and hitting ray have opposite directions

    // 6. Compute the 2D coordinates on the surface (u,v) of the intersection point (they are the same in the world's
    // reference frame by our convention)
    float u = atan2f(hit_point.y, hit_point.x) / (2.f * std::numbers::pi_v<float>); // atan2 is the arctangent
    if (u < 0.f) {
      u = u + 1.f;
    } // This is necessary in order to have v in range (0, 1] because the output of atan2 is in range (-pi, pi]
    float v = std::acos(hit_point.z) / std::numbers::pi_v<float>;
    Vec2d surface_coordinates = Vec2d(u, v); // We follow the convention (u, v) = (phi, theta)

    // 7. Transform the intersection point parameters back to the world's reference frame
    std::optional<HitRecord> hit;
    // nullable type cannot be built by calling the construcor directly, need to use emplace or similar syntax instead
    hit.emplace(shared_from_this(), transformation * hit_point, transformation * normal, surface_coordinates, ray_world_frame,
                t_first_hit);
    return hit;
  };
};

//-------------------------------------------------------------------------------------------------------------
// -----------PLANE CLASS ------------------
// ------------------------------------------------------------------------------------------------------------
class Plane : public Shape {
public:
  //-------Properties--------
  // again by default the plane is at the origin and normal to the z axis,
  // so we need no futher properties here, all the info is in the transformation

  //-----------Constructors-----------
  /// Default constructor
  Plane() : Shape() {};

  /// Constructor with parameters
  Plane(Transformation transformation, std::shared_ptr<Material> material = nullptr) : Shape(transformation, material) {};

  //--------------------Methods----------------------

  ///@brief implementation of virtual method that returns the information of the intersection with a given ray
  ///@param ray incoming ray hitting the shape
  virtual std::optional<HitRecord> ray_intersection(Ray ray_world_frame) const override {
    // Important note: unless otherwise specified, every geometrical object in the body of this method is in the
    // reference frame of the *standard* plane

    // 1. Transform the ray to the plane reference frame
    Ray ray = ray_world_frame.transform(transformation.inverse());

    // 2. Return null if the ray is parallel to the plane
    if (are_close(ray.direction.z, 0.f)) {
      return std::nullopt;
    }

    // 3. Compute the t at the intersection point and return null if t < 0, compute the intersection point
    float t_hit = -ray.origin.to_vector().z / ray.direction.z;
    if (t_hit < ray.tmin || t_hit > ray.tmax) {
      return std::nullopt;
    }
    Point hit_point = ray.at(t_hit);

    // 4. Compute the normal to the plane at the intersection point (i. e. choose the sign of the normal)
    Normal normal = enforce_correct_normal_orientation(VEC_Z.to_normal(), ray);

    // 5. Compute surface 2D coordinates (u,v) of the intersection point (periodic parametrization Tomasi Lesson 8a
    // slides 37-38)
    Vec2d surface_coordinates = Vec2d(hit_point.x - std::floor(hit_point.x), hit_point.y - std::floor(hit_point.y));

    // 6. Transform the intersection point parameters (HitRecord) back to the world reference frame
    std::optional<HitRecord> hit;
    hit.emplace(shared_from_this(), transformation * hit_point, transformation * normal, surface_coordinates, ray_world_frame,
                t_hit);
    return hit;
  };
};

//-------------------------------------------------------------------------------------------------------------
// ----------- POINT LIGHT SOURCE CLASS ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief point-like light source, used in point light tracing
class PointLightSource {
public:
  // ------- Properties --------
  Point point;
  Color color;
  float emission_radius; // fictitious radius r of the light source, used to compute solid angle rescaling at distance
                         // d: (r/d)^2

  // ------- Constructors --------
  /// Constructor with arguments
  /// @param position of the light source
  /// @param color of the light
  /// @param fictitious radius, used to compute solid angle rescaling with distance
  PointLightSource(Point point = Point(), Color color = Color(1.f, 1.f, 1.f), float emission_radius = 0.f)
      : point(point), color(color), emission_radius(emission_radius) {};
};

//-------------------------------------------------------------------------------------------------------------
// -----------WORLD CLASS ------------------
// ------------------------------------------------------------------------------------------------------------
/// @brief World class contains all the objects in the scene (and light sources for pointlight tracing) and trace rays against
/// them
class World {
public:
  // ------- Properties --------

  /// @brief vector containing the shapes in the scene (stored as shared_ptr for polymorphism and memory safety)
  std::vector<std::shared_ptr<Shape>> objects;
  /// @brief vector containing the light sources in the scene (use for illumination in point-light tracing)
  std::vector<std::shared_ptr<PointLightSource>> light_sources;

  // ----------- Constructors -----------

  /// default constructor
  World() : objects(), light_sources() {};

  // -------------------- Methods ----------------------

  /// @brief adds a shape to the scene
  /// @param object shape to add
  void add_object(std::shared_ptr<Shape> object) { objects.push_back(object); }

  /// @brief add a point light source to the scene
  /// @param point light source to add
  void add_light_source(std::shared_ptr<PointLightSource> light_source) { light_sources.push_back(light_source); }

  /// @brief returns the closest intersection of a ray with the objects in the scene
  /// @param ray to be traced through the world
  /// @return std::optional<HitRecord> containing the closest intersection info (or std::nullopt if no hit)
  std::optional<HitRecord> ray_intersection(const Ray &ray) const {
    std::optional<HitRecord> closest_hit; // closest hit found (if any)

    // loop through all objects in World
    for (const auto &object : objects) {
      std::optional<HitRecord> hit = object->ray_intersection(ray); // try intersecting with this object
      // object->ray_intersection(ray) is shorthand for (*object).ray_intersection(ray)

      // if there's a valid hit and it's closer than any previous hit, update closest_hit
      // (note there's no need to check hit->t > 0 since we already do it inside ray_intersection method of Shapes)
      if (hit.has_value() && hit->t < (closest_hit.has_value() ? closest_hit->t : infinite)) {
        closest_hit = hit;
      }
    }
    return closest_hit; // return the closest hit (or nullopt if none found)
  }

  /// @brief returns first intersection of a ray with objects in the world list (in iteration order, not necessarily the closest)
  /// used to speed up on_off rendering of images
  /// @param ray to be traced through the world
  /// @return std::optional<HitRecord> containing the info on the first intersection in the list (or std::nullopt if no
  /// hit)
  std::optional<HitRecord> on_off_ray_intersection(const Ray &ray) const {
    std::optional<HitRecord> first_hit; // first hit found in the list (if any)
    // loop through all objects in World
    for (const auto &object : objects) {
      first_hit = object->ray_intersection(ray); // try intersecting with this object
      if (first_hit.has_value()) {
        return first_hit; // return the first hit found
      }
    }
    return first_hit; // return the first hit (or nullopt if none found)
  }

  /// @brief returns ray connecting a viewer's point to a point on the surface of an object if the latter is visible
  /// @param viewer point
  /// @param surface point
  /// @param normal at the surface
  std::optional<Vec> offset_if_visible(Point viewer_point, Point surface_point, Normal normal_at_surface) {
    Vec in_dir = surface_point - viewer_point;
    Ray in_ray{viewer_point, in_dir};

    // return null if the ray comes from inside the object
    if (in_dir * normal_at_surface > 0.f) {
      return std::nullopt;
    }

    // Note that the algorithm doesn't consider the case where the point light source is visible via a speculaar reflection
    // Loop over the objects in the World and return null if one of them sits before surface_point
    for (const auto &object : objects) {
      std::optional<HitRecord> hit = object->ray_intersection(in_ray);
      if (hit.has_value() && hit->t < 1.f && !hit->world_point.is_close(surface_point)) {
        return std::nullopt;
      }
    }
    return std::make_optional<Vec>(in_dir);
  }
};
