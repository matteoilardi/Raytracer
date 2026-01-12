// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR MATERIALS (LIGHT SOURCES AND BRDFs) -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include "cameras.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "random.hpp"
#include <cmath>
#include <memory>
#include <numbers>

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

struct Pigment;
struct UniformPigment;
struct CheckeredPigment;

class BRDF;
class DiffusiveBRDF;
class SpecularBRDF;

class Material;

//-------------------------------------------------------------------------------------------------------------
// -----------PIGMENT ABSTRACT STRUCT ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Abstract functor that associates a Color to a Vec2d
struct Pigment {
  virtual ~Pigment() {}

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const = 0;
};

//-------------------------------------------------------------------------
//---------------------- UNIFORM PIGMENT STRUCT ------------------
//-------------------------------------------------------------------------
/// @brief Returns constant Color
struct UniformPigment : public Pigment {

  //-------Properties--------
  Color color;

  //-----------Constructors-----------

  /// Default constructor
  UniformPigment() = default;

  /// Constructor with parameters
  UniformPigment(Color color) : Pigment{}, color{color} {}

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const override { return color; }
};

//-------------------------------------------------------------------------
//---------------------- CHECKERED PIGMENT STRUCT ------------------
//-------------------------------------------------------------------------

/// @brief Return checkered pattern of two Colors
struct CheckeredPigment : public Pigment {

  //-------Properties--------
  Color color1, color2;
  int n_intervals; // number of intervals into which the range [0, 1] is divided for u and v coordinates
  float subinterval;
  //-----------Constructor-----------
  CheckeredPigment(Color color1, Color color2, int n_intervals = 10)
      : Pigment{}, color1{color1}, color2{color2}, n_intervals{n_intervals} {
    subinterval = 1.f / n_intervals; // length of u and v subintervals
  }

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const {
    // Get the column and row of the subinterval which the (u, v) coordinates fall in
    int col = static_cast<int>(uv.u / subinterval);
    int row = static_cast<int>(uv.v / subinterval);
    if ((col + row) % 2 == 0) { // entry has color1 if col and row have same parity
      return color1;
    } else {
      return color2; // it has color 2 otherwise
    }
  };
};

//-------------------------------------------------------------------------------------------------------------
// -----------IMAGE PIGMENT ------------------
// ------------------------------------------------------------------------------------------------------------
/// @brief Pigment obtained by wrapping an HDR image (pfm format) around the given shape
/// @brief uv coordinates on the surface are mapped to column and row of the image and the corresponding pixel color is returned
class ImagePigment : public Pigment {
public:
  // ------- Properties -------
  HdrImage image; // HDR image used as a texture to be wrapped around the shape

  // ------- Constructors -------
  /// @brief Construct an ImagePigment from a given HdrImage object
  /// @param image HdrImage object containing the HDR image
  ImagePigment(const HdrImage &image) : image{image} {}

  /// @brief Construct an ImagePigment from a given PFM image file
  /// @param filename path to the pfm file containing the HDR image
  ImagePigment(const std::string &filename) : image{filename} {}

  // ------- Methods -------
  /// @brief Given surface uv coordinates, return the corresponding Color from the texture
  /// @param uv coordinates in [0, 1)^2 identifying a point on the surface
  /// @return color extracted from the HDR image at that position
  virtual Color operator()(Vec2d uv) const override {
    // Convert u, v ∈ [0,1) to pixel indices in the image (flooring positive integers)
    int col = static_cast<int>(uv.u * image.width);
    int row = static_cast<int>(uv.v * image.height);

    // Clamp indices to avoid potential out-of-bounds (only needed if u or v == 1.0)
    // Technically, u and v should be in [0, 1) so this is just a safety check, but it might be that some rounding error makes
    // u == 1 or v == 1
    if (col >= image.width) [[unlikely]] {
      col = image.width - 1;
    }
    if (row >= image.height) [[unlikely]] {
      row = image.height - 1;
    }

    // Return the corresponding pixel color from the image
    return image.get_pixel(col, row);
  }
};

//-------------------------------------------------------------------------------------------------------------
// -----------BRDF CLASS------------------
// ------------------------------------------------------------------------------------------------------------

class BRDF {
public:
  //-------Properties--------
  std::unique_ptr<Pigment> pigment;

  //-----------Constructors-----------

  BRDF(std::unique_ptr<Pigment> pigment) : pigment{std::move(pigment)} {}

  virtual ~BRDF() {}

  //------------Methods-----------

  /// @brief Returns the BRDF integrated over r, g, b bands, that is 3 scalar values as a color object
  /// @param normal at hitting point
  /// @param incident direction
  /// @param outgoing direction
  /// @param uv coordinates of the point on the surface
  /// @details Actually this method is not used be called in path tracing: we use the BRDF to perform importance sampling instead
  virtual Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const = 0;

  /// @brief Scatters ray in random direction using BRDF-based importance sampling
  /// @param pcg used to generate random numbers for importance sampling MC
  /// @param direction of the incoming ray
  /// @param point world point where the incoming ray hits the surface
  /// @param normal to the surface at that point
  /// @param depth value for the newly scattered ray, counting how many times it has been reflected
  virtual Ray scatter_ray(PCG *pcg, Vec in_dir, Point intersection_point, Normal normal, int depth) const noexcept = 0;
};

//-------------------------------------------------------------------------------------------------------------
// -----------DIFFUSIVE BRDF  ------------------
// ------------------------------------------------------------------------------------------------------------
/// @brief BRDF for isotropic light diffusion
class DiffusiveBRDF : public BRDF {
public:
  //-------Properties--------

  //-----------Constructor-----------
  /// @param pigment of the object
  DiffusiveBRDF(std::unique_ptr<Pigment> pigment = nullptr) : BRDF{std::move(pigment)} {
    if (!this->pigment) {
      this->pigment = std::make_unique<UniformPigment>(); // if no pigment is provided, set uniform pigment (black)
    }
  };

  //------------Methods-----------

  /// @brief Evaluate the BRDF at the given point: diffusive BRDF is just the color/pigment divided by pi
  Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const override {
    return (*pigment)(uv) * (1.f / std::numbers::pi_v<float>);
  }

  Ray scatter_ray(PCG *pcg, Vec in_dir, Point intersection_point, Normal normal, int depth) const noexcept override {
    normal = normal.normalized();
    // Note that BRDF::scatter_ray methods are the only places in the code that require a normalized normal (except for
    // SpecularBRDF::eval, which is never used), so it makes sense to enforce normalization here.

    ONB onb{normal.to_vector()};
    auto [theta, phi] = pcg->random_phong(1); // Uniform BRDF makes the integrand of the rendering equation proportional to
                                              // cos(theta), hence we perform importance sampling using Phong n=1 distribution
    Vec outgoing_dir{onb.e1 * std::sin(theta) * std::cos(phi) + onb.e2 * std::sin(theta) * std::sin(phi) +
                     onb.e3 * std::cos(theta)}; // Get outgoing direction from the local ONB basis

    return Ray(intersection_point, outgoing_dir, 1.e-3f, infinity, depth);
  };
};

// ------------------------------------------------------------------------------------------------------------
// -----------SPECULAR BRDF  ------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief BRDF for ideal mirror-like surfaces
class SpecularBRDF : public BRDF {
public:
  //-------Properties--------

  //-----------Constructor-----------
  SpecularBRDF(std::unique_ptr<Pigment> pigment = nullptr) : BRDF{std::move(pigment)} {
    if (!this->pigment) {
      this->pigment =
          std::make_unique<UniformPigment>(WHITE); // if no pigment is provided, set uniform pigment WHITE for perfect mirror
    }
  }

  //------------Methods-----------

  /// @brief Evaluate the BRDF at the given point
  Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const override {
    normal = normal.normalized();
    in_dir = in_dir.normalized();
    out_dir = out_dir.normalized();

    float theta_in = std::acos(normal * -in_dir);  // incidence angle
    float theta_out = std::acos(normal * out_dir); // reflection angle

    // Apply reflection law (both angles equal and lie in the reflection plane)
    if (are_close(theta_in, theta_out) && are_close((in_dir ^ normal) * out_dir, 0) && theta_in < std::numbers::pi * 0.5f) {
      return (*pigment)(uv); // If incidence and reflection angles agree and if ray hits the surface from outside (theta_in <
                             // pi/2) return the color of the pigment at the given uv coordinates
    } else {
      return BLACK;
    }
  }

  /// @brief Deterministic perfect mirror reflection
  Ray scatter_ray(PCG *pcg, Vec in_dir, Point intersection_point, Normal normal, int depth) const noexcept override {
    in_dir = in_dir.normalized();
    Vec n = normal.normalized().to_vector();
    // Note that BRDF::scatter_ray methods are the only places in the code that require a normalized normal (except for
    // SpecularBRDF::eval, which is never used), so it makes sense to enforce normalization here

    Vec reflected = in_dir - n * 2.f * (n * in_dir);

    return Ray(intersection_point, reflected, 1.e-5f, infinity, depth);
  }
};

//-------------------------------------------------------------------------------------------------------------
// -----------MATERIAL CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Light emissive and reflective properties of a shape object as a function of (u, v) coordinates on the surface
class Material {
public:
  //-------Properties--------
  std::unique_ptr<BRDF> brdf;
  std::unique_ptr<Pigment> emitted_radiance; // Pigment that describes the emitted radiance of the material, if any

  //-----------Constructors-----------
  Material(std::unique_ptr<BRDF> brdf = nullptr, std::unique_ptr<Pigment> emitted_radiance = nullptr)
      : brdf{std::move(brdf)}, emitted_radiance{std::move(emitted_radiance)} {
    if (!this->brdf) {
      this->brdf = std::make_unique<DiffusiveBRDF>(); // If no BRDF is provided, set diffusive BRDF with uniform pigment black
    }
    if (!this->emitted_radiance) {
      this->emitted_radiance =
          std::make_unique<UniformPigment>(); // If no emitted radiance is provided, set uniform pigment black
    }
  }
};

/// @brief Build default material
/// @details Use where a shape's material doesn't matter, e. g. in tests (a material is always required)
inline Material make_neutral_material() {
  return Material{std::make_unique<DiffusiveBRDF>(std::make_unique<UniformPigment>(WHITE)),
                  std::make_unique<UniformPigment>(BLACK)};
}
