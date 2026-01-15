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
#include "random.hpp" // random numbers for antialiasing
#include <cassert>
#include <functional> // for std::function
#include <limits>     // for std::numeric_limits (infinity as float)
#include <memory>

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class Ray;
class Camera;
class ImageTracer;

inline constexpr auto infinity = std::numeric_limits<float>::infinity();

// ------------------------------------------------------------------------------------------------------------
// -----------RAY CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class Ray {
public:
  //-------Properties--------
  Point origin;          // Origin of the ray
  Vec direction;         // Direction of the ray
  float tmin = 1e-5f;    // Minimum distance run along the ray
  float tmax = infinity; // Maximum distance run along the ray
  int depth = 0;         // Number of reflections before is considered exhausted

  //-----------Constructors-----------

  /// Default constructor
  constexpr Ray() noexcept : origin{}, direction{} {};

  /// Constructor with parameters
  constexpr Ray(Point origin, Vec direction) noexcept : origin{origin}, direction{direction} {};

  constexpr Ray(Point origin, Vec direction, float tmin, float tmax, int depth) noexcept
      : origin{origin}, direction{direction}, tmin{tmin}, tmax{tmax}, depth{depth} {};

  //--------------------Methods----------------------
  ///@brief check if origin and direction are close to the other ray's
  bool is_close(const Ray &other, float error_tolerance = DEFAULT_ERROR_TOLERANCE) const noexcept {
    return origin.is_close(other.origin, error_tolerance) && direction.is_close(other.direction, error_tolerance);
  }

  /// @brief returns Point at the given distance from the screen
  /// @param t distance from the screen
  constexpr Point at(float t) const noexcept { return origin + Vec{t * direction}; }

  /// @brief apply a transformation to the ray (origin transformed like a point, direction transformed like a vector)
  /// @param transformation to be applied
  constexpr Ray transform(const Transformation &T) const noexcept { return Ray{T * origin, T * direction, tmin, tmax, depth}; };
};

// ------------------------------------------------------------------------------------------------------------
// -----------CAMERA CLASS (abstract)------------------
// ------------------------------------------------------------------------------------------------------------

class Camera {
public:
  //-------Properties--------
  std::optional<float> asp_ratio; // aspect ratio
  Transformation transformation;  // transformation that takes into account camera orientation

  //-----------Constructors-----------

  /// @brief Constructor with parameters
  /// @param aspect ratio
  /// @param tranformation encoding observer's orientation
  Camera(std::optional<float> asp_ratio, Transformation transformation = Transformation{})
      : asp_ratio{asp_ratio}, transformation{transformation} {};

  Camera(float asp_ratio, Transformation transformation)
      : asp_ratio(std::make_optional<float>(asp_ratio)), transformation{transformation} {};

  virtual ~Camera() {}

  //--------------------Methods----------------------
  /// @brief Fires a ray through the point of the screen of coordinates (u, v)
  /// @param Screen coordinate u
  /// @param Screen coordinate v
  /// @details This method is supposed to be called after asp_ratio is given a float value,
  /// which usually happens when the instance of Camera starts seeing an HdrImage object,
  /// inside Imagetracer. This is because the default aspect ratio is inferred from the size
  /// of the image.
  virtual Ray fire_ray(float u, float v) const = 0;

protected:
  /// @brief Maximum z coordinate of the screen; maximum y coordinate is scaled by aspect ratio
  static constexpr float SCREEN_MAX = 1.f;

  /// @brief Minimum z coordinate of the screen; minimum y coordinate is scaled by aspect ratio
  static constexpr float SCREEN_MIN = -1.f;

  static constexpr float SCREEN_RANGE = SCREEN_MAX - SCREEN_MIN;

  /// @brief Maps screen coordinate u to y coordinate of the general reference frame
  float u_to_y(float u) const {
    // Compare Lab 6, slide 15 ad slide 20-21
    return (SCREEN_MAX - u * SCREEN_RANGE) * asp_ratio.value();
  }

  /// @brief Maps screen coordinate v to z coordinate of the general reference frame
  static float v_to_z(float v) {
    // Compare Lab 6, slide 15 ad slide 20-21
    return SCREEN_MIN + v * SCREEN_RANGE;
  }
};

class OrthogonalCamera : public Camera {
public:
  //-----------Constructors-----------
  //
  /// Constructor with parameters
  OrthogonalCamera(std::optional<float> asp_ratio = std::nullopt, const Transformation &transformation = Transformation{})
      : Camera{asp_ratio, transformation} {}
  //--------------------Methods----------------------

  virtual Ray fire_ray(float u, float v) const override {
    Point origin = Point{-1.f, u_to_y(u), v_to_z(v)};
    constexpr Vec direction{VEC_X};
    return Ray{origin, direction}.transform(transformation);
  };
};

class PerspectiveCamera : public Camera {
public:
  //-------Properties--------
  float distance;

  //-----------Constructors-----------

  /// Constructor with parameters
  PerspectiveCamera(float distance = 1.f, std::optional<float> asp_ratio = std::nullopt,
                    Transformation transformation = Transformation{})
      : Camera{asp_ratio, transformation}, distance{distance} {}

  //--------------------Methods----------------------

  ///@brief virtual method that fires a ray through the point of the screen of coordinates (u, v)
  virtual Ray fire_ray(float u, float v) const override {
    Point origin{-distance, 0.f, 0.f};
    Vec direction{distance, u_to_y(u), v_to_z(v)};
    return Ray{origin, direction}.transform(transformation);
  };
};

// ------------------------------------------------------------------------------------------------------------
// -----------IMAGE_TRACER CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class ImageTracer {
public:
  //-------Properties--------

  std::unique_ptr<HdrImage> image;
  std::shared_ptr<Camera> camera; // use shared_ptr for camera to allow sharing with Scene::camera
  int samples_per_pixel_edge;     // total samples per pixel = samples_per_pixel_side^2
  std::unique_ptr<PCG> pcg;       // random number generator, for antialiasing (stratified sampling)

  //-----------Constructors-----------

  /// @brief Constructor with parameters
  ImageTracer(std::unique_ptr<HdrImage> image, std::shared_ptr<Camera> camera, int samples_per_pixel_edge = 1,
              std::unique_ptr<PCG> pcg = nullptr)
      : image{std::move(image)}, camera{camera}, samples_per_pixel_edge{samples_per_pixel_edge}, pcg{std::move(pcg)} {
    if (!pcg) {
      this->pcg = std::make_unique<PCG>();
    }
    if (!camera->asp_ratio.has_value()) {
      camera->asp_ratio.emplace(static_cast<float>(this->image->width) / static_cast<float>(this->image->height));
    }
  }
  // Note that it is ok for image and camera to stay in the heap, since they will be created once and after that access to
  // heap memory is as fast as access to stack

  //--------------------Methods----------------------

  /// @brief returns a ray originating from the camera hitting the pixel (col, row) of the image
  /// @param col column of the pixel
  /// @param row row of the pixel
  /// @param u_pixel (optional) x-coordinate of the pixel (default value is 0.5, meaning the ray hits the x-center of
  /// the pixel)
  /// @param v_pixel (optional) y-coordinate of the pixel (default value is 0.5, meaning the ray hits y-center of the
  /// pixel)
  Ray fire_ray(int col, int row, float u_pixel = 0.5f, float v_pixel = 0.5f) {
    // Convert pixel indices into a position on the screen
    // Default values of u_pixel and v_pixel make the ray hit the center of the pixel

    float u = (col + u_pixel) / (image->width);
    float v = 1.f - ((row + v_pixel) / (image->height));

    return camera->fire_ray(u, v);
  }

  // General function that takes a Ray as input and returns a Color
  using RaySolver = std::function<Color(Ray)>;
  // Note that we use both OO polymorphism and PO polymorphism here (defining RaySolver as a generic function on its own,
  // rather than creating a parent class with a virtual method to be implemented by derived classes)

  // General function that takes as input a float ∈ [0.0, 1.0] representing progress with a given task and reports it to the main
  // The method perfomrming the task should accept a function of type ProgressCallback as an argument
  using ProgressCallback = std::function<void(float)>;

  /// @brief Calls fire_ray on every pixel of the image (multiple times if antialiasing is set on) and reports progress to the
  /// main
  void fire_all_rays(RaySolver func, ProgressCallback report_progress = [](float progress) -> void {}) {
    for (int col = 0; col < image->width; ++col) {
      for (int row = 0; row < image->height; ++row) {

        Color cum_color{};
        Ray ray;
        const int spp = samples_per_pixel_edge; // Just for code readability

        if (spp > 1) {                      // Perform antialiasing
          for (int i = 0; i < spp; i++) {   // i: intra-pixel col
            for (int j = 0; j < spp; j++) { // j: intra-pixel row (as opposed to v, v_pixel increases downwards)
              float u_pixel = ((float)i + pcg->random_float()) / spp;
              float v_pixel = ((float)j + pcg->random_float()) / spp;

              ray = fire_ray(col, row, u_pixel, v_pixel);
              cum_color += func(ray);
            }
          }
          cum_color /= static_cast<float>((spp * spp));

        } else {
          ray = fire_ray(col, row);
          cum_color = func(ray);
        }

        image->set_pixel(col, row, cum_color);
      }
      // Report progress every time the end of the outer loop is reached
      report_progress(static_cast<float>(col + 1) / image->width);
    }
  }
};
