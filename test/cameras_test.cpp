// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR CAMERAS AND RAYTRACING OPERATIONS 
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// -------------LIBRARIES---------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#include "colors.hpp"
#include "geometry.hpp"
#include "cameras.hpp"

// TODO decide whether we should divide the test in functions and name the these functions in the same exact way as Tomasi
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
  Transformation T = translation(Vec(10.0, 11.0, 12.0)) * rotation_x(0.5 * M_PI); // Beware that std::sin accepts the angle measured in rads
  Ray transformed = ray.transform(T);

  assert(transformed.origin.is_close(Point(11.0, 8.0, 14.0)));
  assert(transformed.direction.is_close(Vec(6.0, -4.0, 5.0)));
}

void test_orthogonal_camera() {
  OrthogonalCamera cam1(2.0); // aspect ratio 2, identity transformation by default
  Ray ray1 = cam1.fire_ray(0., 0.);
  Ray ray2 = cam1.fire_ray(1., 0.);
  Ray ray3 = cam1.fire_ray(0., 1.);
  Ray ray4 = cam1.fire_ray(1., 1.);

  assert(are_close((ray1.direction^ray2.direction).squared_norm(), 0.));
  assert(are_close((ray1.direction^ray3.direction).squared_norm(), 0.));
  assert(are_close((ray1.direction^ray4.direction).squared_norm(), 0.));

  assert(ray1.at(1.).is_close(Point(0., 2., -1.)));
  assert(ray2.at(1.).is_close(Point(0., -2., -1.)));
  assert(ray3.at(1.).is_close(Point(0., 2., 1.)));
  assert(ray4.at(1.).is_close(Point(0., -2., 1.)));

  OrthogonalCamera cam2{1., translation(-VEC_Y*2) * rotation_z(0.5*M_PI)};
  Ray ray5 = cam2.fire_ray(0.5, 0.5);

  assert(ray5.at(1.0).is_close(Point(0., -2., 0.)));
}

void test_perspective_camera() {
  PerspectiveCamera cam1(1.0, 2.0); // distance 1, aspect ratio 2, identity transformation by default
  Ray ray1 = cam1.fire_ray(0., 0.);
  Ray ray2 = cam1.fire_ray(1., 0.);
  Ray ray3 = cam1.fire_ray(0., 1.);
  Ray ray4 = cam1.fire_ray(1., 1.);

  assert(ray1.origin.is_close(ray2.origin));
  assert(ray1.origin.is_close(ray3.origin));
  assert(ray1.origin.is_close(ray4.origin));

  assert(ray1.at(1.).is_close(Point(0., 2., -1.)));
  assert(ray2.at(1.).is_close(Point(0., -2., -1.)));
  assert(ray3.at(1.).is_close(Point(0., 2., 1.)));
  assert(ray4.at(1.).is_close(Point(0., -2., 1.)));
}


void test_image_tracer() {
  HdrImage img(4, 2);
  ImageTracer tracer(img, std::make_unique<PerspectiveCamera>(1., 2.));

  Ray ray1 = tracer.fire_ray(0., 0., 2.5, 1.5);
  Ray ray2 = tracer.fire_ray(2., 1.);
  assert(ray1.is_close(ray2));

  tracer.fire_all_rays([](Ray ray) -> Color {return Color(1., 2., 3.); });

  for(int col = 0; col < img.width; ++col) {
    for(int row = 0; row < img.height; ++row) {
      assert(are_close(tracer.image.get_pixel(col, row), Color(1., 2., 3.)));
    }
  }
}


int main() {

  test_ray();

  test_ray_transformation();

  test_orthogonal_camera();

  test_perspective_camera();

  test_image_tracer();

  return EXIT_SUCCESS;
}

