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

class Material;

//-------------------------------------------------------------------------------------------------------------
// -----------PIGMENT STRUCT------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief abstract functor that associates a Color to a Vec2d
struct Pigment {

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const = 0;
};

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

/// @brief return checkered pattern of two Colors
struct CheckeredPigment : public Pigment {

  //-------Properties--------
  Color color1, color2;
  int n_intervals; // number of intervals into which the range [0, 1] is divided for u and v

  //-----------Constructor-----------
  CheckeredPigment(Color color1 = Color(), Color color2 = Color(), int n_intervals = 10)
      : Pigment(), color1(color1), color2(color2), n_intervals(n_intervals) {};

  //------------Methods-----------
  virtual Color operator()(Vec2d uv) const {
    float subinterval = 1.f / n_intervals; // length of u and v subintervals

    int col = std::floor(uv.u / subinterval);
    int row = std::floor(uv.v / subinterval);
    if ((col + row) % 2 == 0) { // top-left rectangle has color1
      return color1;
    } else {
      return color2;
    }
  };
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
  /// @brief returns a Color corresponding to the BRDF integrated over r, g, b bands
  /// @param normal at hitting point
  /// @param incident direction
  /// @param outgoing direction
  /// @ (u, v) coordinates
  virtual Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const = 0;

  /// @brief scatters ray in random direction using BRDF-based importance sampling
  /// @param used to generate random numbers
  /// @param direction of the incolming ray
  /// @param point where the incoming ray hits the surface
  /// @param normal to the surface at that point
  /// @param depth value for the newly scattered ray
  virtual Ray scatter_ray(std::shared_ptr<PCG> pcg, Vec incoming_dir, Point intersection_point, Normal normal,
                          int depth) const = 0;
};

/// @brief BRDF for isotropic light diffusion
class DiffusiveBRDF : public BRDF {
public:
  //-------Properties--------
  float reflectance; // reflectance of the object
  // QUESTION isn't reflactance already encoded in the pigment (three reflectances, one for each band)???

  //-----------Constructor-----------
  /// @param reflectance of the object
  /// @param pigment of the object
  DiffusiveBRDF(std::shared_ptr<Pigment> pigment = nullptr, float reflectance = 1.f)
      : BRDF(pigment), reflectance(reflectance) {
    if (!this->pigment) {
      this->pigment = std::make_shared<UniformPigment>();
    }
  };

  //------------Methods-----------
  Color eval(Normal normal, Vec in_dir, Vec out_dir, Vec2d uv) const override {
    return (*pigment)(uv)*reflectance * (1.f / std::numbers::pi);
  }

  Ray scatter_ray(std::shared_ptr<PCG> pcg, Vec incoming_dir, Point intersection_point, Normal normal,
                  int depth) const override {
    normal.normalize(); // QUESTION is it necessary?
    ONB onb{normal.to_vector()};
    auto [theta, phi] =
        pcg->random_phong(1); // uniform BRDF makes the integrand of the rendering equation proportional to cos(theta),
                              // hence we perform importance sampling using Phong n=1 distribution
    Vec outgoing_dir{onb.e1 * std::sin(theta) * std::cos(phi) + onb.e2 * std::sin(theta) * std::sin(phi) +
                     onb.e3 * std::cos(theta)};

    // QUESTION why should tmin be bigger than usual? see lab 11 slide 13
    return Ray(intersection_point, outgoing_dir, 1.e-3f, infinite, depth);
  };
};

//-------------------------------------------------------------------------------------------------------------
// -----------MATERIAL CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief light emitting and reflective properties of an object as a function of (u, v)
class Material {
public:
  //-------Properties--------
  std::shared_ptr<BRDF> brdf;
  std::shared_ptr<Pigment> emitted_radiance;

  //-----------Constructors-----------
  Material(std::shared_ptr<BRDF> brdf = nullptr, std::shared_ptr<Pigment> emitted_radiance = nullptr)
      : brdf(brdf), emitted_radiance(emitted_radiance) {
    if (!this->brdf) {
      this->brdf = std::make_shared<DiffusiveBRDF>();
    }
    if (!this->emitted_radiance) {
      this->emitted_radiance = std::make_shared<UniformPigment>();
    }
  }

  //------------Methods-----------
};