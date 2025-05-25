// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS RENDERERS
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// -------------LIBRARIES---------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#include "cameras.hpp"
#include "geometry.hpp"
#include "materials.hpp"
#include "renderers.hpp"
#include "shapes.hpp"
#include <gtest/gtest.h>

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR PATH TRACING-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// Furnace test: cast a ray inside a closed surface with diffusive BRDF and uniform reflectance rho_d and emitted
// radiance L_e; scatter exactly one ray at every hit (every ray should yield the same contribution regardless of its
// direction); stop after a large number of reflections; compare with the analytical solution: L = L_e/(1 - rho_d),
// which arises from a geometric sum; repeat 200 times for random valued of reflectance and emitted radiance
TEST(PathTracingTest, test_furnace) {

  // 1. Build material
  auto pigment = std::make_shared<UniformPigment>();
  auto brdf = std::make_shared<DiffusiveBRDF>(pigment);
  auto emitted_radiance = std::make_shared<UniformPigment>();

  auto material = std::make_shared<Material>(brdf, emitted_radiance);

  // 2. Build enclosing sphere and world
  auto enclosure = std::make_shared<Sphere>(Transformation(), material);

  auto world = std::make_shared<World>();
  world->add_object(enclosure);

  // 3. Cast ray and compute total radiance
  Ray ray{Point(), VEC_X};
  auto pcg = std::make_shared<PCG>();
  PathTracing renderer{world, pcg, 1, 200, 200}; // russian roulette is not invoked up to depth=200, which is the max depth

  // 4. Compute total radiance and compare with expected result
  for (int i = 0; i < 100; i++) {
    float reflectance = pcg->random_float() * 0.9f; // avoid rho_d = 1
    float luminosity = pcg->random_float();
    pigment->color = Color(reflectance, 0.f, 0.f);
    emitted_radiance->color = Color(luminosity, 0.f, 0.f);

    Color total_radiance = renderer(ray);
    Color expected{luminosity / (1.f - reflectance), 0.f, 0.f};
    EXPECT_TRUE(total_radiance.is_close_to(expected));
  }
}
