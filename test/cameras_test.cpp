// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR CAMERAS AND RAYTRACING OPERATIONS
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// -------------LIBRARIES---------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#include "cameras.hpp"
#include "colors.hpp"
#include "geometry.hpp"

void test_ray() {
  Ray ray1 = Ray(Point(1.0, 2.0, 3.0), Vec(5.0, 4.0, -1.0));
  Ray ray2 = Ray(Point(1.0, 2.0, 3.0), Vec(5.0, 4.0, -1.0));
  Ray ray3 = Ray(Point(5.0, 1.0, 4.0), Vec(3.0, 9.0, 4.0));

  assert(ray1.is_close(ray2));
  assert(!ray1.is_close(ray3));

  Ray ray4 = Ray(Point(1.0, 2.0, 4.0), Vec(4.0, 2.0, 1.0));

  assert(ray4.at(0.0).is_close(ray4.origin));
  assert(ray4.at(1.0).is_close(Point(5.0, 4.0, 5.0)));
  assert(ray4.at(2.0).is_close(Point(9.0, 6.0, 6.0)));
}

void test_ray_transformation() {
  Ray ray = Ray(Point(1.0, 2.0, 3.0), Vec(6.0, 5.0, 4.0));
  Transformation T = translation(Vec(10.0, 11.0, 12.0)) *
                     rotation_x(0.5 * M_PI); // Beware that std::sin accepts the angle measured in rads
  Ray transformed = ray.transform(T);

  assert(transformed.origin.is_close(Point(11.0, 8.0, 14.0)));
  assert(transformed.direction.is_close(Vec(6.0, -4.0, 5.0)));
}

// test firing rays from orthogonal camera
void test_orthogonal_camera() {
  OrthogonalCamera cam1(2.0); // aspect ratio 2, transformation set to identity by default constructor
  Ray ray1 = cam1.fire_ray(0., 0.);
  Ray ray2 = cam1.fire_ray(1., 0.);
  Ray ray3 = cam1.fire_ray(0., 1.);
  Ray ray4 = cam1.fire_ray(1., 1.);

  // verify rays from orthogonal camera are all parallel by checking that cross products of directions vanish
  assert(are_close((ray1.direction ^ ray2.direction).squared_norm(), 0.));
  assert(are_close((ray1.direction ^ ray3.direction).squared_norm(), 0.));
  assert(are_close((ray1.direction ^ ray4.direction).squared_norm(), 0.));

  // verify that rays hitting the corners of the screen have the right coordinates
  //(orthogonal camera is at distance -1 from the screen and ray directions have x-component 1 by default)
  assert(ray1.at(1.).is_close(Point(0., 2., -1.)));
  assert(ray2.at(1.).is_close(Point(0., -2., -1.)));
  assert(ray3.at(1.).is_close(Point(0., 2., 1.)));
  assert(ray4.at(1.).is_close(Point(0., -2., 1.)));
}

// test transformation to orient orthogonal camera according to the observer
void test_orthogonal_camera_transformation() {
  OrthogonalCamera cam2{1., translation(-VEC_Y * 2) * rotation_z(0.5 * M_PI)};
  Ray ray5 = cam2.fire_ray(0.5, 0.5);

  // check ray fired after transformation is at the expected coordinates
  //  (orthogonal camera is at distance -1 from the screen by default)
  assert(ray5.at(1.0).is_close(Point(0., -2., 0.)));
}

void test_perspective_camera() {
  PerspectiveCamera cam1(1.0, 2.0); // distance 1, aspect ratio 2, identity transformation by default
  Ray ray1 = cam1.fire_ray(0., 0.);
  Ray ray2 = cam1.fire_ray(1., 0.);
  Ray ray3 = cam1.fire_ray(0., 1.);
  Ray ray4 = cam1.fire_ray(1., 1.);

  // verify rays from perspective camera all start from the same point (cf. case of orthogonal camera)
  assert(ray1.origin.is_close(ray2.origin));
  assert(ray1.origin.is_close(ray3.origin));
  assert(ray1.origin.is_close(ray4.origin));

  // verify rays hitting the corners of the screen have the right coordinates
  //(perspective camera is at distance -d from screen, but ray directions have x-component equal to d)
  assert(ray1.at(1.).is_close(Point(0., 2., -1.)));
  assert(ray2.at(1.).is_close(Point(0., -2., -1.)));
  assert(ray3.at(1.).is_close(Point(0., 2., 1.)));
  assert(ray4.at(1.).is_close(Point(0., -2., 1.)));
}

// test transformation to orient perspective camera according to the observer
void test_perspective_camera_transformation() {
  PerspectiveCamera cam2{1., 1., translation(-VEC_Y * 2) * rotation_z(0.5 * M_PI)};
  Ray ray5 = cam2.fire_ray(0.5, 0.5);
  PerspectiveCamera cam3{1., 1., translation(-VEC_Z * 3) * rotation_y(0.5 * M_PI)};
  Ray ray6 = cam3.fire_ray(0.5, 0.5);

  // check ray fired after transformation is at the expected coordinates
  //  (perspective camera is at distance -d from the screen, but ray directions have x-component equal to d)
  assert(ray5.at(1.0).is_close(Point(0., -2., 0.)));
  assert(ray6.at(1.0).is_close(Point(0., 0., -3.)));
}

// test methods of ImageTracer class
void test_image_tracer() {
  std::unique_ptr<HdrImage> img = std::make_unique<HdrImage>(4, 2);
  std::unique_ptr<Camera> cam = std::make_unique<PerspectiveCamera>(1., 2.);
  ImageTracer tracer(std::move(img), std::move(cam));
  // ImageTracer tracer(std::make_unique<HdrImage>(4, 2), std::make_unique<PerspectiveCamera>(1., 2.)); // Same as the
  // three lines above

  // choose on purpose to fire the first ray at the pixel (0,0), not at the center (0.5,0.5)
  // but rather well outside the pixel boundaries so as to hit the center of the pixel (2,1)
  Ray ray1 = tracer.fire_ray(0., 0., 2.5, 1.5);

  // fire the second ray at the center of the pixel (2,1)
  Ray ray2 = tracer.fire_ray(2., 1.);
  assert(ray1.is_close(ray2));

  tracer.fire_all_rays([](Ray ray) -> Color { return Color(1., 2., 3.); });

  for (int col = 0; col < tracer.image->width; ++col) {
    for (int row = 0; row < tracer.image->height; ++row) {
      assert(are_close(tracer.image->get_pixel(col, row), Color(1., 2., 3.)));
    }
  }
}

//NEW additional test for image tracer vs HdrImage orientation highlighting a bug in original code (issue #4)
void test_image_tracer_coordinate_orientation() {
  std::unique_ptr<HdrImage> img = std::make_unique<HdrImage>(4, 2);
  std::unique_ptr<Camera> cam = std::make_unique<PerspectiveCamera>(1., 2.);
  ImageTracer tracer(std::move(img), std::move(cam));

  // fire a ray against top left corner of the screen
  // you expect this ray to hit the screen at (x=0, y=2, z=1) but using the original code it would hit (x=0, y=2, z=-1)
  // (see BUG tag in cameras.hpp)  since v coordinates increase upwards while HdrImage rows are ordered top to bottom
  Ray top_left_ray = tracer.fire_ray(0., 0., 0., 0.);
  assert(Point(0., 2., 1.).is_close(top_left_ray.at(1.)));

  // fire a ray against bottom right corner of the screen
  // you expect this ray to hit the screen at (x=0, y=-2, z=-1) but with original code it hits (x=0, y=-3.333, z=3)
  // (see BUG tag in cameras.hpp) since we divided by height/height-1 rather than width/height
  // & since coordinates increase upwards while HdrImage rows are ordered top to bottom
  Ray bottom_right_ray = tracer.fire_ray(3., 1., 1., 1.);
  assert(Point(0., -2, -1.).is_close(bottom_right_ray.at(1.)));
}

int main() {

  test_ray();

  test_ray_transformation();

  test_orthogonal_camera();

  test_orthogonal_camera_transformation();

  test_perspective_camera();

  test_perspective_camera_transformation();

  test_image_tracer();

  test_image_tracer_coordinate_orientation(); // additional test showing a bug in original code (issue #4)

  std::cout << "All cameras tests passed!" << std::endl;

  return EXIT_SUCCESS;
}
