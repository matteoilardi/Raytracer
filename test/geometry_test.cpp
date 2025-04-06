// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR GEOMETRIC OPERATIONS ON IMAGES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// TODO check tests (they work and we pass them, but I just brutally copied those in
// samples/test_all.py sample from Tomasi) and and implement further
// test for other operations

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------

#include "colors.hpp"
#include "geometry.hpp"
#include "stb_image_write.h" //external library for LDR images
#include <cassert>
#include <cmath>

void test_vectors() {
  Vec a(1.0f, 2.0f, 3.0f);
  Vec b(4.0f, 6.0f, 8.0f);

  assert(a.is_close(a));
  assert(!a.is_close(b));
}

void test_vector_operations() {
  Vec a(1.0f, 2.0f, 3.0f);
  Vec b(4.0f, 6.0f, 8.0f);

  assert((-a).is_close(Vec(-1.0f, -2.0f, -3.0f)));
  assert((a + b).is_close(Vec(5.0f, 8.0f, 11.0f)));
  assert((b - a).is_close(Vec(3.0f, 4.0f, 5.0f)));
  assert((a * 2.0f).is_close(Vec(2.0f, 4.0f, 6.0f)));

  assert(are_close(a * b, 40.0f));
  assert((a ^ b).is_close(Vec(-2.0f, 4.0f, -2.0f)));
  assert((b ^ a).is_close(Vec(2.0f, -4.0f, 2.0f)));

  assert(are_close(a.squared_norm(), 14.0f));
  assert(are_close(std::pow(a.norm(), 2.0f), 14.0f));
}

void test_points() {
  Point a(1.0f, 2.0f, 3.0f);
  Point b(4.0f, 6.0f, 8.0f);

  assert(a.is_close(a));
  assert(!a.is_close(b));
}

void test_point_operations() {
  Point p1(1.0f, 2.0f, 3.0f);
  Vec v(4.0f, 6.0f, 8.0f);
  Point p2(4.0f, 6.0f, 8.0f);

  assert((p1 * 2.0f).is_close(Point(2.0f, 4.0f, 6.0f)));
  assert((p1 + v).is_close(Point(5.0f, 8.0f, 11.0f)));
  assert((p2 - p1).is_close(Vec(3.0f, 4.0f, 5.0f)));
  assert((p1 - v).is_close(Point(-3.0f, -4.0f, -5.0f)));
}

int main() {
  test_vectors();
  test_vector_operations();
  test_points();
  test_point_operations();

  std::cout << "All geometry tests on Vec, Point and Normal passed!" << std::endl;
  return 0;
}

//TODO implement tests for transformations (see samples/test_all.py from Tomasi)
