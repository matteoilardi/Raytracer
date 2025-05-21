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
#include <functional> // library for std::function
#include <limits>     // library to have infinity as a float
#include <memory>

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class Ray;
class Camera;
class ImageTracer;

/// @brief define infinity as a float
const float infinite = std::numeric_limits<float>::infinity();

// ------------------------------------------------------------------------------------------------------------
// -----------RAY CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class Ray {
public:
  //-------Properties--------
  Point origin;          // Origin of the ray
  Vec direction;         // Direction of the ray
  float tmin = 1e-5f;    // Minimum distance run along the ray
  float tmax = infinite; // Maximum distance run along the ray
  int depth = 0;         // Number of reflections before is considered exhausted

  //-----------Constructors-----------

  /// Default constructor
  Ray() : origin(Point()), direction(Vec()) {};

  /// Constructor with parameters
  Ray(Point origin, Vec direction) : origin(origin), direction(direction) {};
  Ray(Point origin, Vec direction, float tmin, float tmax, int depth)
      : origin(origin), direction(direction), tmin(tmin), tmax(tmax), depth(depth) {};

  //--------------------Methods----------------------
  ///@brief check if origin and direction are close to the other ray's
  bool is_close(Ray other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const {
    return origin.is_close(other.origin, error_tolerance) && direction.is_close(other.direction, error_tolerance);
  }

  /// @brief returns Point at the given distance from the screen
  /// @param t distance from the screen
  Point at(float t) const { return origin + Vec(t * direction); }

  /// @brief apply a transformation to the ray (origin transformed like a point, direction transformed like a vector)
  /// @param transformation to be applied
  Ray transform(Transformation T) { return Ray(T * origin, T * direction, tmin, tmax, depth); };
};

// ------------------------------------------------------------------------------------------------------------
// -----------CAMERA CLASS (abstract)------------------
// ------------------------------------------------------------------------------------------------------------

class Camera {
public:
  //-------Properties--------
  float asp_ratio;               // aspect ratio
  Transformation transformation; // transformation that takes into account camera orientation

  //-----------Constructors-----------
  /// Default constructor
  Camera() : asp_ratio(1.f), transformation(Transformation()) {};

  /// @brief Constructor with parameters
  /// @param aspect ratio
  /// @param tranformation encoding observer's orientation
  Camera(float asp_ratio, Transformation transformation) : asp_ratio(asp_ratio), transformation(transformation) {};

  virtual ~Camera() {}

  //--------------------Methods----------------------
  /// @brief virtual method that fires a ray through a point of the screen
  /// @param screen coordinate u
  /// @param screen coordinate v
  virtual Ray fire_ray(float u, float v) const {
    throw std::logic_error("Camera.fire_ray is an abstract method and cannot be called directly");
  };
};

class OrthogonalCamera : public Camera {
public:
  //-----------Constructors-----------
  /// Default constructor

  /// Constructor with parameters
  OrthogonalCamera(float asp_ratio = 1.f, Transformation transformation = Transformation())
      : Camera(asp_ratio, transformation) {}
  //--------------------Methods----------------------

  ///@brief virtual method that fires a ray through the point of the screen of coordinates (u, v)
  virtual Ray fire_ray(float u, float v) const override {
    Point origin = Point(-1.f, (1.f - 2.f * u) * asp_ratio, -1.f + 2.f * v); // compare Lab 6, slide 15 ad slide 20-21
    Vec direction = VEC_X;
    return Ray(origin, direction).transform(transformation);
  };
};

class PerspectiveCamera : public Camera {
public:
  //-------Properties--------
  float distance;

  //-----------Constructors-----------
  /// Default constructor

  /// Constructor with parameters
  PerspectiveCamera(float distance = 1.f, float asp_ratio = 1.f, Transformation transformation = Transformation())
      : Camera(asp_ratio, transformation), distance(distance) {}

  //--------------------Methods----------------------

  ///@brief virtual method that fires a ray through the point of the screen of coordinates (u, v)
  virtual Ray fire_ray(float u, float v) const override {
    Point origin = Point(-distance, 0.f, 0.f);
    Vec direction =
        Vec(distance, (1.f - 2.f * u) * asp_ratio, -1.f + 2.f * v); // compare Lab 6, slide 15 ad slide 20-21
    return Ray(origin, direction).transform(transformation);
  };
};

// ------------------------------------------------------------------------------------------------------------
// -----------IMAGE_TRACER CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class ImageTracer {
public:
  //-------Properties--------

  // the pointers below can only point to objects in the heap, and if we define image/camera in the stack then we cannot
  // pass them via (say) &image,&camera
  std::unique_ptr<HdrImage> image;
  std::unique_ptr<Camera> camera; // Safer version of a regular pointer. "Unique" because the only owner of the object
                                  // Camera is the ImageTracer

  //-----------Constructors-----------

  /// Default constructor

  /// Constructor with parameters
  ImageTracer(std::unique_ptr<HdrImage> image, std::unique_ptr<Camera> camera)
      : image(std::move(image)), camera(std::move(camera)) {}
  // note it is ok for image and camera to stay in the heap, since they will be created once and after that access to
  // heap memory is as fast as access to stack NOTE pay attention to dangling pointers inside the main

  //--------------------Methods----------------------

  ///@brief returns a ray originating from the camera hitting the pixel (col, row) of the image
  /// @param col column of the pixel
  /// @param row row of the pixel
  /// @param u_pixel (optional) x-coordinate of the pixel (default value is 0.5, meaning the ray hits the x-center of
  /// the pixel)
  /// @param v_pixel (optional) y-coordinate of the pixel (default value is 0.5, meaning the ray hits y-center of the
  /// pixel)
  Ray fire_ray(int col, int row, float u_pixel = 0.5f, float v_pixel = 0.5f) {
    // convert pixel indices into a position on the screen
    // default values of u_pixel and v_pixel make the ray hit the center of the pixel

    float u = (col + u_pixel) / (image->width);
    float v = 1.f - ((row + v_pixel) / (image->height));

    return camera->fire_ray(u, v);
  }

  // note we use both OO polymorphism and PO polymorphism here (defining RaySolver as a generic function on its own,
  // rather than creating a parent class with a virtual method to be implemented by derived classes)
  using RaySolver = std::function<Color(Ray)>; // General function that takes a Ray as input and returns a Color

  void fire_all_rays(RaySolver func) {
    for (int col = 0; col < image->width; ++col) {
      for (int row = 0; row < image->height; ++row) {
        Ray ray = fire_ray(col, row);
        Color color = func(ray);
        image->set_pixel(col, row, color);
      }
    }
  }
};