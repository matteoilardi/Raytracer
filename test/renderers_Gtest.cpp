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

// Test on/off tracing
TEST(OnOffTracerTest, test_example) {
  auto img = std::make_unique<HdrImage>(3, 3);
  auto cam = std::make_shared<OrthogonalCamera>();
  ImageTracer tracer(std::move(img), cam);

  World world{};
  auto pigment = std::make_unique<UniformPigment>(Color(1.f, 1.f, 1.f));
  Material material{std::make_unique<DiffusiveBRDF>(std::move(pigment))};

  auto sphere = std::make_unique<Sphere>(translation(Vec(2.f, 0.f, 0.f)) * scaling({0.2f, 0.2f, 0.2f}), material);
  world.add_object(std::move(sphere));

  tracer.fire_all_rays(OnOffTracer(world));

  EXPECT_TRUE(tracer.image->get_pixel(0, 0).is_close(Color()));
  EXPECT_TRUE(tracer.image->get_pixel(1, 0).is_close(Color()));
  EXPECT_TRUE(tracer.image->get_pixel(2, 0).is_close(Color()));

  EXPECT_TRUE(tracer.image->get_pixel(0, 1).is_close(Color()));
  EXPECT_TRUE(tracer.image->get_pixel(1, 1).is_close(Color(1.f, 1.f, 1.f)));
  EXPECT_TRUE(tracer.image->get_pixel(2, 1).is_close(Color()));

  EXPECT_TRUE(tracer.image->get_pixel(0, 2).is_close(Color()));
  EXPECT_TRUE(tracer.image->get_pixel(1, 2).is_close(Color()));
  EXPECT_TRUE(tracer.image->get_pixel(2, 2).is_close(Color()));
}

// Test flat renderer
TEST(FlatTracerTest, test_example) {
  Color sphere_color{1.f, 2.f, 3.f};
  auto pigment = std::make_unique<UniformPigment>(sphere_color);
  auto brdf = std::make_unique<DiffusiveBRDF>(std::move(pigment));
  Material material{std::move(brdf)};

  // Move the sphere to the center of the screen and make it small enough to cover only the central pixel
  auto sphere = std::make_unique<Sphere>(translation(Vec(2.f, 0.f, 0.f)) * scaling({0.2f, 0.2f, 0.2f}), material);

  World world{};
  world.add_object(std::move(sphere));
  FlatTracer renderer{world, Color()};

  auto img = std::make_unique<HdrImage>(3, 3);
  auto cam = std::make_shared<OrthogonalCamera>();
  ImageTracer tracer{std::move(img), cam};

  tracer.fire_all_rays(renderer);

  EXPECT_TRUE(tracer.image->get_pixel(0, 0).is_close(BLACK));
  EXPECT_TRUE(tracer.image->get_pixel(0, 1).is_close(BLACK));
  EXPECT_TRUE(tracer.image->get_pixel(0, 2).is_close(BLACK));

  EXPECT_TRUE(tracer.image->get_pixel(1, 0).is_close(BLACK));
  EXPECT_TRUE(tracer.image->get_pixel(1, 1).is_close(sphere_color));
  EXPECT_TRUE(tracer.image->get_pixel(1, 2).is_close(BLACK));

  EXPECT_TRUE(tracer.image->get_pixel(2, 0).is_close(BLACK));
  EXPECT_TRUE(tracer.image->get_pixel(2, 1).is_close(BLACK));
  EXPECT_TRUE(tracer.image->get_pixel(2, 2).is_close(BLACK));
}

// Test point light tracing
TEST(PointLightTracer, test_example) {
  // A single ray is scattered along the x axis
  auto img = std::make_unique<HdrImage>(1, 1);
  auto cam = std::make_shared<OrthogonalCamera>();
  ImageTracer tracer{std::move(img), cam, 1};

  // The ray intersects the plane 1 at (1, 0, 0)
  auto plane_pigment = std::make_unique<UniformPigment>(Color(0.2f, 0.f, 0.f));
  auto plane_brdf = std::make_unique<DiffusiveBRDF>(std::move(plane_pigment));
  auto plane_emitted_radiance = std::make_unique<UniformPigment>(Color(0.f, 0.3f, 0.f));
  Material plane_material{std::move(plane_brdf), std::move(plane_emitted_radiance)};

  World world{};
  auto plane1 = std::make_unique<Plane>(translation(VEC_X) * rotation_y(-std::numbers::pi_v<float> / 2.f), plane_material);
  auto plane2 = std::make_unique<Plane>(translation(VEC_Y) * rotation_x(std::numbers::pi_v<float> / 2.f), plane_material);
  world.add_object(std::move(plane1));
  world.add_object(std::move(plane2));

  // The first light source is behind plane 2, the other two are visible from point (1, 0, 0)
  world.add_light_source(std::make_unique<PointLightSource>(Point(0.f, 2.f, 0.f))); // default Color(1.f, 1.f, 1.f)
  world.add_light_source(std::make_unique<PointLightSource>(Point(0.f, -2.f, 0.f)));
  world.add_light_source(std::make_unique<PointLightSource>(Point(0.f, -3.f, 0.f)));

  PointLightTracer renderer{world, Color(0.f, 0.f, 0.1f)}; // Provided ambient color, default background color is black
  tracer.fire_all_rays(renderer);

  // Expected r component: cos_theta * brdf_r_component * light_source_color (= 1) / pi for each visible source
  Color expected_color = Color(0.f, 0.3f, 0.1f) +
                         (1.f / std::sqrtf(5.f) + 1.f / std::sqrtf(10.f)) * Color(0.2f, 0.f, 0.f) / std::numbers::pi_v<float>;
  EXPECT_TRUE(tracer.image->get_pixel(0, 0).is_close(expected_color));
}

// Test reflective BRDFs are handled corrctly in point light tracing
TEST(PointLightTracer, test_reflections) {
  auto make_black_pigment = []() { return std::make_unique<UniformPigment>(BLACK); };
  auto make_grey_pigment = []() { return std::make_unique<UniformPigment>(Color(0.5f, 0.5f, 0.5f)); };

  // Mirror material
  auto mirror_brdf = std::make_unique<SpecularBRDF>(make_grey_pigment());
  Material mirror_material{std::move(mirror_brdf), make_black_pigment()};

  // Material for grey surface
  auto grey_brdf = std::make_unique<DiffusiveBRDF>(make_grey_pigment());
  Material grey_material{std::move(grey_brdf), make_black_pigment()};

  // Material for the non-reflective and non-emitting sphere
  auto sphere_brdf = std::make_unique<DiffusiveBRDF>(make_black_pigment());
  Material sphere_material{std::move(sphere_brdf), make_black_pigment()};

  Ray ray{Point(), VEC_X};
  auto light_source = std::make_unique<PointLightSource>(Point(-1.f, 0.f, 0.f));
  auto sphere = std::make_unique<Sphere>(translation(Vec(-0.5f, 0.f, 0.f)) * scaling({0.1f, 0.1f, 0.1f}), sphere_material);
  World world{};
  world.add_object(std::move(sphere));
  world.add_light_source(std::move(light_source));
  Color ambient_color = Color(0.f, 0.f, 0.1f);
  PointLightTracer renderer{world, ambient_color}; // Provided ambient color, default background color is black

  // First screen, facing south, hit expected at (2, 0, 0): light source not visible
  auto screen1 = std::make_unique<Plane>(translation(2.f * VEC_X) * rotation_y(-std::numbers::pi_v<float> / 2.f), grey_material);
  world.add_object(std::move(screen1));
  Color color1 = renderer(ray);

  EXPECT_TRUE(color1.is_close(ambient_color));

  // Mirror facing south-west in the x-y plane, hit expected at (1, 0, 0) and second screen facing east, hit expected at (1, 2, 0)
  // after reflection o the mirror: light source is visile
  auto mirror = std::make_unique<Plane>(translation(VEC_X) * rotation_z(-std::numbers::pi_v<float> / 4.f) *
                                            rotation_y(-std::numbers::pi_v<float> / 2.f),
                                        mirror_material);
  world.add_object(std::move(mirror));
  auto screen2 = std::make_unique<Plane>(translation(2.f * VEC_Y) * rotation_x(std::numbers::pi_v<float> / 2.f), grey_material);
  world.add_object(std::move(screen2));
  Color color2 = renderer(ray);

  Color screen_brdf_attenuation =
      (Color(0.5f, 0.5f, 0.5f) / std::numbers::pi_v<float>)*std::cosf(std::numbers::pi_v<float> / 4.f);
  Color mirror_brdf_attenuation = Color(0.5f, 0.5f, 0.5f);
  Color expected_color = (ambient_color + WHITE * screen_brdf_attenuation) * mirror_brdf_attenuation;

  EXPECT_TRUE(color2.is_close(expected_color));
}

// Test path tracing
// Furnace test: cast a ray inside a closed surface with diffusive BRDF and uniform reflectance rho_d and emitted
// radiance L_e; scatter exactly one ray at every hit (every ray should yield the same contribution regardless of its
// direction); stop after a large number of reflections (200); compare with the analytical solution: L = L_e/(1 - rho_d),
// which arises from a geometric sum; repeat 100 times for random valued of reflectance and emitted radiance
TEST(PathTracerTest, test_furnace) {

  // 1. Build material
  // Save raw pointer handles before moving the unique_ptrs so that pigment parameters can be mutated inside
  // the test loop. This is an intentional test-only hack: materials are conceptually immutable, but here we
  // want to sweep reflectance and emitted radiance without rebuilding the whole scene each iteration.
  auto pigment = std::make_unique<UniformPigment>();
  UniformPigment *pigment_raw = pigment.get();
  auto brdf = std::make_unique<DiffusiveBRDF>(std::move(pigment));
  auto emitted_radiance = std::make_unique<UniformPigment>();
  UniformPigment *emitted_radiance_raw = emitted_radiance.get();

  Material material{std::move(brdf), std::move(emitted_radiance)};

  // 2. Build enclosing sphere and world
  auto enclosure = std::make_unique<Sphere>(Transformation{}, material);
  World world{};
  world.add_object(std::move(enclosure));

  // 3. Cast ray and compute total radiance
  Ray ray{Point(), VEC_X};
  auto pcg = std::make_unique<PCG>();
  auto pcg_test = std::make_unique<PCG>();

  PathTracer renderer{world, std::move(pcg), 1, 200,
                      200}; // Russian roulette is not invoked up to depth=200, which is the max depth

  // 4. Compute total radiance and compare with expected result
  for (int i = 0; i < 100; i++) {
    float reflectance = pcg_test->random_float() * 0.9f; // avoid rho_d = 1 for numerical stability
    float luminosity = pcg_test->random_float();
    pigment_raw->color = Color(reflectance, 0.f, 0.f);
    emitted_radiance_raw->color = Color(luminosity, 0.f, 0.f);

    Color total_radiance = renderer(ray);
    Color expected{luminosity / (1.f - reflectance), 0.f, 0.f};
    EXPECT_TRUE(total_radiance.is_close(expected));
  }
}
