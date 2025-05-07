#include "colors.hpp"
#include "geometry.hpp"
#include "shapes.hpp"
#include <fstream>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "CLI11.hpp"
#include "stb_image_write.h"

std::unique_ptr<HdrImage> make_demo_image(bool orthogonal, int width, int height,
                                          const Transformation &obs_transformation);

int main(int argc, char **argv) {
  CLI::App app{"Raytracer"};    // Define the main App
  argv = app.ensure_utf8(argv); // Ensure utf8 standard

  // The main App requires one subcommand from the command line: the user has to choose between pfm2png and demo mode
  app.require_subcommand(1);

  // -----------------------------------------------------------
  // Parameters that are common to both demo and pfm2png modes
  // -----------------------------------------------------------

  // Default values of gamma and alpha (may be overwritten in pfm2png mode)
  float gamma = 2.2f;
  float alpha = 0.18f;

  // Name of output file
  std::string output_file_name;

  // -----------------------------------------------------------
  // Command line parsing for demo mode
  // -----------------------------------------------------------

  auto demo_subc =
      app.add_subcommand("demo", "Run demo rendering and save PFM and PNG files"); // Returns a pointer to an App object

  // Parse image width and height (# pixels)
  int width = 1280;
  int height = 960;
  demo_subc->add_option("--width", width, "Specify image width")
      ->check(CLI::PositiveNumber)
      ->default_val(1280);
  demo_subc->add_option("--height", height, "Specify image height")
      ->check(CLI::PositiveNumber)
      ->default_val(960);
  ;

  // Choose between perspective and orthogonal projection
  bool orthogonal = false;
  auto orthogonal_flag =
      demo_subc->add_flag("--orthogonal", orthogonal, "Use orthogonal projection (default is perspective)");

  // Parse output file name
  demo_subc->add_option("-o,--output-file", output_file_name, "Insert name of the output PNG file")
      ->default_val("demo");

  // Parse observer transformation: composition of a translation along -VEC_X and rotation around the scene (endcoded in
  // distance, angle phi and angle theta). Default position: Origin - VEC_X.
  float distance = 1.f;
  float theta = std::numbers::pi / 2.f;
  float phi = 0.f;

  demo_subc->add_option("-d,--distance", distance, "Specify observer's distance")
      ->excludes(orthogonal_flag)
      ->default_val(1.f);

  demo_subc->add_option_function<float>(
      "--theta-deg", [&theta](const float &theta_deg) { theta = theta_deg / 180.f * std::numbers::pi; },
      "Specify observer's angle theta")
      ->default_val(90.f);

  demo_subc->add_option_function<float>(
      "--phi-deg", [&phi](const float &phi_deg) { phi = phi_deg / 180.f * std::numbers::pi; },
      "Specify observer's angle phi")
      ->default_val(0.f);;

  // -----------------------------------------------------------
  // Command line parsing for pfm2png converter mode
  // -----------------------------------------------------------

  auto pfm2png_subc = app.add_subcommand("pfm2png", "Convert a PFM file into a PNG file");
  std::string input_pfm_file_name;

  pfm2png_subc->add_option("-i,--input-file", input_pfm_file_name, "Insert name of the input PFM file")->required();
  pfm2png_subc->add_option("-o,--output-file", output_file_name, "Insert name of the output PNG file")->required();

  pfm2png_subc->add_option("-g,--gamma", gamma, "Insert gamma factor for tone mapping")
      ->check(CLI::PositiveNumber); // reject negative values
  pfm2png_subc->add_option("-a,--alpha", alpha, "Insert alpha factor for luminosity regularization")
      ->check(CLI::PositiveNumber); // reject negative values

  // -----------------------------------------------------------
  // Procedure
  // -----------------------------------------------------------

  // 1. Parse command line
  CLI11_PARSE(app, argc, argv);

  // 2. Fill in HdrImage
  std::unique_ptr<HdrImage> img;
  if (*demo_subc) {
    // 2. (DEMO) Compute the demo image and save PFM file
    Transformation observer_transformation =
        rotation_z(phi) * rotation_y(std::numbers::pi / 2.f - theta) * translation(-VEC_X * distance);

    std::cout << "Rendering demo image... " << std::flush;
    img = make_demo_image(orthogonal, width, height, observer_transformation);
    std::cout << "Done." << std::endl;

    // Save PFM image
    img->write_pfm(output_file_name + ".pfm");

  } else if (*pfm2png_subc) {
    // 2. (CONVERTER) Read input image from file
    try {
      img = make_unique<HdrImage>(input_pfm_file_name);
      std::cout << "File \"" << input_pfm_file_name << "\" has been read from disk.\n";
    } catch (const std::exception &err) {
      std::cerr << "Error reading image. " << err.what() << '\n';
      return EXIT_FAILURE;
    }
  }
  // Note that these are the only possibilities, since the user is required to run a subcommand

  // 3. Process the image (normalize and clamp)
  img->normalize_image(alpha);
  img->clamp_image();

  // 4. Save the output image
  try {
    img->write_ldr_image(output_file_name + ".png", gamma, "png");
    std::cout << "File \"" << output_file_name + ".png" << "\" has been written to disk.\n";
  } catch (const std::exception &err) {
    std::cerr << "Error writing image. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

std::unique_ptr<HdrImage> make_demo_image(bool orthogonal, int width, int height,
                                          const Transformation &obs_transformation) {
  // Initialize ImageTracer
  auto img = std::make_unique<HdrImage>(width, height);

  float aspect_ratio = (float)width / height;

  std::unique_ptr<Camera> cam;
  if (orthogonal) {
    // provide aspect ratio and observer transformation
    cam = std::make_unique<OrthogonalCamera>(aspect_ratio, obs_transformation);
  } else {
    // provide default *origin-screen* distance, aspect ratio and observer transformation
    cam = std::make_unique<PerspectiveCamera>(1.f, aspect_ratio, obs_transformation);
  }
  ImageTracer tracer(std::move(img), std::move(cam));

  // Initialize demo World
  World world = World();

  scaling sc({0.1f, 0.1f, 0.1f}); // common scaling for all spheres

  std::vector<Vec> sphere_positions = {
      {0.5f, 0.5f, 0.5f},   {0.5f, 0.5f, -0.5f},  {0.5f, -0.5f, 0.5f},   {0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f},
      {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -0.5f},  {0.0f, 0.5f, 0.0f}};

  for (const Vec &pos : sphere_positions) {
    auto sphere = std::make_shared<Sphere>(translation(pos) * sc);
    world.add_object(sphere);
  }

  // Perform on/off tracing
  tracer.fire_all_rays([&world](Ray ray) -> Color {
    return world.on_off_trace(ray);
  }); // World::on_off_trace requires three arguments, the first one being the World instance, hence it is not
      // compatible with type RaySolver. A lambda wrapping is therefore needed.

  // Return demo image
  return std::move(tracer.image);
}

// int main(int argc, char *argv[]) {
//   // Step 1: Parse command-line arguments
//   Parameters parameters;
//   try {
//     parameters.parse_command_line(argc, argv);
//   } catch (const std::runtime_error &err) {
//     std::cerr << "Error parsing command line. " << err.what() << '\n';
//     return EXIT_FAILURE;
//   }
//
//   // Step 2: Read HDR image from PFM file
//   HdrImage img(0, 0);
//   try {
//     img = HdrImage(parameters.input_pfm_file_name);
//     std::cout << "File \"" << parameters.input_pfm_file_name << "\" has been read from disk.\n";
//
//   } catch (const std::exception &err) {
//     std::cerr << "Error reading image. " << err.what() << '\n';
//     return EXIT_FAILURE;
//   }
//
//   // Step 3: Process the image (normalize + clamp)
//   img.normalize_image(parameters.a_factor);
//   img.clamp_image();
//
//   // Step 4: Write the output image (LDR, 8-bit PNG)
//   try {
//     img.write_ldr_image(parameters.output_ldr_file_name, parameters.gamma, "png");
//     std::cout << "File \"" << parameters.output_ldr_file_name << "\" has been written to disk.\n";
//   } catch (const std::exception &err) {
//     std::cerr << "Error writing image. " << err.what() << '\n';
//     return EXIT_FAILURE;
//   }
//
//   return EXIT_SUCCESS;
// }
