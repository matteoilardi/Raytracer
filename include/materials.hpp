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

/// @brief abstract functor that associates a Color to a Vec2d
struct Pigment {

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const = 0;
};

//-------------------------------------------------------------------------
//---------------------- UNIFORM PIGMENT STRUCT ------------------
//-------------------------------------------------------------------------
/// @brief returns constant Color
struct UniformPigment : public Pigment {

  //-------Properties--------
  Color color;

  //-----------Constructors-----------

  /// Default constructor
  UniformPigment() : Pigment(), color() {};

  /// Constructor with parameters
  UniformPigment(Color color) : Pigment(), color(color) {};

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const override { return color; };
};

//-------------------------------------------------------------------------
//---------------------- CHECKERED PIGMENT STRUCT ------------------
//-------------------------------------------------------------------------

/// @brief return checkered pattern of two Colors
struct CheckeredPigment : public Pigment {

  //-------Properties--------
  Color color1, color2;
  int n_intervals; // number of intervals into which the range [0, 1] is divided for u and v cohordinates

  //-----------Constructor-----------
  CheckeredPigment(Color color1 = Color(), Color color2 = Color(), int n_intervals = 10)
      : Pigment(), color1(color1), color2(color2), n_intervals(n_intervals) {};

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const {
    float subinterval = 1.f / n_intervals; // length of u and v subintervals

    // get the column and row of the subinterval in which the (u, v) coordinates fall
    int col = std::floor(uv.u / subinterval);
    int row = std::floor(uv.v / subinterval);
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
/// @brief pigment obtained by wrapping an HDR image (pfm format) around the given shape
/// @brief uv coordinates on the surface are mapped to column and row of the image and the corresponding pixel color is returned
class ImagePigment : public Pigment {
public:
  // ------- Properties -------
  HdrImage image; // HDR image used as a texture to be wrapped around the shape

  // ------- Constructors -------
  /// @brief Construct an ImagePigment from a given HdrImage object
  /// @param image HdrImage object containing the HDR image
  ImagePigment(const HdrImage &image) : image(image) {}

  /// @brief Construct an ImagePigment from a given PFM image file
  /// @param filename path to the pfm file containing the HDR image
  ImagePigment(const std::string &filename) : image(filename) {}

  // ------- Methods -------
  /// @brief Given surface UV coordinates, return the corresponding Color from the texture
  /// @param uv coordinates in [0, 1)^2 identifying a point on the surface
  /// @return color extracted from the HDR image at that position
  virtual Color operator()(Vec2d uv) const override {
    // Convert UV ∈ [0,1) to pixel indices in the image (truncating to floor integer)
    int col = static_cast<int>(uv.u * image.width);
    int row = static_cast<int>(uv.v * image.height);

    // Clamp indices to avoid potential out-of-bounds (only needed if u or v == 1.0)
    // TODO Technically, uv should be in [0, 1) so this is just a safety check, but it might be that some rounding error makes u
    // or v=1 We should check if these 'if checks' slow down the code significantly, if so, we can remove them
    if (col >= image.width)
      col = image.width - 1;
    if (row >= image.height)
      row = image.height - 1;

    if (col < 10 && row < 10) {
      (image.get_pixel(col, row)).display(); // TODO remove this debug print
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
  std::shared_ptr<Pigment> pigment;

  //-----------Constructors-----------

  BRDF(std::shared_ptr<Pigment> pigment) : pigment(pigment) {};

  //------------Methods-----------

  /// @brief returns the BRDF integrated over r, g, b bands, that is 3 scalar values as a color object
  /// @param normal at hitting point
  /// @param incident direction
  /// @param outgoing direction
  /// @param uv coordinates of the point on the surface
  virtual Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const = 0;
  // TODO the virtual method implemented in BRDF parent class passes arguments by direct value, wouldn't it be better to
  // pass them by const reference?

  /// @brief scatters ray in random direction using BRDF-based importance sampling
  /// @param pcg used to generate random numbers for importance sampling MC
  /// @param direction of the incoming ray
  /// @param point world point where the incoming ray hits the surface
  /// @param normal to the surface at that point
  /// @param depth value for the newly scattered ray, counting how many times it has been reflected
  virtual Ray scatter_ray(std::shared_ptr<PCG> pcg, Vec incoming_dir, Point intersection_point, Normal normal,
                          int depth) const = 0;
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
  DiffusiveBRDF(std::shared_ptr<Pigment> pigment = nullptr) : BRDF(pigment) {
    if (!this->pigment) {
      this->pigment = std::make_shared<UniformPigment>(); // if no pigment is provided, set uniform pigment (black)
    }
  };

  //------------Methods-----------

  /// @brief evaluate the BRDF at the given point, by definition diffusive BRDF is just the color/pigment divided by pi
  // in fact this method will not be used since BRDF simplifies in importance sampling for MC pathtracing
  Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const override {
    // TODO the virtual method implemented in BRDF parent class passes arguments by direct value, wouldn't it be better to
    // pass them by const reference?
    return (*pigment)(uv) * (1.f / std::numbers::pi); // dereference pigment pointer, get color at uv coordinates, divide by pi
  }

  Ray scatter_ray(std::shared_ptr<PCG> pcg, Vec incoming_dir, Point intersection_point, Normal normal, int depth) const override {
    normal = normal.normalize(); // QUESTION is it necessary?
    ONB onb{normal.to_vector()};
    auto [theta, phi] = pcg->random_phong(1); // uniform BRDF makes the integrand of the rendering equation proportional to
                                              // cos(theta), hence we perform importance sampling using Phong n=1 distribution
    Vec outgoing_dir{onb.e1 * std::sin(theta) * std::cos(phi) + onb.e2 * std::sin(theta) * std::sin(phi) +
                     onb.e3 * std::cos(theta)}; // get outgoing direction from the local ONB basis

    return Ray(intersection_point, outgoing_dir, 1.e-3f, infinite, depth);
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
  SpecularBRDF(std::shared_ptr<Pigment> pigment = nullptr) : BRDF(pigment) {
    if (!this->pigment) {
      this->pigment = std::make_shared<UniformPigment>(
          Color(1.f, 1.f, 1.f)); // if no pigment is provided, set uniform pigment WHITE for perfect mirror
    }
  }

  //------------Methods-----------

  /// @brief Evaluate the BRDF at the given point
  // in fact this method will not be used since BRDF simplifies in importance sampling for MC pathtracing
  Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const override {
    // TODO the virtual method implemented in BRDF parent class passes arguments by direct value, wouldn't it be better to
    // pass them by const reference?
    Normal n = normal.normalize();
    Vec in = in_dir.normalize();
    Vec out = out_dir.normalize();

    float theta_in = std::acos(n * -in);  // incidence angle
    float theta_out = std::acos(n * out); // reflection angle

    // check the reflection law (both angles equal and lie in the reflection plane)
    if (are_close(theta_in, theta_out) && are_close((in ^ n) * out, 0) && theta_in < std::numbers::pi * 0.5f) {
      return (*pigment)(uv); // if incidence and reflection angles agree and if ray hits the surface from outside (theta_in <
                             // pi/2) return the color of the pigment at the given uv coordinates
    } else {
      return Color(0.f, 0.f, 0.f); // otherwise return black
    }
  }

  /// @brief deterministic perfect mirror reflection
  Ray scatter_ray(std::shared_ptr<PCG> pcg, Vec incoming_dir, Point intersection_point, Normal normal, int depth) const override {
    Vec in = incoming_dir.normalize(); // TODO just like for diffusiveBRDF scatter ray, make sure we normalize only once be it
                                       // here or somewhereelse
    Vec n = (normal.to_vector()).normalize();
    // n.normalize();

    Vec reflected = in - n * 2.f * (n * in);

    return Ray(intersection_point, reflected, 1.e-5f, infinite, depth);
  }
};

//-------------------------------------------------------------------------------------------------------------
// -----------MATERIAL CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief light emissive and reflective properties of a shape object as a function of (u, v) coordinates on the surface
class Material {
public:
  //-------Properties--------
  std::shared_ptr<BRDF> brdf;
  std::shared_ptr<Pigment> emitted_radiance; // Pigment that describes the emitted radiance of the material, if any

  //-----------Constructors-----------
  Material(std::shared_ptr<BRDF> brdf = nullptr, std::shared_ptr<Pigment> emitted_radiance = nullptr)
      : brdf(brdf), emitted_radiance(emitted_radiance) {
    if (!this->brdf) {
      this->brdf = std::make_shared<DiffusiveBRDF>(); // if no BRDF is provided, set diffusive BRDF with uniform pigment black
    }
    if (!this->emitted_radiance) {
      this->emitted_radiance =
          std::make_shared<UniformPigment>(); // if no emitted radiance is provided, set uniform pigment black
    }
  }

  // Costructor with only emitted radiance
  Material(std::shared_ptr<Pigment> emitted_radiance) : Material(nullptr, emitted_radiance) {}

  //------------Methods-----------
};
