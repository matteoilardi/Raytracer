// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR RENDERERS -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include "materials.hpp"
#include "shapes.hpp"
#include <algorithm> // needed for std::max

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class Renderer;

class OnOffTracer;
class FlatTracer;
class PathTracer;

// ------------------------------------------------------------------------------------------------------------
// --------------- RENDERER ABSTRACT CLASS -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief abstract functor that associates a Color to a Ray
class Renderer {
public:
  //------------Properties-----------
  std::shared_ptr<World> world; // world to render
  Color background;             // background color

  //-----------Constructors-----------
  /// Constructor with parameters
  Renderer(std::shared_ptr<World> world, Color background = Color()) : world(world), background(background) {};

  //------------Methods-----------
  virtual Color operator()(Ray ray) const = 0;
};

// ------------------------------------------------------------------------------------------------------------
// --------------- ON_OFF RENDERER -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief functor performing ON/OFF tracing: returns white if the ray hits any object, black otherwise
class OnOffTracer : public Renderer {
public:
  //------------Properties-----------

  //-----------Constructors-----------
  /// Constructor with parameters
  /// @param world to render
  OnOffTracer(std::shared_ptr<World> world) : Renderer(world) {};

  //------------Methods-----------
  virtual Color operator()(Ray ray) const {
    if (world->on_off_ray_intersection(
            ray)) { // use ad hoc implemented on_off_ray_intersection method to stop looping over objects as soon as one is hit
      return WHITE; // return white if any object is hit
    } else {
      return Color(); // return black if no object is hit
    }
  }
};

//----------------------------------------------------------------------------------------------------------------
// --------------- FLAT RENDERER -------------------------------------------------
//----------------------------------------------------------------------------------------------------------------

/// @brief functor performing flat tracing, i. e. returning for each ray just the plain color of the closest object hit
class FlatTracer : public Renderer {
public:
  //------------Properties-----------

  //-----------Constructors-----------
  /// Constructor with parameters
  /// @param world to render
  /// @param background color
  FlatTracer(std::shared_ptr<World> world, Color background = Color()) : Renderer(world, background) {};

  //------------Methods-----------
  virtual Color operator()(Ray ray) const {
    std::optional<HitRecord> hit = world->ray_intersection(ray); // save the closest hit (if any)
    if (!hit) {
      return background; // Return background color if no object is hit
    }
    // Return the color of the closest object hit (if any), both the brdf pigment and the emitted radiance (see Tomasi's Pytracer)
    return (*(hit->shape->material->brdf->pigment))(hit->surface_point) +
           (*(hit->shape->material->emitted_radiance))(hit->surface_point);
  };
};

// ------------------------------------------------------------------------------------------------------------
// --------------- PATH TRACER -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief functor performing the path tracing algorithm
/// @details importance sampling in MC integration based on the scatter_ray method of the BRDF
class PathTracer : public Renderer {
public:
  //-------Properties--------
  std::shared_ptr<PCG> pcg; // random number generator
  int n_rays;               // number of rays recursively scattered
  int russian_roulette_lim; // minimum ray depth before russian roulette starts applying
  int max_depth;            // maximum ray depth

  //-----------Constructors-----------
  // TODO choose reasonable value for n_rays ()
  // ANSWER I would rather set 10 as default for n_rays(), also shouldn't max_depth default value be biggger then
  // russian_roulette_lim defualt value?

  /// @brief constructor with parameters
  /// @param world to render
  /// @param pcg random number generator
  /// @param number of rays recursively scattered
  /// @param ray depth before russian roulette kicks in
  /// @param maximum ray depth
  /// @param background color
  PathTracer(std::shared_ptr<World> world, std::shared_ptr<PCG> pcg = nullptr, int n_rays = 10, int russian_roulette_lim = 2,
             int max_depth = 4, Color background = Color())
      : Renderer(world, background), pcg(pcg), n_rays(n_rays), russian_roulette_lim(russian_roulette_lim), max_depth(max_depth) {
    if (!this->pcg) {
      pcg = std::make_shared<PCG>();
    }
  };

  //------------Methods-----------
  virtual Color operator()(Ray ray) const {
    // 1. Stop recursion if maximum depth is reached
    if (ray.depth > max_depth) {
      return Color();
    }

    // 2. Get closest intersection of ray with world objects or return background Color if no object is hit
    std::optional<HitRecord> hit = world->ray_intersection(ray);
    if (!hit) {
      return background;
    }

    // 3. Unpack hit
    std::shared_ptr<Material> hit_material = hit->shape->material;
    Color reflected_color = (*(hit_material->brdf->pigment))(hit->surface_point); // both pigment and emitted_radiance are
                                                                                  // pointers
    Color emitted_radiance = (*(hit_material->emitted_radiance))(hit->surface_point);

    // 4. Apply russian roulette: decide whether to scatter new rays and rescale the BRDF to compesate for possible
    // truncations and get unbiased expected value
    float hit_lum = std::max({reflected_color.r, reflected_color.g, reflected_color.b});
    if (ray.depth > russian_roulette_lim) {
      float q = std::max(1.f - hit_lum, 0.05f);
      // according to Shirley & Morley, use max reflectance of hit color as Russian roulette probability
      if (pcg->random_float() > q) {
        // stop with higher probability if the hit point has low reflactance: this improves efficiency without increasing variance
        // too much Keep a finite stopping probability 0.05 even if hit_lum is close to 1
        reflected_color = reflected_color * (1.f / (1.f - q));
      } else {
        return emitted_radiance;
      }
    }

    // 5. Calculate reflected radiance recursively by: a) scattering rays in random directions according to the BRDF; b)
    // averaging the radiance from corresponding directions; c) multiplying by reflected color
    // NOTE the algorithm is correct for a diffusiveBRDF provided reflected_color is rho_d for the three bands. This is
    // beacuse the normalization of the diffusiveBRDF exactly cancels out the normalization of the n=1 Phong
    // distribution. A further multiplicative factor should probably be supplied for other BRDFs: remember to do the
    // maths ad check.
    Color cum_radiance = Color();
    if (hit_lum > 0.f) { // proceed with recursion only if hit object is not perfectly absorbing (i.e. hit_lum==0)
      for (int i_ray = 0; i_ray < n_rays; i_ray++) {
        Ray new_ray = hit_material->brdf->scatter_ray(
            pcg, ray.direction, hit->world_point, hit->normal,
            ray.depth + 1); // Scatter a new ray by sampling the solid angle distribution proportional to the BRDF
        cum_radiance =
            cum_radiance +
            (*this)(new_ray); // dereference pointer `this' to the PathTracer class object and recurively call its operator()
      }
    }
    cum_radiance = cum_radiance * (1.f / n_rays) * reflected_color;

    // 6. Add emitted radiance specific of the object
    cum_radiance = cum_radiance + emitted_radiance;

    return cum_radiance;
  }
};
