#include "CLI11.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "renderers.hpp"
#include "shapes.hpp"
#include <fstream>
#include <iostream>

//---------------------------------------------
//---------- FORWARD DECLARATIONS ----------
//---------------------------------------------
// generate demo image with on off rendering
std::unique_ptr<HdrImage> make_demo_image_onoff(bool orthogonal, int width, int height, const Transformation &obs_transformation);
// generate demo image with flat rendering
std::unique_ptr<HdrImage> make_demo_image_flat(bool orthogonal, int width, int height, const Transformation &obs_transformation);
// generate demo image with Monte Carlo path tracing rendering
std::unique_ptr<HdrImage> make_demo_image_path(bool orthogonal, int width, int height, const Transformation &obs_transformation);

enum class DemoMode {
  OnOff,
  Flat,
  Path // Path tracing demo is not implemented yet, but will be
};

//-------------------------------------------------------------------
//---------- MAIN FUNCTION ----------
//-------------------------------------------------------------------

int main(int argc, char **argv) {
  CLI::App app{"Raytracer"};    // Define the main CLI11 App
  argv = app.ensure_utf8(argv); // Ensure utf8 standard

  // The main App requires exactly one subcommand from the command line: the user has to choose between pfm2png and demo mode
  app.require_subcommand(1);

  // -----------------------------------------------------------
  // Parameters common to both demo and pfm2png modes
  // -----------------------------------------------------------

  // Default values of gamma and alpha for HDR to LDR image conversion (may be overwritten in pfm2png mode)
  float gamma = 2.2f;
  float alpha = 0.18f;

  // Name of output file
  std::string output_file_name;

  // -----------------------------------------------------------
  // Command line parsing for demo mode
  // -----------------------------------------------------------

  // Add a subcommand for the demo mode and provide a description
  auto demo_subc =
      app.add_subcommand("demo", "Run demo rendering and save PFM and PNG files"); // Returns a pointer to an App object

  // Add option for on_off, flat or MonteCarlo pathtracing rendering and provide description in the help output
  // default demo mode is on_off rendering
  std::string mode_str = "onoff";
  DemoMode demo_mode;
  demo_subc->add_option("-m,--mode", mode_str, "Rendering mode: on/off (default), flat, or MonteCarlo path tracing")
      ->check(CLI::IsMember({"onoff", "flat", "path"}))
      ->default_val("onoff");

  // Add options for image width and height (# pixels) and provide description in the help output
  // Default values 1280x960, but can be overwritten by command line arguments
  int width = 1280;
  int height = 960;
  demo_subc->add_option("--width", width, "Specify image width")
      ->check(CLI::PositiveNumber)
      ->default_val(1280); // reject negative values
  demo_subc->add_option("--height", height, "Specify image height")
      ->check(CLI::PositiveNumber)
      ->default_val(960); // reject negative values
  ;

  // Add option for perspective or orthogonal projection and provide description in the help output
  bool orthogonal = false;
  auto orthogonal_flag = demo_subc->add_flag("--orthogonal", orthogonal, "Use orthogonal projection (default is perspective)");

  // Add option for output file name and provide description in the help output
  // Default value is "demo", but can be overwritten by command line arguments
  demo_subc->add_option("-o,--output-file", output_file_name, "Insert name of the output PNG file")->default_val("demo");

  // Add option for observer transformation: composition of a translation along -VEC_X and rotation around the scene
  // endcoded in distance, angles phi and angle theta (colatitude, theta=0 north pole). Default position: Origin - VEC_X.
  float distance = 1.f;
  float theta = std::numbers::pi / 2.f;
  float phi = 0.f;

  // specific transformations only make sense for perspective camera
  // if user selects a transformation, orthogonal flag is automatically set to false
  demo_subc->add_option("-d,--distance", distance, "Specify observer's distance")->excludes(orthogonal_flag)->default_val(1.f);

  demo_subc
      ->add_option_function<float>(
          "--theta-deg", [&theta](const float &theta_deg) { theta = (theta_deg / 180.f) * std::numbers::pi; },
          "Specify observer's colatitude angle theta (default is 90 degrees, observer at equator)")
      ->default_val(90.f);

  demo_subc
      ->add_option_function<float>(
          "--phi-deg", [&phi](const float &phi_deg) { phi = (phi_deg / 180.f) * std::numbers::pi; },
          "Specify observer's longitude angle phi (default is 0 degrees, observer at prime meridian)")
      ->default_val(0.f);
  ;

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
        rotation_z(phi) * rotation_y(std::numbers::pi / 2.f - theta) * translation(-VEC_X * distance);

    // check which demo mode is selected
    if (mode_str == "onoff") {
      demo_mode = DemoMode::OnOff;
    } else if (mode_str == "flat") {
      demo_mode = DemoMode::Flat;
    } else if (mode_str == "path") {
      demo_mode = DemoMode::Path;
    }

    // Generate the demo image accordingly
    std::cout << "Rendering demo image... " << std::flush;

    if (demo_mode == DemoMode::Flat) {
      img = make_demo_image_flat(orthogonal, width, height, observer_transformation);
    } else if (demo_mode == DemoMode::OnOff) {
      img = make_demo_image_onoff(orthogonal, width, height, observer_transformation);
    } else if (demo_mode == DemoMode::Path) {
      img = make_demo_image_path(orthogonal, width, height, observer_transformation); // placeholder
    }

    std::cout << "Done." << std::endl;

    // Save PFM image
    img->write_pfm(output_file_name + ".pfm");

  } else if (*pfm2png_subc) {
    // B. (CONVERTER) Read input image from file
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

//-------------------------------------------------------------------
//-------------- HELPER FUNCTIONS --------------
//-------------------------------------------------------------------

//------------- OnOff Tracing Demo Image -------------

std::unique_ptr<HdrImage> make_demo_image_onoff(bool orthogonal, int width, int height,
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
  tracer.fire_all_rays(OnOffTracer(world));

  // Return demo image
  return std::move(tracer.image);
}

//--------------------- Flat Tracing demo image ---------------------

std::unique_ptr<HdrImage> make_demo_image_flat(bool orthogonal, int width, int height, const Transformation &obs_transformation) {
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
  auto world = std::make_shared<World>();

  scaling sc({0.1f, 0.1f, 0.1f}); // common scaling for all spheres

  std::vector<Vec> sphere_positions = {{0.5f, 0.5f, 0.5f},  {0.5f, 0.5f, -0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, -0.5f},
                                       {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f},
                                       {0.0f, 0.0f, -0.5f}, {0.0f, 0.5f, 0.0f}};

  // set colors for spheres
  std::shared_ptr<Pigment> only_emitted_radiance =
      std::make_shared<CheckeredPigment>(Color(1.f, 0.f, 0.f), Color(0.f, 0.f, 1.f), 4);
  std::shared_ptr<DiffusiveBRDF> diffusive_brdf = std::make_shared<DiffusiveBRDF>(only_emitted_radiance);
  std::shared_ptr<Material> material = std::make_shared<Material>(diffusive_brdf, only_emitted_radiance);

  // add spheres to the world
  for (const Vec &pos : sphere_positions) {
    auto sphere = std::make_shared<Sphere>(translation(pos) * sc, material);
    world->add_object(sphere);
  }

  // Perform on/off tracing
  tracer.fire_all_rays(FlatTracer(world));

  // Return demo image
  return std::move(tracer.image);
}

//--------------------- MonteCarlo path tracing demo image ---------------------

//TODO check implementation and take care of camera orientation (see demo in Tomasi Pytracer)


std::unique_ptr<HdrImage> make_demo_image_path(bool orthogonal, int width, int height, const Transformation &obs_transformation) {
  // 1. Create World
  std::shared_ptr<World> world = std::make_shared<World>();

  // 2. Define Pigments and Materials
  auto sky_emission = std::make_shared<UniformPigment>(Color(0.7f, 0.5f, 1.0f));
  auto black_pigment = std::make_shared<UniformPigment>(BLACK);
  auto sky_material = std::make_shared<Material>(std::make_shared<DiffusiveBRDF>(black_pigment), sky_emission);

  auto ground_pattern = std::make_shared<CheckeredPigment>(Color(0.3f, 0.5f, 0.1f), Color(0.1f, 0.2f, 0.5f), 4);
  auto ground_material = std::make_shared<Material>(std::make_shared<DiffusiveBRDF>(ground_pattern),
                                                    std::make_shared<UniformPigment>(Color(0.f, 0.f, 0.f)));

  auto grey_pigment = std::make_shared<UniformPigment>(Color(0.5f, 0.5f, 0.5f));
  auto sphere_material = std::make_shared<Material>(std::make_shared<SpecularBRDF>(grey_pigment),
                                                    std::make_shared<UniformPigment>(Color(0.f, 0.f, 0.f)));

  // 3. Add objects
  Transformation sky_transform = rotation_y(5.f * std::numbers::pi / 6.f)* translation(Vec(0.f, 0.f, 100.f));
  world->add_object(std::make_shared<Plane>(sky_transform, sky_material));
  world->add_object(std::make_shared<Plane>(translation(Vec(0.f, 0.f, -2.f)), ground_material));
  world->add_object(std::make_shared<Sphere>(translation(Vec(0.f, 0.f, 1.f)), sphere_material));

  // 4. Add light source //NOTE off for now, it is in Tomasi, maybe turn it on later
 // world->add_light(std::make_shared<PointLight>(Point(10.f, 10.f, 10.f), Color(1.f, 1.f, 1.f), 1.0f));

  // 5. Setup camera
  std::unique_ptr<Camera> camera;
  //Transformation my_obs_transformation = translation(Vec(-4.f, 0.f, 1.f)) * rotation_z(30.f * std::numbers::pi / 180.f);

  if (orthogonal) { 
    camera = std::make_unique<OrthogonalCamera>(static_cast<float>(height) / width, obs_transformation);
  } else {
    camera = std::make_unique<PerspectiveCamera>(1.f, static_cast<float>(height) / width, obs_transformation);
  }

  // 6. Render image with Montecarlo path tracing
  auto pcg = std::make_shared<PCG>();
  //PathTracer path_tracer(world, pcg, 10, 2, 4); // tweakable: n_rays, roulette limit, max_depth
  FlatTracer flat_tracer(world, Color(1.f, 0.f, 0.f));

  // 7. Trace the image
  auto image = std::make_unique<HdrImage>(width, height);
  ImageTracer image_tracer(std::move(image), std::move(camera));
  //image_tracer.fire_all_rays(path_tracer);
  image_tracer.fire_all_rays(flat_tracer);

  return std::move(image_tracer.image);
}

//-------------------------------------------------------------------------------------------------------------------------------
// ---------------------------OLD MAIN BODY (for reference)---------------------------
//--------------------------------------------------------------------------------------------------------------------------------

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
