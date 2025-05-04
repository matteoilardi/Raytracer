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
  virtual std::optional<HitRecord> ray_intersection(Ray ray)  const {
    throw std::logic_error("Shape.ray_intersection is an abstract method and cannot be called directly");
}
};

class Sphere : public Shape {
public:
  //-------Properties--------
  //NOTE is it useful in any sense to store the origin and 3 semiaxes of the ellipsoid, according to the transformation?
  // (recall by default we transform it into the unit sphere at the origin)

  //-----------Constructors-----------
  /// Default constructor
  Sphere() : Shape() {};

  /// Constructor with parameters
  Sphere(Transformation transformation) : Shape(transformation) {};

  //--------------------Methods----------------------

  ///@brief implementation of virtual method that returns the information of the intersection with a given ray
  ///@param ray incoming ray hitting the shape
  virtual std::optional<HitRecord> ray_intersection(Ray ray) const override { 
    //TODO implement this method and the auxiliary functions needed
    return HitRecord(); };
};