// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR DEMO SCENES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include "colors.hpp"
#include "geometry.hpp"
#include "profiling.hpp"
#include "renderers.hpp"
#include "scenefiles.hpp"
#include "shapes.hpp"

// ------------------------------------------------------------------------------------------------------------
// ON/OFF TRACING
// ------------------------------------------------------------------------------------------------------------

/// @brief Builds demo image for on/off tracing
std::unique_ptr<HdrImage> make_demo_image_onoff(bool orthogonal, int width, int height, float distance,
                                                const Transformation &screen_transformation, int samples_per_pixel_edge) {

  // Initialize ImageTracer
  auto img = std::make_unique<HdrImage>(width, height);

  float aspect_ratio = (float)width / height;

  std::shared_ptr<Camera> cam;
  if (orthogonal) {
    // provide aspect ratio and observer transformation
    cam = std::make_shared<OrthogonalCamera>(aspect_ratio, screen_transformation);
  } else {
    // provide default *origin-screen* distance, aspect ratio and observer transformation
    cam = std::make_unique<PerspectiveCamera>(distance, aspect_ratio, screen_transformation);
  }
  ImageTracer tracer(std::move(img), cam, samples_per_pixel_edge);

  // Initialize demo World
  auto world = std::make_shared<World>();

  scaling sc({0.1f, 0.1f, 0.1f}); // common scaling for all spheres

  std::vector<Vec> sphere_positions = {{0.5f, 0.5f, 0.5f},  {0.5f, 0.5f, -0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, -0.5f},
                                       {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f},
                                       {0.0f, 0.0f, -0.5f}, {0.0f, 0.5f, 0.0f}};

  for (const Vec &pos : sphere_positions) {
    auto sphere = std::make_shared<Sphere>(translation(pos) * sc);
    world->add_object(sphere);
  }

  // Perform on/off tracing
  tracer.fire_all_rays(OnOffTracer(world), show_progress);

  // Return demo image
  return std::move(tracer.image);
}

// ------------------------------------------------------------------------------------------------------------
// PATH TRACING
// ------------------------------------------------------------------------------------------------------------

/// @brief Builds demo image for path tracing
std::unique_ptr<HdrImage> make_demo_image_path(bool orthogonal, int width, int height, float distance,
                                               const Transformation &screen_transformation, int samples_per_pixel_edge) {
  // 1. Create World
  std::shared_ptr<World> world = std::make_shared<World>();

  // 2. Define Pigments and Materials
  auto sky_emission = std::make_shared<UniformPigment>(Color(0.2f, 0.3f, 1.f));
  auto black_pigment = std::make_shared<UniformPigment>(BLACK);
  auto sky_material = std::make_shared<Material>(std::make_shared<DiffusiveBRDF>(black_pigment), sky_emission);

  auto ground_pattern = std::make_shared<CheckeredPigment>(Color(0.3f, 0.5f, 0.1f), Color(0.1f, 0.2f, 0.5f), 4);
  auto ground_material = std::make_shared<Material>(std::make_shared<DiffusiveBRDF>(ground_pattern),
                                                    std::make_shared<UniformPigment>(Color(0.f, 0.f, 0.f)));

  auto grey_pigment = std::make_shared<UniformPigment>(Color(0.5f, 0.5f, 0.5f));
  auto sphere_material = std::make_shared<Material>(std::make_shared<SpecularBRDF>(grey_pigment), black_pigment);

  auto red_pigment = std::make_shared<UniformPigment>(Color(0.8f, 0.1f, 0.f));
  auto sphere2_material = std::make_shared<Material>(std::make_shared<DiffusiveBRDF>(red_pigment), black_pigment);

  // 3. Add objects
  Transformation sky_transform = scaling({50.f, 50.f, 50.f});
  world->add_object(std::make_shared<Sphere>(sky_transform, sky_material));
  world->add_object(std::make_shared<Plane>(translation(Vec(0.f, 0.f, -2.f)), ground_material));
  world->add_object(std::make_shared<Sphere>(scaling({0.4f, 0.4f, 0.4f}), sphere_material));
  world->add_object(std::make_shared<Sphere>(translation(Vec(0.f, -1.5f, -2.f)), sphere2_material));

  // 4. Setup camera
  std::unique_ptr<Camera> camera;

  if (orthogonal) {
    camera = std::make_unique<OrthogonalCamera>(static_cast<float>(width) / height, screen_transformation);
  } else {
    camera = std::make_unique<PerspectiveCamera>(distance, static_cast<float>(width) / height, screen_transformation);
  }

  // 5. Render image with path tracing
  auto pcg = std::make_shared<PCG>();
  PathTracer tracer(world, pcg, 10, 2, 6); // n_rays, roulette limit, max_depth
  // FlatTracer tracer(world);

  // 6. Trace the image
  auto image = std::make_unique<HdrImage>(width, height);
  ImageTracer image_tracer(std::move(image), std::move(camera));
  image_tracer.fire_all_rays(tracer, show_progress);

  return std::move(image_tracer.image);
}