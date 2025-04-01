// NOTE Fyi, I have added the tags NOTE and QUESTION to the TodoTree extension
// QUESTION Can you see them? Or should you modify the settings of your
// extension too? When this works for you too, just delete these lines

#include "colors.hpp"
#include <iostream>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char* argv[]) {
  // Step 1: Parse command-line arguments
  Parameters parameters;
  try {
      parameters.parse_command_line(argc, argv);
  } catch (const std::runtime_error& err) {
      std::cerr << "Error: " << err.what() << '\n';
      return 1;
  }

  // Step 2: Read HDR image from PFM file
  // Step 3: Process the image (normalize + clamp)
  try {
      std::ifstream input(parameters.input_pfm_file_name, std::ios::binary);
      if (!input) {
          throw std::runtime_error("Failed to open input file: " + parameters.input_pfm_file_name);
      }
    //FIXME insert proper call to the HdrImage constructor
    //Step 2
      HdrImage img(input);
      std::cout << "File \"" << parameters.input_pfm_file_name << "\" has been read from disk.\n";
      
    // Step 3
    img.normalize_image(parameters.a_factor);
    img.clamp_image();
  } catch (const std::exception& err) {
      std::cerr << "Error reading image: " << err.what() << '\n';
      return 1;
  }

  //FIXME image out of scope here
  // Step 4: Write the output image (LDR, 8-bit PNG)
  try {
      img.write_ldr_image(parameters.output_png_file_name, parameters.gamma, "png");
      std::cout << "File \"" << parameters.output_png_file_name << "\" has been written to disk.\n";
  } catch (const std::exception& err) {
      std::cerr << "Error writing image: " << err.what() << '\n';
      return 1;
  }

  return 0;
}

