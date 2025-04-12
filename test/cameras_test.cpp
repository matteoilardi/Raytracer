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

int main() {

  test_ray();

  test_ray_transformation();

  return EXIT_SUCCESS;
}

