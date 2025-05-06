#include "colors.hpp"
#include <fstream>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "CLI11.hpp"

HdrImage make_demo_image();

int main(int argc, char **argv) {
  CLI::App app{"Raytracer"};     // Define the main App
  argv = app.ensure_utf8(argv);  // Ensure utf8 standard
  app.require_subcommand(1);     // The main App requires one subcommand from the command line

  // Command line parsing for demo behaviour
  auto demo_subc = app.add_subcommand("demo", "Run demo rendering and save PFM and PNG files"); // Returns a pointer to an App object

  // NOTE is it necessary also in case of demo??
  // Command line parsing for pfm2png converter behaviuor
  auto pfm2png_subc = app.add_subcommand("pfm2png", "Convert a PFM file into a PNG file");
  std::string input_pfm_file_name, output_png_file_name;

  pfm2png_subc->add_option("-i,--input-file", input_pfm_file_name, "Insert name of the input PFM file")->required();
  pfm2png_subc->add_option("-o,--output-file", output_png_file_name, "Insert name of the output PNG file")->required();

  float gamma = 2.2f;
  float alpha = 0.18f;
  pfm2png_subc->add_option("-g,--gamma", gamma, "Insert gamma factor for tone mapping")->check(CLI::PositiveNumber); // reject negative values
  pfm2png_subc->add_option("-a,--alpha", alpha, "Insert alpha factor for luminosity regularization")->check(CLI::PositiveNumber); // reject negative values

  CLI11_PARSE(app, argc, argv);

  // 1. Fill an HdrImage
  HdrImage img(0, 0);
  if(*demo_subc) {
    // 1. (DEMO) Compute the demo image
    img = make_demo_image();

  } else if(*pfm2png_subc) {
    // 1. (CONVERTER) Read input image from file
     try {
       img = HdrImage(input_pfm_file_name);
       std::cout << "File \"" << input_pfm_file_name << "\" has been read from disk.\n";
     } catch (const std::exception &err) {
       std::cerr << "Error reading image. " << err.what() << '\n';
       return EXIT_FAILURE;
     }
  }
  // Note that these are the only possibilities, since we are requiring a subcommand to be run

  // 2. Process the image (normalize and clamp)
  img.normalize_image(alpha);
  img.clamp_image();

  // 4. Save the output image
  try {
    img.write_ldr_image(output_png_file_name, gamma, "png");
    std::cout << "File \"" << output_png_file_name << "\" has been written to disk.\n";
  } catch (const std::exception &err) {
    std::cerr << "Error writing image. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


HdrImage make_demo_image() {
  return
}




//int main(int argc, char *argv[]) {
//  // Step 1: Parse command-line arguments
//  Parameters parameters;
//  try {
//    parameters.parse_command_line(argc, argv);
//  } catch (const std::runtime_error &err) {
//    std::cerr << "Error parsing command line. " << err.what() << '\n';
//    return EXIT_FAILURE;
//  }
//
//  // Step 2: Read HDR image from PFM file
//  HdrImage img(0, 0);
//  try {
//    img = HdrImage(parameters.input_pfm_file_name);
//    std::cout << "File \"" << parameters.input_pfm_file_name << "\" has been read from disk.\n";
//
//  } catch (const std::exception &err) {
//    std::cerr << "Error reading image. " << err.what() << '\n';
//    return EXIT_FAILURE;
//  }
//
//  // Step 3: Process the image (normalize + clamp)
//  img.normalize_image(parameters.a_factor);
//  img.clamp_image();
//
//  // Step 4: Write the output image (LDR, 8-bit PNG)
//  try {
//    img.write_ldr_image(parameters.output_ldr_file_name, parameters.gamma, "png");
//    std::cout << "File \"" << parameters.output_ldr_file_name << "\" has been written to disk.\n";
//  } catch (const std::exception &err) {
//    std::cerr << "Error writing image. " << err.what() << '\n';
//    return EXIT_FAILURE;
//  }
//
//  return EXIT_SUCCESS;
//}
