#include "colors.hpp"
#include "geometry.hpp"
#include "shapes.hpp"
#include <fstream>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "CLI11.hpp"
#include "stb_image_write.h"

std::unique_ptr<HdrImage> make_demo_image(bool orthogonal);

int main(int argc, char **argv) {
  CLI::App app{"Raytracer"};    // Define the main App
  argv = app.ensure_utf8(argv); // Ensure utf8 standard

  // The main App requires one subcommand from the command line: the user has to choose between pfm2png and demo mode
  app.require_subcommand(1);

  // Default values of gamma and alpha (may be overwritten in pfm2png mode)
  float gamma = 2.2f;
  float alpha = 0.18f;

  // Name of output file
  std::string output_png_file_name;

  // Command line parsing for demo mode
  auto demo_subc =
      app.add_subcommand("demo", "Run demo rendering and save PFM and PNG files"); // Returns a pointer to an App object

  bool orthogonal = false;
  demo_subc->add_flag("--orthogonal", orthogonal, "Use orthogonal projection (default is perspective)");

  // Command line parsing for pfm2png converter mode
  auto pfm2png_subc = app.add_subcommand("pfm2png", "Convert a PFM file into a PNG file");
  std::string input_pfm_file_name;

  pfm2png_subc->add_option("-i,--input-file", input_pfm_file_name, "Insert name of the input PFM file")->required();
  pfm2png_subc->add_option("-o,--output-file", output_png_file_name, "Insert name of the output PNG file")->required();

  pfm2png_subc->add_option("-g,--gamma", gamma, "Insert gamma factor for tone mapping")
      ->check(CLI::PositiveNumber); // reject negative values
  pfm2png_subc->add_option("-a,--alpha", alpha, "Insert alpha factor for luminosity regularization")
      ->check(CLI::PositiveNumber); // reject negative values

  CLI11_PARSE(app, argc, argv);

  // 1. Fill in HdrImage
  std::unique_ptr<HdrImage> img;
  if (*demo_subc) {
    // 1. (DEMO) Compute the demo image
    std::cout << "Rendering demo image... " << std::flush;
    img = make_demo_image(orthogonal);
    std::cout << "Done." << std::endl;

    output_png_file_name = "demo.png";
    // TODO in demo mode also the pfm image should be saved

  } else if (*pfm2png_subc) {
    // 1. (CONVERTER) Read input image from file
    try {
      img = make_unique<HdrImage>(input_pfm_file_name);
      std::cout << "File \"" << input_pfm_file_name << "\" has been read from disk.\n";
    } catch (const std::exception &err) {
      std::cerr << "Error reading image. " << err.what() << '\n';
      return EXIT_FAILURE;
    }
  }
  // Note that these are the only possibilities, since the user is required to run a subcommand

  // 2. Process the image (normalize and clamp)
  img->normalize_image(alpha);
  img->clamp_image();

  // 4. Save the output image
  try {
    img->write_ldr_image(output_png_file_name, gamma, "png");
    std::cout << "File \"" << output_png_file_name << "\" has been written to disk.\n";
  } catch (const std::exception &err) {
    std::cerr << "Error writing image. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

// TODO add support for other possible observer transformations (perhaps the composition of a translation along the x-axis (first) and a rotation (two angles))
// TODO consider using CLI11 callback support (e. g. to define the observer transformation while parsing)
// TODO you might want to specify the size of the image from command line 

std::unique_ptr<HdrImage> make_demo_image(bool orthogonal) {
  // Initialize ImageTracer
  auto img = std::make_unique<HdrImage>(4000, 3000);
  std::unique_ptr<Camera> cam;
  if (orthogonal) {
    cam =
        std::make_unique<OrthogonalCamera>(4.f / 3.f, translation(-VEC_X)); // aspect ratio and observer transformation
  } else {
    cam = std::make_unique<PerspectiveCamera>(
        1.f, 4.f / 3.f, translation(-VEC_X)); // distance, aspect ratio and observer transformation
  }
  ImageTracer tracer(std::move(img), std::move(cam));

  // Initialize demo World
  World world = World();

  // TODO refine... the following doesn't look nice
  scaling sc = scaling({0.1f, 0.1f, 0.1f});
  auto sphere1 = std::make_shared<Sphere>(translation(Vec(0.5f, 0.5f, 0.5f)) * sc);
  auto sphere2 = std::make_shared<Sphere>(translation(Vec(0.5f, 0.5f, -0.5f)) * sc);
  auto sphere3 = std::make_shared<Sphere>(translation(Vec(0.5f, -0.5f, 0.5f)) * sc);
  auto sphere4 = std::make_shared<Sphere>(translation(Vec(0.5f, -0.5f, -0.5f)) * sc);
  auto sphere5 = std::make_shared<Sphere>(translation(Vec(-0.5f, 0.5f, 0.5f)) * sc);
  auto sphere6 = std::make_shared<Sphere>(translation(Vec(-0.5f, 0.5f, -0.5f)) * sc);
  auto sphere7 = std::make_shared<Sphere>(translation(Vec(-0.5f, -0.5f, 0.5f)) * sc);
  auto sphere8 = std::make_shared<Sphere>(translation(Vec(-0.5f, -0.5f, -0.5f)) * sc);
  auto sphere9 = std::make_shared<Sphere>(translation(Vec(0.f, 0.f, -0.5f)) * sc);
  auto sphere10 = std::make_shared<Sphere>(translation(Vec(0.f, 0.5f, 0.f)) * sc);

  world.add_object(sphere1);
  world.add_object(sphere2);
  world.add_object(sphere3);
  world.add_object(sphere4);
  world.add_object(sphere5);
  world.add_object(sphere6);
  world.add_object(sphere7);
  world.add_object(sphere8);
  world.add_object(sphere9);
  world.add_object(sphere10);

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
