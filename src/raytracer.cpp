#ifdef type
#undef type
#endif

#include "CLI11.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "renderers.hpp"
#include "scenefiles.hpp"
#include "shapes.hpp"
#include <fstream>
#include <iostream>

std::unique_ptr<HdrImage> make_demo_image(bool orthogonal, int width, int height, const Transformation &obs_transformation,
                                          int samples_per_pixel_edge);

int main(int argc, char **argv) {
  CLI::App app{"Raytracer"};    // Define the main App
  argv = app.ensure_utf8(argv); // Ensure utf8 standard

  // The main App requires one subcommand from the command line: the user has to choose between pfm2png and demo mode
  app.require_subcommand(1);

  // -----------------------------------------------------------
  // Parameters that are common to at least two of the modes
  // -----------------------------------------------------------

  // Default values of gamma and alpha (may be overwritten in pfm2png mode)
  float gamma = 2.2f;
  float alpha = 0.18f;

  // Name of output file
  std::string output_file_name;

  // Image size (# pixels)
  int width = 1280;
  int height = 960;

  // Antialisasing parameter
  int samples_per_pixel_edge = 3;

  // -----------------------------------------------------------
  // Command line parsing for demo mode
  // -----------------------------------------------------------

  auto demo_subc =
      app.add_subcommand("demo", "Run demo rendering and save PFM and PNG files"); // Returns a pointer to an App object

  // Parse image width and height (# pixels)
  demo_subc->add_option("--width", width, "Specify image width")->check(CLI::PositiveNumber)->default_val(1280);
  demo_subc->add_option("--height", height, "Specify image height")->check(CLI::PositiveNumber)->default_val(960);

  // Choose between perspective and orthogonal projection
  bool orthogonal = false;
  auto orthogonal_flag = demo_subc->add_flag("--orthogonal", orthogonal, "Use orthogonal projection (default is perspective)");

  // Parse output file name
  demo_subc->add_option("-o,--output-file", output_file_name, "Insert name of the output PNG file")->default_val("demo");

  // Parse observer transformation: composition of a translation along -VEC_X and rotation around the scene (endcoded in
  // distance, angle phi and angle theta). Default position: Origin - VEC_X.
  float distance = 1.f;
  float theta = std::numbers::pi / 2.f;
  float phi = 0.f;

  demo_subc->add_option("-d,--distance", distance, "Specify observer's distance")->excludes(orthogonal_flag)->default_val(1.f);

  demo_subc
      ->add_option_function<float>(
          "--theta-deg", [&theta](const float &theta_deg) { theta = theta_deg / 180.f * std::numbers::pi; },
          "Specify observer's angle theta")
      ->default_val(90.f);

  demo_subc
      ->add_option_function<float>(
          "--phi-deg", [&phi](const float &phi_deg) { phi = phi_deg / 180.f * std::numbers::pi; }, "Specify observer's angle phi")
      ->default_val(0.f);

  demo_subc
      ->add_option<int>("--antialiasing", samples_per_pixel_edge,
                        "Specify #samples per pixel edge (square root of #samples per pixel)")
      ->default_val(3);

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
      ->default_val(3);

  // Float variable definition from command line
  std::vector<std::string> definition_strings;
  std::unordered_map<std::string, float> floats_from_cl;
  render_subc->add_option_function<std::vector<std::string>>("--define-float", [&floats_from_cl](const std::vector<std::string> definition_strings){
    for (const auto& def : definition_strings) {
      auto pos = def.find('=');
      if (pos == std::string::npos) {
        throw CLI::ValidationError("Invalid --define format: use name=value");
      }
      std::string name = def.substr(0, pos);        // First pos characters of the string def
      std::string value_string = def.substr(pos+1); // Characters of string def from pos+1 to the end
      float value;
      try {
        value = std::stof(value_string);
      } catch (...) {
        throw CLI::ValidationError("Invalid float value");
      }
      floats_from_cl[name] = value;
    }
  }, "Define named float variables as name=value");

  // DEBUG
  SourceLocation loc{};
  Token token{loc, TokenType::KEYWORD};
  static_assert(std::is_same<decltype(token.type), TokenType>::value, "Wrong Token::type");



  // TODO add algorithm switch after merge of branch pathtracing, and remember to implement  sub-swithces (e. g. ambient_color for
  // point light tracing)

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
    // A. (DEMO) Compute the demo image and save PFM file
    Transformation observer_transformation =
        rotation_z(phi) * rotation_y(std::numbers::pi / 2.f - theta) *
        translation(-VEC_X); // evaluates to (identity * backwards translation) for default values of theta and phi

    std::cout << "Rendering demo image... " << std::flush;
    img = make_demo_image(orthogonal, width, height, observer_transformation, samples_per_pixel_edge);
    std::cout << "Done." << std::endl;

    // Save PFM image
    img->write_pfm(output_file_name + ".pfm");

  } else if (*render_subc) {
    // B. (RENDERER) Parse input file
    std::ifstream is;
    try {
      is.open(source_file_name);
    } catch (const std::exception &err) {
      std::cerr << "Error opening input (source) file. " << err.what() << '\n';
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
    PointLightTracer renderer{scene.world, Color(0.1f, 0.f, 0.1f), Color(0.f, 0.f, 0.f)};

    std::cout << "Rendering image in " << source_file_name << "... " << std::flush;
    tracer.fire_all_rays(renderer);
    std::cout << "Done." << std::endl;

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



std::unique_ptr<HdrImage> make_demo_image(bool orthogonal, int width, int height, const Transformation &obs_transformation,
                                          int samples_per_pixel_edge) {
  // Initialize ImageTracer
  auto img = std::make_unique<HdrImage>(width, height);

  float aspect_ratio = (float)width / height;

  std::shared_ptr<Camera> cam;
  if (orthogonal) {
    // provide aspect ratio and observer transformation
    cam = std::make_shared<OrthogonalCamera>(aspect_ratio, obs_transformation);
  } else {
    // provide default *origin-screen* distance, aspect ratio and observer transformation
    cam = std::make_shared<PerspectiveCamera>(1.f, aspect_ratio, obs_transformation);
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

  // COMMENT TO RESTORE THE OLD DEMO
  auto sphere_material =
      std::make_shared<Material>(std::make_shared<DiffusiveBRDF>(std::make_shared<UniformPigment>(Color(0.5f, 0.5f, 0.5f))));
  for (auto sphere : world->objects) {
    sphere->material = sphere_material;
  }
  auto source = std::make_shared<PointLightSource>();
  world->add_light_source(source);
  PointLightTracer renderer{world, Color(0.1f, 0.f, 0.1f), Color(0.f, 0.f, 0.f)};
  tracer.fire_all_rays(renderer);

  // UNCOMMENT TO RESTORE OLD DEMO
  //   // Perform on/off tracing
  //   tracer.fire_all_rays([&world](Ray ray) -> Color {
  //     return world->on_off_trace(ray);
  //   }); // World::on_off_trace requires three arguments, the first one being the World instance, hence it is not
  //       // compatible with type RaySolver. A lambda wrapping is therefore needed.

  // Return demo image
  return std::move(tracer.image);
}
