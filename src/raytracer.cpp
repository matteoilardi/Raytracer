#include "colors.hpp"
#include <fstream>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char *argv[]) {
  // Step 1: Parse command-line arguments
  Parameters parameters;
  try {
    parameters.parse_command_line(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cerr << "Error parsing command line. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  // Step 2: Read HDR image from PFM file
  HdrImage img(0,0);
  try {
    img = HdrImage(parameters.input_pfm_file_name);
    std::cout << "File \"" << parameters.input_pfm_file_name
              << "\" has been read from disk.\n";

  } catch (const std::exception &err) {
    std::cerr << "Error reading image. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  // Step 3: Process the image (normalize + clamp)
  img.normalize_image(parameters.a_factor);
  img.clamp_image();

  //  Step 4: Write the output image (LDR, 8-bit PNG)
  try {
    img.write_ldr_image(parameters.output_ldr_file_name, parameters.gamma,
                        "png");
    std::cout << "File \"" << parameters.output_ldr_file_name
              << "\" has been written to disk.\n";
  } catch (const std::exception &err) {
    std::cerr << "Error writing image. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
