// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR RAYTRACING
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include "colors.hpp"
#include "geometry.hpp"
#include <limits> // library to have infinity as a float

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class Ray;
class Camera;
class ImageTracer;

const float infinite = std::numeric_limits<float>::infinity(); // Define infinity as a float

// ------------------------------------------------------------------------------------------------------------
// -----------RAY CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class Ray {
    public:
      //-------Properties--------
      Point origin; // Origin of the ray
      Vec direction; // Direction of the ray
      float tmin = 1e-5; // Minimum distance run along the ray
      float tmax = infinite; // Maximum distance run along the ray
      int depth = 0; // Number of reflections before is considered exhausted
    
      //-----------Constructors-----------
    
      /// Default constructor
    
      /// Constructor with parameters
      Ray(Point origin, Vec direction) : origin(origin), direction(direction) {};
      Ray(Point origin, Vec direction, float tmin, float tmax, int depth) : origin(origin), direction(direction), tmin(tmin), tmax(tmax), depth(depth) {};
    
      //--------------------Methods----------------------
      bool is_close(Ray other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
        return origin.is_close(other.origin, error_tolerance) && direction.is_close(other.direction, error_tolerance);
      }

      Point at(float t) const { return origin + Vec(t * direction); }

      Ray transform(Transformation T) { return Ray(T * origin, T * direction, tmin, tmax, depth); };
    };

// ------------------------------------------------------------------------------------------------------------
// -----------CAMERA CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class Camera {
public:
  //-------Properties--------
  float distance; // distance from the screen
  float asp_ratio; // aspect ratio
  Transformation transformation; // transformation that takes into account camera orientation

  //-----------Constructors-----------
  /// Default constructor

  /// Constructor with parameters
  Camera(float distance, float asp_ratio, Transformation transformation) : distance(distance), asp_ratio(asp_ratio), transformation(transformation) {};

  //--------------------Methods----------------------
  virtual Ray fire_ray(float u, float v) const = 0; // virtual method that fires a ray through the point of the screen of coordinates (u, v)
};

// ------------------------------------------------------------------------------------------------------------
// -----------IMAGE_TRACER CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class ImageTracer {
    public:
      //-------Properties--------
    
      //-----------Constructors-----------
    
      /// Default constructor
    
      /// Constructor with parameters
    
      //--------------------Methods----------------------
    };