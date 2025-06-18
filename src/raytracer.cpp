#include "CLI11.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "renderers.hpp"
#include "scenefiles.hpp"
#include "shapes.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

//---------------------------------------------
//---------- FORWARD DECLARATIONS ----------
//---------------------------------------------

// Function wrapper to calculate total elapsed time
template<typename Func>
void run_with_timer(Func func);

// Callback for progress bar generation
void show_progress(float progress);

// Generate demo image with on off rendering
std::unique_ptr<HdrImage> make_demo_image_onoff(bool orthogonal, int width, int height, float distance,
                                                const Transformation &obs_transformation, int samples_per_pixel_edge);
// Generate demo image with Monte Carlo path tracing rendering
std::unique_ptr<HdrImage> make_demo_image_path(bool orthogonal, int width, int height, float distance,
                                               const Transformation &obs_transformation, int samples_per_pixel_edge);

//-------------------------------------------------------------------
//---------- MAIN FUNCTION ----------
//-------------------------------------------------------------------

int main(int argc, char **argv) {
  CLI::App app{"Raytracer"};    // Define the main CLI11 App
  argv = app.ensure_utf8(argv); // Ensure utf8 standard

  // The main App requires exactly one subcommand from the command line: the user has to choose between pfm2png and demo mode
  app.require_subcommand(1);

  // -----------------------------------------------------------
  // Parameters that are common to at least two of the modes
  // -----------------------------------------------------------

  // Default values of gamma and alpha for HDR to LDR image conversion (may be overwritten in pfm2png mode)
  float gamma = 2.2f;
  float alpha = 0.18f;

  // Name of output file
  std::string output_file_name;

  // Image size (# pixels)
  int width = 1280;
  int height = 960;

  // Antialisasing parameter
  int samples_per_pixel_edge = 1;

  // Flag for dark image tone mapping
  bool dark_mode = false;

  // -----------------------------------------------------------
  // Command line parsing for demo mode
  // -----------------------------------------------------------

  // Add a subcommand for the demo mode
  auto demo_subc =
      app.add_subcommand("demo", "Run demo rendering and save PFM and PNG files"); // Returns a pointer to an App object

  // Add option for on_off tracing or path tracing rendering
  // Default is on/off tracing
  std::string mode_str = "onoff";
  demo_subc->add_option("-m,--mode", mode_str, "Rendering mode: on/off tracing (default) or path tracing")
      ->check(CLI::IsMember({"onoff", "path"}))
      ->default_val("onoff");

  // Add options for image width and height (# pixels) and provide description in the help output
  // Default values are 1280x960
  demo_subc->add_option("--width", width, "Specify image width")
      ->check(CLI::PositiveNumber)
      ->default_val(1280); // Reject negative values
  demo_subc->add_option("--height", height, "Specify image height")
      ->check(CLI::PositiveNumber)
      ->default_val(960); // Reject negative values

  // Add option for perspective or orthogonal projection
  bool orthogonal = false;
  auto orthogonal_flag = demo_subc->add_flag("--orthogonal", orthogonal, "Use orthogonal projection (default is perspective)");

  // Add option for output file name
  // Default value is "demo"
  demo_subc->add_option("-o,--output-file", output_file_name, "Insert name of the output PNG file")->default_val("demo");

  // Add option for observer transformation: rotation around the scene and camera-screen distance (only for perspective camera).
  // Angles phi and theta: longitude and colatitude (theta=0 -> north pole). Default position along the negative x direction:
  // theta = 90, phi = 180.
  float distance = 1.f;
  float theta = std::numbers::pi_v<float> / 2.f;
  float phi = std::numbers::pi_v<float>;

  demo_subc->add_option("-d,--distance", distance, "Specify observer's distance from screen")
      ->excludes(orthogonal_flag)
      ->default_val(1.f);

  demo_subc
      ->add_option_function<float>(
          "--theta-deg", [&theta](const float &theta_deg) { theta = (theta_deg / 180.f) * std::numbers::pi_v<float>; },
          "Specify observer's colatitude angle theta (0 degree is north pole)")
      ->default_val(90.f);

  demo_subc
      ->add_option_function<float>(
          "--phi-deg", [&phi](const float &phi_deg) { phi = (phi_deg / 180.f) * std::numbers::pi_v<float>; },
          "Specify observer's longitude angle phi (default is 180 degrees, observer along negative direction of x-axis)")
      ->default_val(180.f);

  demo_subc
      ->add_option<int>("--antialiasing", samples_per_pixel_edge,
                        "Specify #samples per pixel edge (square root of #samples per pixel)")
      ->default_val(1);

  // -----------------------------------------------------------
  // Command line parsing for render mode
  // -----------------------------------------------------------

  auto render_subc =
      app.add_subcommand("render", "Render the scene encoded in an input file"); // Returns a pointer to an App object

  // Image width and height (# pixels)
  render_subc->add_option("--width", width, "Specify image width")->check(CLI::PositiveNumber)->default_val(1280);
  render_subc->add_option("--height", height, "Specify image height")->check(CLI::PositiveNumber)->default_val(960);

  // Input (source) file
  std::string source_file_name;
  render_subc->add_option("-s, --source", source_file_name, "Specify input (source) file.txt containing the scene to render");

  // Output file
  render_subc->add_option("-o,--output-file", output_file_name, "Insert name of the output PNG file")->default_val("demo");

  // Antialiasing
  render_subc
      ->add_option<int>("--antialiasing", samples_per_pixel_edge,
                        "Specify #samples per pixel edge (square root of #samples per pixel)")
      ->default_val(1);

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

  // Option to decide which rendering algorithm to use
  std::string render_mode_str = "flat"; // Default is flat tracing
  render_subc
      ->add_option("-m,--mode", render_mode_str,
                   "Rendering mode: on/off tracing, flat tracing (default), point light tracing or path tracing")
      ->check(CLI::IsMember({"onoff", "flat", "point_light", "path"}))
      ->default_val("flat");

  // Parameters for path tracing
  int n_rays = 10;
  int russian_roulette_lim = 3;
  int max_depth = 5;
  render_subc->add_option("--n_rays", n_rays, "Specify number of rays scattered at every hit (requires path tracing)")
      ->default_val(10);
  render_subc
      ->add_option("--roulette", russian_roulette_lim,
                   "Specify ray depth reached before russian roulette starts applying (requires path tracing)")
      ->default_val(3);
  render_subc->add_option("--max-depth", max_depth, "Specify maximum ray depth (requires path tracing)")->default_val(5);

  // Flag for dark (almost-black) image rendering: sets a fixed (default) value for parameter avg_luminosity of
  // HdrImage::normalize_image() used in tone mapping (i. e. exposure)
  render_subc
      ->add_flag("--dark", dark_mode,
                 "Set default exposure for dark images (works if rgb values of non-dark colors are of order 0.1-1)")
      ->default_val(false);

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

  // Flag for dark (almost-black) image rendering: sets a fixed (default) value for parameter avg_luminosity of
  // HdrImage::normalize_image() used in tone mapping (i. e. exposure)
  pfm2png_subc
      ->add_flag("--dark", dark_mode,
                 "Set default exposure for dark images (works if rgb values of non-dark colors are of order 0.1-1)")
      ->default_val(false);

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

    if (mode_str == "onoff") {
      screen_transformation =
          rotation_z(phi - std::numbers::pi_v<float>) * rotation_y(std::numbers::pi_v<float> / 2.f - theta) * translation(-VEC_X);
      // Default is translation(-VEC_X)
    } else if (mode_str == "path") {
      screen_transformation = rotation_z(phi - std::numbers::pi_v<float>) * rotation_y(std::numbers::pi_v<float> / 2.f - theta) *
                              translation(-3.f * VEC_X);
      // Default is translation(-3.f * VEC_X)
    }

    // Generate the demo image accordingly
    std::cout << "Rendering demo image... " << std::endl << std::flush;
    if (mode_str == "onoff") {
      img = make_demo_image_onoff(orthogonal, width, height, distance, screen_transformation, samples_per_pixel_edge);
    } else if (mode_str == "path") {
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
      auto pcg = std::make_shared<PCG>();
      renderer = make_shared<PathTracer>(scene.world, pcg, n_rays, russian_roulette_lim, max_depth, BLACK);
    }

    std::cout << "Rendering image in " << source_file_name << "... " << std::endl << std::flush;
    run_with_timer([&]() {
      tracer.fire_all_rays([&](const Ray &ray) { return (*renderer)(ray); }, show_progress);
    });

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

//-------------------------------------------------------------------
//-------------- HELPER FUNCTIONS --------------
//-------------------------------------------------------------------

//------------- Function wrapper to calculate total elapsed time -------------

template<typename Func>
void run_with_timer(Func func) {
  auto start = std::chrono::steady_clock::now();
  func();
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << std::endl;
  std::cout << "Elapsed time: " << elapsed_seconds.count() << " s" << std::endl;
}

//------------- Callback for progress bar generation -------------

void show_progress(float progress) {
  const int bar_width = 50;
  int pos = static_cast<int>(bar_width * progress);

  std::cout << "\r["; // Carriage return
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos) std::cout << "\033[1;32m█\033[0m";  // Green block
    else std::cout << " ";
  }
  std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100.0f) << " %";
  std::cout.flush();
}

//------------- OnOff Tracing Demo Image -------------

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

//--------------------- Path tracing demo image ---------------------

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
