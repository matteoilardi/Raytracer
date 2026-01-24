#include "CLI11.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "profiling.hpp"
#include "renderers.hpp"
#include "scenefiles.hpp"
#include "shapes.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

// -----------------------------------------------------------
// Helpers for command line parsing
// -----------------------------------------------------------

/// @brief Input for pfm2png mode
struct ConvInput { std::string file; };

/// @brief Input for render mode
struct RenderInput {
  std::string file;
  std::unordered_map<std::string, float> floats_from_cl;
};

/// @brief Output HDR image configuration
struct PngCfg {
  float alpha;
  float gamma;
  bool dark_mode;
  std::string output_file;
};

/// @brief Image size (pixels)
/// @details Used for render mode
struct ImgSize {
  int width;
  int height;
};

struct PathCfg {
  int n_rays;
  int russian_roulette_lim;
  int max_depth;
  int seq_number;
};

struct ModeCfg {
  enum class Mode { PATH, FLAT, POINTLIGHT, ONOFF } mode;
  std::variant<std::monostate, PathCfg> params;
  int samples_per_pixel_edge; // Antialiasing
};

/// @brief Add option for single input file
ConvInput& add_pfm2png_input(CLI::App *subc);

/// @brief Add option for input file and float definitions
RenderInput& add_render_input(CLI::App *subc);

/// @brief Add options for HDR output parameters
PngCfg& add_hdr_options(CLI::App *subc);

/// @brief Add option for image size (number of pixels)
ImgSize& add_img_size_options(CLI::App *subc);

/// @brief Add options for rendering algorithm parameters
ModeCfg& add_mode_options(CLI::App *subc);


// -----------------------------------------------------------
// Other helpers
// -----------------------------------------------------------

/// @brief Define renderer given its settings and an instance of World
std::unique_ptr<Renderer> build_renderer(const ModeCfg& mode_cfg, const World& world);

// -----------------------------------------------------------
// Main
// -----------------------------------------------------------

int main(int argc, char **argv) {

  // -----------------Command line parsing--------------------
  CLI::App app{"Raytracer"};    // Define the main CLI11 App
  argv = app.ensure_utf8(argv); // Ensure utf8 standard

  // The main App requires exactly one subcommand from the command line
  // The user has to choose between pfm2png and render mode
  app.require_subcommand(1);

  // Command line parsing for render mode
  auto render_subc = app.add_subcommand("render", "Render the scene reading description from an input file");

  auto& render_input = add_render_input(render_subc);
  auto& render_png_cfg = add_hdr_options(render_subc);
  auto& render_img_size = add_img_size_options(render_subc);
  auto& render_mode_cfg = add_mode_options(render_subc);

  // Command line parsing for pfm2png converter mode
  auto pfm2png_subc = app.add_subcommand("pfm2png", "Convert a PFM file into a PNG file");

  auto& pfm2png_input = add_pfm2png_input(pfm2png_subc);
  auto& pfm2png_png_cfg = add_hdr_options(pfm2png_subc);

  // Parse command line
  CLI11_PARSE(app, argc, argv);

  // -----------------Program pipeline--------------------
  PngCfg png_cfg;
  std::unique_ptr<HdrImage> img;

  // RENDERER
  if (*render_subc) {

    // Initialize input stream
    if (!std::filesystem::is_regular_file(render_input.file)) {
      std::cerr << "Path \"" << render_input.file << "\" does not exist or is not a regular file" << std::endl;
      return EXIT_FAILURE;
    }
    std::ifstream is;
    is.open(render_input.file);
    if (!is) {
      std::cerr << "Error opening input file \"" << render_input.file << "\"" << std::endl;
      return EXIT_FAILURE;
    }
    InputStream input_stream{is, render_input.file};

    // Parse scene
    Scene scene;
    if (!render_input.floats_from_cl.empty()) {
      scene.initialize_float_variables_with_priority(std::move(render_input.floats_from_cl));
    }
    try {
      scene.parse_scene(input_stream);
    } catch (const std::exception &err) {
      std::cerr << err.what() << '\n';
      return EXIT_FAILURE;
    }

    // Define the image tracer
    ImageTracer tracer = ImageTracer{
      std::make_unique<HdrImage>(render_img_size.width, render_img_size.height),
      scene.camera,
      render_mode_cfg.samples_per_pixel_edge
    };

    // Define the renderer
    std::unique_ptr<Renderer> renderer = build_renderer(render_mode_cfg, scene.world);

    // Render the image
    std::cout << "Rendering image in " << render_input.file << std::endl << std::flush;
    run_with_timer([&renderer, &tracer]() {
      tracer.fire_all_rays([&renderer](const Ray &ray) { return (*renderer)(ray); }, show_progress);
    });

    // Save PFM image
    tracer.image->write_pfm(png_cfg.output_file + ".pfm");
    img = std::move(tracer.image);

    png_cfg = render_png_cfg;

  // CONVERTER
  } else if (*pfm2png_subc) {
    try {
      img = make_unique<HdrImage>(pfm2png_input.file);
      std::cout << "File \"" << pfm2png_input.file << "\" has been read from disk.\n";
    } catch (const std::exception &err) {
      std::cerr << "Error reading image. " << err.what() << '\n';
      return EXIT_FAILURE;
    }

    png_cfg = pfm2png_png_cfg;
  }
  // Note that these are the only possibilities, since the user is required to run a subcommand

  // RENDERER AND CONVERTER
  // Process the image (normalize and clamp)
  if (!png_cfg.dark_mode) {
    img->normalize_image(png_cfg.alpha);
  } else {
    img->normalize_image(png_cfg.alpha, DEFAULT_AVG_LUMINOSITY_DARK_MODE);
  }
  img->clamp_image();

  // Save the output image
  try {
    img->write_ldr_image(png_cfg.output_file + ".png", png_cfg.gamma);
    std::cout << "File \"" << png_cfg.output_file + ".png" << "\" has been written to disk.\n";
  } catch (const std::exception &err) {
    std::cerr << "Error writing image. " << err.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


// -----------------------------------------------------------
// Helpers for command line parsing
// -----------------------------------------------------------

/// @brief Add option for single input file
ConvInput& add_pfm2png_input(CLI::App *subc) {
  static ConvInput conv_input;
  subc->add_option("input", conv_input.file, "Name of the input PFM file")->required();
  return conv_input;
}

/// @brief Add option for input file and float definitions
RenderInput& add_render_input(CLI::App *subc) {
  static RenderInput render_input;

  // Input (source) file
  subc->add_option("source", render_input.file, "Input file containing the scene to render")
      ->required();

  // Float variable definition from command line
  subc->add_option_function<std::vector<std::string>>(
      "--define-float",
      [](const std::vector<std::string> definition_strings) {
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
          render_input.floats_from_cl[name] = value;
        }
      },
      "Float variables defined as name=value");

  return render_input;
}

/// @brief Add options for HDR output parameters
PngCfg& add_hdr_options(CLI::App *subc) {
  static PngCfg png_cfg;

  subc->add_option("-g,--gamma", png_cfg.gamma, "Factor gamma for tone mapping")
      ->check(CLI::PositiveNumber)
      ->default_val("2.2"); // Reject negative values
  subc->add_option("-a,--alpha", png_cfg.alpha, "Factor alpha for luminosity regularization")
      ->check(CLI::PositiveNumber)
      ->default_val("0.18"); // Reject negative values

  // Flag for dark (almost-black) image rendering: sets a fixed (default) value for parameter avg_luminosity of
  // HdrImage::normalize_image() used in tone mapping (i. e. exposure)
  subc ->add_flag("--dark", png_cfg.dark_mode,
                 "Set default exposure for dark images (works if rgb values of non-dark colors are of order 0.1-1)")
      ->default_val("false");

  subc->add_option("-o,--output-file", png_cfg.output_file,
                 "Name of the output file name stem (extension is PNG)")
      ->default_val("out");

  return png_cfg;
}


/// @brief Add option for image size (number of pixels)
ImgSize& add_img_size_options(CLI::App *subc) {
  static ImgSize img_size;

  // Add options for image width and height (# pixels) and provide description in the help output
  // Default values are 1280x960
  subc->add_option("--width", img_size.width, "Image width (number of pixels)")
      ->check(CLI::PositiveNumber)
      ->default_val("1280"); // Reject negative values
  subc->add_option("--height", img_size.height, "Image height (number of pixels)")
      ->check(CLI::PositiveNumber)
      ->default_val("960"); // Reject negative values

  return img_size;
}

/// @brief Add options for rendering algorithm parameters
ModeCfg& add_mode_options(CLI::App *subc) {
  static ModeCfg mode_cfg;

  // Mode (rendering algorithm)
  static std::string mode_str;
  subc->add_option("-m,--mode", mode_str, "Rendering mode: onoff, flat, path, pointlight")
      ->check(CLI::IsMember({"onoff", "flat", "path", "pointlight"}))
      ->default_val("flat");

  // Path tracing specific parameters
  static PathCfg path_cfg;
  subc->add_option("--n_rays", path_cfg.n_rays,
                     "Number of rays scattered at every hit (requires path tracing)")
      ->default_val("10");
  subc->add_option("--roulette", path_cfg.russian_roulette_lim,
                     "Ray depth reached before russian roulette starts applying (requires path tracing)")
      ->default_val("3");
  subc->add_option("--max-depth", path_cfg.max_depth, "Maximum ray depth (requires path tracing)")
      ->default_val("5");
  subc->add_option("--seq-number", path_cfg.seq_number,
      "Sequence number for PCG random number generator (requires path tracing)")
      ->default_val("54");

  // Antialiasing
  subc->add_option<int>("--antialiasing", mode_cfg.samples_per_pixel_edge,
                        "Number of samples per pixel edge (square root of #samples per pixel)")
      ->default_val("1");

  // Initialize ModeCfg based on the selected value of mode
  subc->callback([&]() {
    if (mode_str == "onoff") mode_cfg.mode = ModeCfg::Mode::ONOFF;
    else if (mode_str == "flat") mode_cfg.mode = ModeCfg::Mode::FLAT;
    else if (mode_str == "path") mode_cfg.mode = ModeCfg::Mode::PATH;
    else if (mode_str == "pointlight") mode_cfg.mode = ModeCfg::Mode::POINTLIGHT;
    else std::unreachable();

    if (mode_str == "path") mode_cfg.params = path_cfg;
    else mode_cfg.params = std::monostate();
  });


  return mode_cfg;
}


// -----------------------------------------------------------
// Other helpers
// -----------------------------------------------------------

/// @brief Define renderer given its settings and an instance of World
std::unique_ptr<Renderer> build_renderer(const ModeCfg& mode_cfg, const World& world) {

  switch (mode_cfg.mode) {
    case ModeCfg::Mode::ONOFF: return std::make_unique<OnOffTracer>(world);
    case ModeCfg::Mode::FLAT: return std::make_unique<FlatTracer>(world, BLACK);
    case ModeCfg::Mode::PATH: {
      auto pcg_init_seq = static_cast<uint64_t>(std::get<PathCfg>(mode_cfg.params).seq_number);
      auto pcg = std::make_unique<PCG>(42ull, pcg_init_seq);
      
      return std::make_unique<PathTracer>(
        world,
        std::move(pcg),
        std::get<PathCfg>(mode_cfg.params).n_rays,
        std::get<PathCfg>(mode_cfg.params).russian_roulette_lim,
        std::get<PathCfg>(mode_cfg.params).max_depth,
        BLACK
      );
    }
    case ModeCfg::Mode::POINTLIGHT: {
      return std::make_unique<PointLightTracer>(world, DARK_GREY, BLACK);
    }
    std::unreachable();
  }
}
