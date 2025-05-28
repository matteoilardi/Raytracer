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
class FlatTracer;
class PointLightTracer;
class PathTracer;

/// @brief abstract functor that associates a Color to a Ray
class Renderer {
public:
  //------------Properties-----------
  std::shared_ptr<World> world; // world to render
  Color background_color;       // background color

  //-----------Constructors-----------
  Renderer(std::shared_ptr<World> world, Color background_color = Color()) : world(world), background_color(background_color) {};

  //------------Methods-----------
  virtual Color operator()(Ray ray) const = 0;
};

/// @brief functor performing flat tracing, i. e. returning for each ray the color of the closest object to be hit
class FlatTracer : public Renderer {
public:
  //------------Properties-----------

  //-----------Constructors-----------
  /// Constructor with parameters
  /// @param world to render
  /// @param background color
  FlatTracer(std::shared_ptr<World> world, Color background_color = Color()) : Renderer(world, background_color) {};

  //------------Methods-----------
  virtual Color operator()(Ray ray) const {
    // Save the colosest hit or return background Color if no object gets hit
    std::optional<HitRecord> hit = world->ray_intersection(ray);
    if (!hit) {
      return background_color;
    }

    // Return Color of the hit object
    return (*hit->shape->material->brdf->pigment)(hit->surface_point);
  };
};

/// @brief functor performing point light tracing (see PR #11)
class PointLightTracer : public Renderer {
public:
  //------------Properties-----------
  Color ambient_color; // constant base illumination applied to all surfaces (used when no light source is in sight)

  //-----------Constructors-----------
  /// Constructor with parameters
  /// @param world to render
  /// @param ambient color
  /// @param background color
  PointLightTracer(std::shared_ptr<World> world, Color ambient_color = Color(), Color background_color = Color())
      : Renderer(world, background_color), ambient_color(ambient_color) {};

  //------------Methods-----------
  virtual Color operator()(Ray ray) const {
    // Forward declarations
    std::optional<HitRecord> hit;
    std::shared_ptr<Material> hit_material;
    std::shared_ptr<BRDF> brdf;

    do {
      // 1. Save the closest hit or return background Color if no object gets hit
      hit = world->ray_intersection(ray);
      if (!hit.has_value()) {
        return background_color;
      }

      // 2. Unpack hit
      hit_material = hit->shape->material;
      brdf = hit_material->brdf;

      auto specular = false; // delete this line and restore the next one when implementation of SpecularBRDF is ready
      // auto specular = std::dynamic_pointer_cast<SpecularBRDF>(brdf); // returns nullptr if brdf is not a pointer to an object of the derived class SpecularBRDF
      // 3. In case you hit an object with SpecularBRDF, scatter a new Ray from the hit point in the direction given by the reflection law; otherwise go ahead with color evaluation
      if (!specular) {
              break;
      }

      Vec new_dir = ray.direction - 2*hit->normal.to_vector() * (hit->normal.to_vector()*ray.direction);
      ray = Ray(hit->world_point, new_dir);

    } while (true);

    // 4. Initalize pixel color
    Color cum_radiance =
        ambient_color +
        (*hit_material->emitted_radiance)(
            hit->surface_point); // QUESTION emitted radiance is summed later and rescaled in pytracer: why?

    // 5. Loop over point light sources and add a contribution to radiance if the light source is visible
    for (auto source : world->light_sources) {
      std::optional<Vec> in_dir = world->offset_if_visible(source->point, hit->world_point, hit->normal);

      if (in_dir.has_value()) {
        float distance = in_dir->norm();
        float distance_factor =
            (source->emission_radius > 0.f) ? std::pow((source->emission_radius / distance), 2) : 1.f;
        // angle between normal at hitting point and incoming direction (from light source)
        float cos_theta = (-1.f / distance) * (*in_dir) * (1.f / hit->normal.norm()) * hit->normal.to_vector();
        cum_radiance += source->color * distance_factor * cos_theta *
                        brdf->eval(hit->normal, *in_dir, -hit->ray.direction, hit->surface_point);
      }
    }
    return cum_radiance;
  };
};

/// @brief functor performing the path tracing algorithm
/// @details importance sampling in MC integration based on the scatter_ray method of the BRDF
class PathTracer : public Renderer {
public:
  //-------Properties--------
  std::shared_ptr<PCG> pcg; // random number generator
  int n_rays;               // number of rays recursively scattered
  int russian_roulette_lim; // maximum ray depth before russian roulette kicks in
  int max_depth;            // maximum ray depth

  //-----------Constructors-----------
  // TODO choose reasonable value for n_rays

  /// @brief constructor with parameters
  /// @param world to render
  /// @param pcg
  /// @param number of rays recursively scattered
  /// @param ray depth before russian roulette kicks in
  /// @param maximum ray depth
  /// @param background color
  PathTracer(std::shared_ptr<World> world, std::shared_ptr<PCG> pcg = nullptr, int n_rays = 100,
             int russian_roulette_lim = 2, int max_depth = 2, Color background = Color())
      : Renderer(world, background), pcg(pcg), n_rays(n_rays), russian_roulette_lim(russian_roulette_lim),
        max_depth(max_depth) {
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

    // 2. Return background Color if no object gets hit
    std::optional<HitRecord> hit = world->ray_intersection(ray);
    if (!hit) {
      return background_color;
    }

    // 3. Unpack hit
    std::shared_ptr<Material> hit_material = hit->shape->material;
    Color reflected_color = (*hit_material->brdf->pigment)(hit->surface_point);
    Color emitted_radiance = (*hit_material->emitted_radiance)(hit->surface_point);

    // 4. Apply russian roulette: decide whether to scatter new rays and renormalize the BRDF to compesate for possible
    // truncations
    float hit_lum = std::max({reflected_color.r, reflected_color.g, reflected_color.b});
    if (ray.depth > russian_roulette_lim) {
      float q = std::max(1.f - hit_lum, 0.05f);
      if (pcg->random_float() >
          q) { // stop with higher probability if the hit point has low reflactance: this improves efficiency without
               // penalizing variance too much. Keep a finite stopiing probability even if hit_lum is close to 1
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
    if (hit_lum > 0.f) { // proceed with recursion only if reflection is possible
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
