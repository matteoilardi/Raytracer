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

#include <memory>   //library for smart pointers
#include <optional> //library for nullable types in c++

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class HitRecord;
class Shape;
class Sphere;
class Plane;

//-------------------------------------------------------------------------------------------------------------
// -----------HIT RECORD CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief HitRecord class is used to store information on the intersection of a ray with a shape
class HitRecord {
public:
  //-------Properties--------

  Point world_point;   // 3D coordinates of the intersection point in real world
  Normal normal;       // normal to the surface at the intersection point
  Vec2d surface_point; // 2D coordinates on the surface
  Ray ray;             // the ray that actually hit the shape
  float t;             // distance from the origin of the ray to the intersection point

  //-----------Constructors-----------
  /// Default constructor
  HitRecord() : world_point(Point()), normal(Normal()), surface_point(Vec2d()), ray(Ray()), t(0.) {};

  /// Constructor with parameters
  HitRecord(Point world_point, Normal normal, Vec2d surface_point, Ray ray, float t)
      : world_point(world_point), normal(normal), surface_point(surface_point), ray(ray), t(t) {};

  //------------Methods-----------
  bool is_close(HitRecord other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return world_point.is_close(other.world_point, error_tolerance) && normal.is_close(other.normal, error_tolerance) &&
           surface_point.is_close(other.surface_point, error_tolerance) && ray.is_close(other.ray, error_tolerance) &&
           are_close(t, other.t, error_tolerance);
  }
};

// ------------------------------------------------------------------------------------------------------------
// -----------SHAPE CLASS (abstract) ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Shape class is the base class for all shapes in the scene
class Shape {
public:
  //-------Properties--------
  ///@brief transformation taking to the proper reference frame of the shape
  Transformation transformation;

  //-----------Constructors-----------
  /// Default constructor
  Shape() : transformation(Transformation()) {};

  /// @brief Constructor with parameters
  /// @param tranformation taking you to the shape reference frame
  Shape(Transformation transformation) : transformation(transformation) {};

  virtual ~Shape() {}

  //--------------------Methods----------------------

  ///@brief virtual method that implements the intersection of a given ray with the shape considered
  ///@param ray incoming ray hitting the shape
  virtual std::optional<HitRecord> ray_intersection(Ray ray) const {
    throw std::logic_error("Shape.ray_intersection is an abstract method and cannot be called directly");
  }

  ///@brief flip the normal to the surface so that it has negative scalar product with the hitting ray
  ///@param normal normal to the surface
  ///@param ray incoming ray hitting the shape
  Normal flip_normal(Normal normal, Ray ray) const {
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
  // NOTE nothing should be here, since by default the sphere is at the origin and has radius 1
  // (even for an ellipsoid, just transform it to the unit sphere at the origin & store all info in the transformation)

  //-----------Constructors-----------
  /// Default constructor
  Sphere() : Shape() {};

  /// Constructor with parameters
  Sphere(Transformation transformation) : Shape(transformation) {};

  //--------------------Methods----------------------

  ///@brief implementation of virtual method that returns the information of the intersection with a given ray
  ///@param ray incoming ray hitting the shape
  virtual std::optional<HitRecord> ray_intersection(Ray ray) const override {

    // TODO implement this method and the auxiliary functions needed

    // STEPS
    // 1. Transform the ray to the plane reference frame
    Ray shape_frame_ray = ray.transform(transformation.inverse());

    // 2. Compute the intersection points if any (2nd degree equation) slides 8b 29-31
    // 3. Choose the closest with t>0
    // 4. Compute the normal to the surface at the intersection point  //NOTE use the FLIP_NORMAL method in Shape
    // 5. Compute the 2D coordinates on the surface (u,v) of the intersection point
    // 4. Transform the intersection point parameters (HitRecord) back to the world reference frame

    return HitRecord();
  };
};

//-------------------------------------------------------------------------------------------------------------
// -----------PLANE CLASS ------------------
// ------------------------------------------------------------------------------------------------------------
class Plane : public Shape {
public:
  //-------Properties--------
  // NOTE nothins should be here, since by default the plane is at the origin and normal to the z axis

  //-----------Constructors-----------
  /// Default constructor
  Plane() : Shape() {};

  /// Constructor with parameters
  Plane(Transformation transformation) : Shape(transformation) {};

  //--------------------Methods----------------------

  ///@brief implementation of virtual method that returns the information of the intersection with a given ray
  ///@param ray incoming ray hitting the shape
  virtual std::optional<HitRecord> ray_intersection(Ray ray) const override {

    // TODO implement this method and the auxiliary functions needed

    // STEPS
    // 1. Transform the ray to the plane reference frame
    Ray shape_frame_ray = ray.transform(transformation.inverse());

    // 2. Check if the ray is parallel to the plane
    // 3. If not, compute the intersection point
    // 4. Compute the normal to the surface at the intersection point //NOTE use the FLIP_NORMAL method in Shape
    // 5. Compute the 2D coordinates on the surface (u,v) of the intersection point
    // 6. Transform the intersection point parameters (HitRecord) back to the world reference frame

    return HitRecord();
  };
};

//-------------------------------------------------------------------------------------------------------------
// -----------WORLD CLASS ------------------
// ------------------------------------------------------------------------------------------------------------
/// @brief World class contains all the objects in the scene and trace rays against them
class World {
public:
  // ------- Properties --------

  ///@brief list of all the shapes present in the scene (stored as shared_ptr for polymorphism and memory safety)
  std::vector<std::shared_ptr<Shape>> objects;

  // ----------- Constructors -----------

  /// default constructor
  World() : objects() {};

  // -------------------- Methods ----------------------

  /// @brief adds a shape to the scene
  /// @param object shape to add to the scene
  void add_object(std::shared_ptr<Shape> object) { objects.push_back(object); }

  /// @brief returns the closest intersection of a ray with the objects in the scene
  /// @param ray to be traced through the world
  /// @return std::optional<HitRecord> containing the closest intersection info (or std::nullopt if no hit)
  std::optional<HitRecord> ray_intersection(const Ray &ray) const {
    std::optional<HitRecord> closest_hit; // closest hit found (if any)
    float closest_t = infinite;           // distance to the closest hit (initialize infinite)

    // loop through all objects in the world
    for (const auto &object : objects) {
      std::optional<HitRecord> hit = object->ray_intersection(ray); // try intersecting with this object
      // object->ray_intersection(ray) is shorthand for (*object).ray_intersection(ray)

      // if there's a valid hit and it's closer than any previous one update closest hit and distance
      if (hit.has_value() && hit->t > 0 && hit->t < closest_t) {
        closest_t = hit->t; // update closest distance
        closest_hit = hit;  // update closest hit info
      }
    }

    return closest_hit; // return the closest hit (or nullopt if none found)
  }
};