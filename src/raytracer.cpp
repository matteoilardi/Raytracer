#include "CLI11.hpp"
#include "colors.hpp"
#include "demo.hpp"
#include "geometry.hpp"
#include "profiling.hpp"
#include "renderers.hpp"
#include "scenefiles.hpp"
#include "shapes.hpp"
#include <filesystem> // Constains functions to extract file name stem from a path
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
  CLI::App app{"Raytracer"};    // Define the main CLI11 App
  argv = app.ensure_utf8(argv); // Ensure utf8 standard

  // The main App requires exactly one subcommand from the command line: the user has to choose between pfm2png and demo mode
  app.require_subcommand(1);

  // -----------------------------------------------------------
  // Parameters that are common to at least two of the modes
  // -----------------------------------------------------------

  // Default values of gamma and alpha for HDR to LDR image conversion
  float gamma;
  float alpha;

  // Name of output file
  std::string output_file_name;

  // Image size (# pixels)
  int width;
  int height;

  // Antialisasing parameter
  int samples_per_pixel_edge;

  // Flag for dark image tone mapping
  bool dark_mode;

  // -----------------------------------------------------------
  // Command line parsing for "demo" mode
  // -----------------------------------------------------------

  // Add a subcommand for the demo mode
  auto demo_subc =
      app.add_subcommand("demo", "Run demo rendering and save PFM and PNG files"); // Returns a pointer to an App object

  // Add option for on/off tracing or path tracing rendering
  // Default is on/off tracing
  std::string demo_mode_str;
  demo_subc->add_option("-m,--mode", demo_mode_str, "Rendering mode: on/off tracing (default) or path tracing")
      ->check(CLI::IsMember({"onoff", "path"}))
      ->default_val("onoff");

  // Add options for image width and height (# pixels) and provide description in the help output
  // Default values are 1280x960
  demo_subc->add_option("--width", width, "Specify image width")
      ->check(CLI::PositiveNumber)
      ->default_val("1280"); // Reject negative values
  demo_subc->add_option("--height", height, "Specify image height")
      ->check(CLI::PositiveNumber)
      ->default_val("960"); // Reject negative values

  // Add option for perspective or orthogonal projection
  bool orthogonal = false;
  auto orthogonal_flag = demo_subc->add_flag("--orthogonal", orthogonal, "Use orthogonal projection (default is perspective)");

  // Add option for output file name
  // Default value is "demo"
  std::string output_file_name_demo;
  demo_subc->add_option("-o,--output-file", output_file_name_demo, "Insert name of the output PNG file")->default_val("demo");
  demo_subc->callback([&]() { output_file_name = output_file_name_demo; });

  // Add option for observer transformation: rotation around the scene and camera-screen distance (only for perspective camera).
  // Angles phi and theta: longitude and colatitude (theta=0 -> north pole). Default position along the negative x direction:
  // theta = 90, phi = 180.
  float distance = 1.f;
  float theta = std::numbers::pi_v<float> / 2.f;
  float phi = std::numbers::pi_v<float>;

  demo_subc->add_option("-d,--distance", distance, "Specify observer's distance from screen (default is 1)")
      ->excludes(orthogonal_flag);

  demo_subc->add_option_function<float>(
      "--theta-deg", [&theta](const float &theta_deg) { theta = (theta_deg / 180.f) * std::numbers::pi_v<float>; },
      "Specify observer's colatitude angle theta (0 degree is north pole, default is 90)");

  demo_subc->add_option_function<float>(
      "--phi-deg", [&phi](const float &phi_deg) { phi = (phi_deg / 180.f) * std::numbers::pi_v<float>; },
      "Specify observer's longitude angle phi (default is 180 degrees, observer along negative direction of x-axis)");

  demo_subc
      ->add_option<int>("--antialiasing", samples_per_pixel_edge,
                        "Specify #samples per pixel edge (square root of #samples per pixel)")
      ->default_val("1");

  demo_subc->add_option("-g,--gamma", gamma, "Insert gamma factor for tone mapping")
      ->check(CLI::PositiveNumber)
      ->default_val("2.2"); // Reject negative values
  demo_subc->add_option("-a,--alpha", alpha, "Insert alpha factor for luminosity regularization")
      ->check(CLI::PositiveNumber)
      ->default_val("0.18"); // Reject negative values

  // -----------------------------------------------------------
  // Command line parsing for "render" mode
  // -----------------------------------------------------------

  auto render_subc =
      app.add_subcommand("render", "Render the scene encoded in an input file"); // Returns a pointer to an App object

  // Option to decide which rendering algorithm to use
  std::string render_mode_str;
  render_subc
      ->add_option("-m,--mode", render_mode_str,
                   "Rendering mode: on/off tracing, flat tracing, point light tracing or path tracing")
      ->check(CLI::IsMember({"onoff", "flat", "point_light", "path"}))
      ->default_val("flat");

  // Image width and height (# pixels)
  render_subc->add_option("--width", width, "Specify image width")->check(CLI::PositiveNumber)->default_val("1280");
  render_subc->add_option("--height", height, "Specify image height")->check(CLI::PositiveNumber)->default_val("960");

  // Input (source) file
  std::string source_file_name;
  render_subc->add_option("source", source_file_name, "Specify input (source) file.txt containing the scene to render")
      ->required();

  // Output file
  std::string output_file_name_render;
  render_subc->add_option("-o,--output-file", output_file_name_render,
                          "Insert name of the output file name stem (default: <source>_<mode>)");
  render_subc->callback([&]() {
    if (output_file_name_render.empty()) { // Extract and assign source file name stem and render mode
      std::filesystem::path source_path(source_file_name);
      output_file_name_render = source_path.stem().string() + "_" + render_mode_str;
    }
    output_file_name = output_file_name_render;
  });

  // Antialiasing
  render_subc
      ->add_option<int>("--antialiasing", samples_per_pixel_edge,
                        "Specify #samples per pixel edge (square root of #samples per pixel)")
      ->default_val("1");

  // Float variable definition from command line
  std::unordered_map<std::string, float> floats_from_cl;
  render_subc->add_option_function<std::vector<std::string>>(
      "--define-float",
      [&floats_from_cl](const std::vector<std::string> definition_strings) { // lambda function parsing float definitions from CL
        for (const auto &def : definition_strings) {
          auto pos = def.find('=');
          if (pos == std::string::npos) {
            throw CLI::ValidationError("Invalid --define format: use name=value");
          }
          std::string name = def.substr(0, pos);          // First pos characters of the string def
          std::string value_string = def.substr(pos + 1); // Characters of string def from pos+1 to the end
          float value;
          try {
            value = std::stof(value_string);
          } catch (...) {
            throw CLI::ValidationError("Invalid float value");
          }
          floats_from_cl[name] = value;
        }
      },
      "Define named float variables as name=value");

  // Parameters for path tracing
  int n_rays;
  int russian_roulette_lim;
  int max_depth;
  render_subc->add_option("--n_rays", n_rays, "Specify number of rays scattered at every hit (requires path tracing)")
      ->default_val("10");
  render_subc
      ->add_option("--roulette", russian_roulette_lim,
                   "Specify ray depth reached before russian roulette starts applying (requires path tracing)")
      ->default_val("3");
  render_subc->add_option("--max-depth", max_depth, "Specify maximum ray depth (requires path tracing)")->default_val("5");

  // Flag for dark (almost-black) image rendering: sets a fixed (default) value for parameter avg_luminosity of
  // HdrImage::normalize_image() used in tone mapping (i. e. exposure)
  render_subc
      ->add_flag("--dark", dark_mode,
                 "Set default exposure for dark images (works if rgb values of non-dark colors are of order 0.1-1)")
      ->default_val("false");

  render_subc->add_option("-g,--gamma", gamma, "Insert gamma factor for tone mapping")
      ->check(CLI::PositiveNumber)
      ->default_val("2.2"); // Reject negative values
  render_subc->add_option("-a,--alpha", alpha, "Insert alpha factor for luminosity regularization")
      ->check(CLI::PositiveNumber)
      ->default_val("0.18"); // Reject negative values

  // -----------------------------------------------------------
  // Command line parsing for pfm2png converter mode
  // -----------------------------------------------------------

  auto pfm2png_subc = app.add_subcommand("pfm2png", "Convert a PFM file into a PNG file");
  std::string input_pfm_file_name;

  pfm2png_subc->add_option("-i,--input-file", input_pfm_file_name, "Insert name of the input PFM file")->required();
  pfm2png_subc->add_option("-o,--output-file", output_file_name, "Insert name of the output PNG file")->required();

  pfm2png_subc->add_option("-g,--gamma", gamma, "Insert gamma factor for tone mapping")
      ->check(CLI::PositiveNumber)
      ->default_val("2.2"); // Reject negative values
  pfm2png_subc->add_option("-a,--alpha", alpha, "Insert alpha factor for luminosity regularization")
      ->check(CLI::PositiveNumber)
      ->default_val("0.18"); // Reject negative values

  // Flag for dark (almost-black) image rendering: sets a fixed (default) value for parameter avg_luminosity of
  // HdrImage::normalize_image() used in tone mapping (i. e. exposure)
  pfm2png_subc
      ->add_flag("--dark", dark_mode,
                 "Set default exposure for dark images (works if rgb values of non-dark colors are of order 0.1-1)")
      ->default_val("false");

  // -----------------------------------------------------------
  // Procedure
  // -----------------------------------------------------------

  // 1. Parse command line
  CLI11_PARSE(app, argc, argv);

  // 2. Fill in HdrImage
  std::unique_ptr<HdrImage> img;

  if (*demo_subc) {
    // A. (DEMO) Compute the demo image and save PFM file
    Transformation screen_transformation;
    // Default position of the screen is the origin, while the parameter distance (that actually matters only for a perspective
    // camera) offsets the position of the observer along the negative direction of the x-axis.

    if (demo_mode_str == "onoff") {
      screen_transformation =
          rotation_z(phi - std::numbers::pi_v<float>) * rotation_y(std::numbers::pi_v<float> / 2.f - theta) * translation(-VEC_X);
      // Default is translation(-VEC_X)
    } else if (demo_mode_str == "path") {
      screen_transformation = rotation_z(phi - std::numbers::pi_v<float>) * rotation_y(std::numbers::pi_v<float> / 2.f - theta) *
                              translation(-3.f * VEC_X);
      // Default is translation(-3.f * VEC_X)
    }

    // Generate the demo image accordingly
    std::cout << "Rendering demo image... " << std::endl << std::flush;
    if (demo_mode_str == "onoff") {
      img = make_demo_image_onoff(orthogonal, width, height, distance, screen_transformation, samples_per_pixel_edge);
    } else if (demo_mode_str == "path") {
      img = make_demo_image_path(orthogonal, width, height, distance, screen_transformation, samples_per_pixel_edge);
    }
    std::cout << std::endl;

    // Save PFM image
    img->write_pfm(output_file_name + ".pfm");

  } else if (*render_subc) {
    // B. (RENDERER) Parse input file
    std::ifstream is;
    is.open(source_file_name);
    if (!is) {
      std::cerr << "Error opening input (source) file \"" << source_file_name << "\"" << std::endl;
      return EXIT_FAILURE;
    }
    InputStream input_stream(is, source_file_name);

    Scene scene;
    if (!floats_from_cl.empty()) {
      scene.initialize_float_variables_with_priority(std::move(floats_from_cl));
    }
    try {
      scene.parse_scene(input_stream);
    } catch (const std::exception &err) {
      std::cerr << err.what() << '\n';
      return EXIT_FAILURE;
    }

    ImageTracer tracer = ImageTracer(std::make_unique<HdrImage>(width, height), scene.camera, samples_per_pixel_edge);

    std::shared_ptr<Renderer> renderer;
    if (render_mode_str == "onoff") {
      renderer = make_shared<OnOffTracer>(scene.world);
    } else if (render_mode_str == "flat") {
      renderer = make_shared<FlatTracer>(scene.world, BLACK);
    } else if (render_mode_str == "point_light") {
      renderer = make_shared<PointLightTracer>(scene.world, Color(0.1f, 0.1f, 0.05f), BLACK);
    } else if (render_mode_str == "path") {
      auto pcg = std::make_unique<PCG>();
      renderer = make_shared<PathTracer>(scene.world, std::move(pcg), n_rays, russian_roulette_lim, max_depth, BLACK);
    }

    std::cout << "Rendering image in " << source_file_name << "... " << std::endl << std::flush;
    run_with_timer([&]() { tracer.fire_all_rays([&](const Ray &ray) { return (*renderer)(ray); }, show_progress); });

    // Save PFM image
    tracer.image->write_pfm(output_file_name + ".pfm");
    img = std::move(tracer.image);

  } else if (*pfm2png_subc) {
    // C. (CONVERTER) Read input image from file
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
  if (!dark_mode) {
    img->normalize_image(alpha);
  } else {
    img->normalize_image(alpha, DEFAULT_AVG_LUMINOSITY_DARK_MODE);
  }
  img->clamp_image();

  // 4. Save the output image
  try {
    img->write_ldr_image(output_file_name + ".png", gamma);
    std::cout << "File \"" << output_file_name + ".png" << "\" has been written to disk.\n";
  } catch (const std::exception &err) {
    std::cerr << "Error writing image. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
