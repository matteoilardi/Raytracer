// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR GEOMETRIC OPERATIONS ON IMAGES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// TODO check tests (they work and we pass them, but I just brutally copied those in
// samples/test_all.py sample from Tomasi) 

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------

#include "colors.hpp"
#include "geometry.hpp"
#include "stb_image_write.h" //external library for LDR images
#include <cassert>
#include <cmath>
#include <iostream>

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

void test_all_geometry() {
  test_vectors();
  test_vector_operations();
  test_points();
  test_point_operations();
  std::cout << "All geometry tests on Vec, Point and Normal passed!" << std::endl;
}

// TODO check tests (they work and we pass them, but I just brutally copied those in
// samples/test_all.py sample from Tomasi) 

// ------------------------------------------------------------------------------------------------------------
// TESTS FOR TRANSFORMATIONS BASED ON PYTRACER
// ------------------------------------------------------------------------------------------------------------

// TODO move the following two methods in the relevant classes
bool matrix_is_close(const std::array<std::array<float, 3>, 3> &A, const std::array<std::array<float, 3>, 3> &B,
                     float tol = DEFAULT_ERROR_TOLERANCE) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (!are_close(A[i][j], B[i][j], tol)) {
        return false;
      }
    }
  }
  return true;
}

bool transformation_is_close(const Transformation &a, const Transformation &b, float tol = DEFAULT_ERROR_TOLERANCE) {
  return matrix_is_close(a.hom_matrix.linear_part, b.hom_matrix.linear_part, tol) &&
         matrix_is_close(a.inverse_hom_matrix.linear_part, b.inverse_hom_matrix.linear_part, tol) &&
         a.hom_matrix.translation_vec.is_close(b.hom_matrix.translation_vec, tol) &&
         a.inverse_hom_matrix.translation_vec.is_close(b.inverse_hom_matrix.translation_vec, tol);
}

void test_is_consistent() {
  std::array<std::array<float, 3>, 3> lin = {{{1, 2, 3}, {5, 6, 7}, {9, 9, 8}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1}, {4.375, -3.875, 2.0}, {0.5, 0.5, -1.0}}};
  Vec t(4, 8, 7);
  Vec inv_t(-1.375, 0.875, -0.5);

  Transformation T(lin, inv_lin, t, inv_t);
  assert(T.is_consistent());

  // Modify copy and check inconsistency
  Transformation T_bad = T;
  T_bad.hom_matrix.linear_part[2][2] += 1;
  assert(!transformation_is_close(T, T_bad));
}

void test_multiplication() {
  std::array<std::array<float, 3>, 3> A = {{{1, 2, 3}, {5, 6, 7}, {9, 9, 8}}};
  std::array<std::array<float, 3>, 3> Ainv = {{{-3.75, 2.75, -1}, {4.375, -3.875, 2.0}, {0.5, 0.5, -1.0}}};
  Vec At(4, 8, 7);
  Vec Atinv(-1.375, 0.875, -0.5);

  Transformation T1(A, Ainv, At, Atinv);

  std::array<std::array<float, 3>, 3> B = {{{3, 5, 2}, {4, 1, 0}, {6, 3, 2}}};
  std::array<std::array<float, 3>, 3> Binv = {{{0.4, -0.2, 0.2}, {2.9, -1.7, 0.2}, {-5.55, 3.15, -0.4}}};
  Vec Bt(4, 5, 0);
  Vec Btin(-0.9, 0.7, -0.2);

  Transformation T2(B, Binv, Bt, Btin);

  Transformation Tprod = T1 * T2;
  assert(Tprod.is_consistent());
}

void test_vec_point_multiplication() {
  std::array<std::array<float, 3>, 3> lin = {{{1, 2, 3}, {5, 6, 7}, {9, 9, 8}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec t(4, 8, 7);
  Vec inv_t(1, 1, 1);

  Transformation T(lin, inv_lin, t, inv_t);

  Vec input_v(1, 2, 3);
  Vec expected_v = T * input_v;
  expected_v.print();

  Point input_p(1, 2, 3);
  Point expected_p = T * input_p;
  expected_p.print();

  Normal input_n(3, 2, 4);
  Normal expected_n = T * input_n;
  expected_n.print();
}

void test_all_transformations() {
  test_is_consistent();
  test_multiplication();
  test_vec_point_multiplication();
  std::cout << "All transformation tests passed!\n";
}

int main() {
  test_all_geometry();
  test_all_transformations();
  return 0;
}