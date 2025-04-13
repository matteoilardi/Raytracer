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
#include <memory>
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

class Ray
{
public:
  //-------Properties--------
  Point origin;          // Origin of the ray
  Vec direction;         // Direction of the ray
  float tmin = 1e-5;     // Minimum distance run along the ray
  float tmax = infinite; // Maximum distance run along the ray
  int depth = 0;         // Number of reflections before is considered exhausted

  //-----------Constructors-----------

  /// Default constructor
  Ray() : origin(Point()), direction(Vec()) {};

  /// Constructor with parameters
  Ray(Point origin, Vec direction) : origin(origin), direction(direction) {};
  Ray(Point origin, Vec direction, float tmin, float tmax, int depth) : origin(origin), direction(direction), tmin(tmin), tmax(tmax), depth(depth) {};

  //--------------------Methods----------------------
  ///@brief check if origin and direction are close to the other ray's
  bool is_close(Ray other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const
  {
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
// -----------CAMERA CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class Camera
{
public:
  //-------Properties--------
  float asp_ratio;               // aspect ratio
  Transformation transformation; // transformation that takes into account camera orientation

  //-----------Constructors-----------
  /// Default constructor
  Camera() : asp_ratio(1.), transformation(Transformation()) {};

  /// @brief Constructor with parameters
  /// @param aspect ratio
  /// @param tranformation encoding observer's orientation
  Camera(float asp_ratio, Transformation transformation) : asp_ratio(asp_ratio), transformation(transformation) {};

  virtual ~Camera() {}

  //--------------------Methods----------------------
  /// @brief virtual method that fires a ray through a point of the screen
  /// @param screen coordinate u
  /// @param screen coordinate v
  virtual Ray fire_ray(float u, float v) const = 0; //
};

class OrthogonalCamera : public Camera
{
public:
  //-----------Constructors-----------
  /// Default constructor

  /// Constructor with parameters
  OrthogonalCamera(float asp_ratio = 1., Transformation transformation = Transformation()) : Camera(asp_ratio, transformation) {}
  //--------------------Methods----------------------
  // TODO check if implementation is optimal and if it matters
  // ANSWER I think it is indeed optimal, I just have issues understanding the coordinate system in slides 15-21.

  ///@brief virtual method that fires a ray through the point of the screen of coordinates (u, v)
  virtual Ray fire_ray(float u, float v) const override
  {
    Point origin = Point(-1., (1. - 2. * u) * asp_ratio, -1. + 2 * v); // compare Lab 6, slide 15 ad slide 20-21
    Vec direction = VEC_X;
    return Ray(origin, direction).transform(transformation);
  };
};

class PerspectiveCamera : public Camera
{
public:
  //-------Properties--------
  float distance;

  //-----------Constructors-----------
  /// Default constructor

  /// Constructor with parameters
  PerspectiveCamera(float distance = 1., float asp_ratio = 1., Transformation transformation = Transformation()) : Camera(asp_ratio, transformation), distance(distance) {}

  //--------------------Methods----------------------
  // TODO check if implementation is optimal (and if it matters)
  // ANSWER I think it is indeed optimal, I just have issues understanding the coordinate system in slides 15-21.

  ///@brief virtual method that fires a ray through the point of the screen of coordinates (u, v)
  virtual Ray fire_ray(float u, float v) const override
  {
    Point origin = Point(-distance, 0., 0.);
    Vec direction = Vec(distance, (1. - 2. * u) * asp_ratio, -1. + 2 * v); // compare Lab 6, slide 15 ad slide 20-21
    return Ray(origin, direction).transform(transformation);
  };
};

// ------------------------------------------------------------------------------------------------------------
// -----------IMAGE_TRACER CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class ImageTracer
{
public:
  //-------Properties--------
  std::unique_ptr<HdrImage> image;
  std::unique_ptr<Camera> camera; // Safer version of a regular pointer. "Unique" because the only owner of the object Camera is the ImageTracer

  //-----------Constructors-----------

  /// Default constructor

  // QUESTION I might need an explanation for this constructor
  /// Constructor with parameters
  ImageTracer(std::unique_ptr<HdrImage> image, std::unique_ptr<Camera> camera) : image(std::move(image)), camera(std::move(camera)) {}
  // TODO check if is ok for ImageTracer to stay on the heap, which is a consequence of the above implementation
  // TODO consider checking whether the unique_ptr provided is not dangling

  //--------------------Methods----------------------

  ///@brief returns a ray originating from the camera hitting the pixel (col, row) of the image
  /// @param col column of the pixel
  /// @param row row of the pixel
  /// @param u_pixel (optional) x-coordinate of the pixel (default value is 0.5, meaning the ray hits the x-center of the pixel)
  /// @param v_pixel (optional) y-coordinate of the pixel (default value is 0.5, meaning the ray hits y-center of the pixel)
  Ray fire_ray(int col, int row, float u_pixel = 0.5, float v_pixel = 0.5)
  {
    // convert pixel indices into a position on the screen
    // default values of u_pixel and v_pixel make the ray hit the center of the pixel
    // NOTE this is the formula from Tomasi Lab 6b slide 31, he says there is a mistake in this formula, but to use it anyway...
    float u = (col + u_pixel) / (image->width - 1);
    float v = (row + v_pixel) / (image->height - 1);
    return camera->fire_ray(u, v);
  }

  // NOTE When we have implemented the solution to the rendering equation, we'll be able to tell if this hybrid kind of polymorphism (OO for cameras, PO for solvers) actually makes sense
  // DISCUSSION I need an explanation for this point

  // NOTE Where should the definition of RaySolver be?
  // ANSWER I would place it at the beginning of cameras.hpp, in the ---forward declarations,etc--- section

  using RaySolver = Color(Ray); // General function that takes a Ray as input and returns a Color

  // QUESTION is there a reason why you using image->method/property instead of image.method/property?
  void fire_all_rays(RaySolver *func)
  {
    for (int col = 0; col < image->width; ++col)
    {
      for (int row = 0; row < image->height; ++row)
      {
        Ray ray = fire_ray(col, row);
        Color color = func(ray);
        image->set_pixel(col, row, color);
      }
    }
  }
};