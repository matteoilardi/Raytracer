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

class Render;
class PathTriacing;

/// @brief abstract functor that associates a Color to a Ray
class Renderer {
public:
  //------------Methods-----------
  virtual Color operator()(Ray ray) const = 0;
};

/// @brief functor performing the path tracing algorithm
/// @details importance sampling in MC integration based on the scatter_ray method of the BRDF
class PathTracing : public Renderer {
public:
  //-------Properties--------
  std::shared_ptr<World> world; // world to render
  std::shared_ptr<PCG> pcg;     // random number generator
  int n_rays;                   // number of rays recursively scattered
  int russian_roulette_lim;     // maximum ray depth before russian roulette kicks in
  int max_depth;                // maximum ray depth
  Color background;             // background color

  //-----------Constructors-----------

  /// Constructor with parameters
  // TODO choose reasonable value for n_rays
  PathTracing(std::shared_ptr<World> world, std::shared_ptr<PCG> pcg = nullptr, int n_rays = 100,
              int russian_roulette_lim = 2, int max_depth = 2, Color background = Color())
      : world(world), pcg(pcg), n_rays(n_rays), russian_roulette_lim(russian_roulette_lim), max_depth(max_depth),
        background(background) {
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

    // 2.Return background Color if no object gets hit
    std::optional<HitRecord> hit = world->ray_intersection(ray);
    if (!hit) {
      return background;
    }

    // 3. Unpack hit
    std::shared_ptr<Material> hit_material = hit->shape->material;
    Color reflected_color = (*hit_material->brdf->pigment)(hit->surface_point);
    Color emitted_radiance = (*hit_material->emitted_radiance)(hit->surface_point);

    // 4. Apply russian roulette: decide whether to scatter new rays and renormalize the BRDF to compesate for possible
    // truncations
    float q = std::max({reflected_color.r, reflected_color.g,
                        reflected_color.b}); // stopping probability <-> reflected_color luminosity
    // QUESTION why?
    if (ray.depth > russian_roulette_lim) {
      if (pcg->random_float() > q) {
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
    if (q > 0.f) { // proceed with recursion only if reflection is possible
      for (int i_ray = 0; i_ray < n_rays; i_ray++) {
        Ray new_ray = hit_material->brdf->scatter_ray(
            pcg, ray.direction, hit->world_point, hit->normal,
            ray.depth + 1); // Scatter a new ray by sampling the solid angle distribution proportional to the BRDF
        cum_radiance = cum_radiance + (*this)(new_ray);
      }
    }
    cum_radiance = cum_radiance * (1.f / n_rays) * reflected_color;

    // 6. Add emitted radiance
    cum_radiance = cum_radiance + emitted_radiance;

    return cum_radiance;
  }
};

// TODO on/off tracing is currently implemented as a method of the class World and as such it requires a lambda wrapping
// inside the main, which is not very elegant. Consider moving it here a class derived from Renderer.
