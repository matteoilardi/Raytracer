// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR GEOMETRIC OPERATIONS ON IMAGES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// -------------LIBRARIES---------------------------------------------
// ------------------------------------------------------------------------------------------------------------

#include "colors.hpp"
#include "geometry.hpp"
#include "stb_image_write.h" //external library for LDR images
#include <cassert>
#include <cmath>
#include <iostream>


//------------------------------------------------------------------------------------------------------------
//----------------- TESTS FOR VEC, POINT AND NORMAL -----------------
//-------------------------------------------------------------------------------------------------------------

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

// wrapper function to call all test on Vec, Point and Normal
void test_all_geometry() {
  test_vectors();
  test_vector_operations();
  test_points();
  test_point_operations();
  std::cout << "All geometry tests on Vec, Point and Normal passed!" << std::endl;
}


// ------------------------------------------------------------------------------------------------------------
// -------------------------TESTS FOR TRANSFORMATIONS-----------------
// ------------------------------------------------------------------------------------------------------------


void test_is_consistent() {
  std::array<std::array<float, 3>, 3> lin = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec t(4., 8., 7.);
  Vec inv_t(0., 1., -2.);

  Transformation T(lin, inv_lin, t, inv_t);
  assert(T.is_consistent());

  // Modify copy and check inconsistency
  Transformation T_bad = T;
  T_bad.hom_matrix.linear_part[2][2] += 1;
  assert(!T.is_close(T_bad));
  assert(!T_bad.is_consistent());
}

void test_multiplication() {
  std::array<std::array<float, 3>, 3> A = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> A_inv = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec At(4., 8., 7.);
  Vec At_inv(0., 1., -2.);
  Transformation T1(A, A_inv, At, At_inv);

  assert(T1.is_consistent());

  std::array<std::array<float, 3>, 3> B = {{{2., 6., 4.}, {0., 3., 5.}, {1., 2., 1.}}};
  std::array<std::array<float, 3>, 3> B_inv = {{{-1.75, 0.5, 4.5}, {1.25, -0.5, -2.5}, {-0.75, 0.5, 1.5}}};
  Vec Bt(3., 2., 6.);
  Vec Bt_inv(-22.75, 12.25, -7.75);
  Transformation T2(B, B_inv, Bt, Bt_inv);

  assert(T2.is_consistent());
  Transformation Tprod = T1 * T2;

  assert(Tprod.is_consistent());

  std::array<std::array<float, 3>, 3> C = {{{5., 18., 17.}, {17., 62., 57.}, {26., 97., 89.}}};
  std::array<std::array<float, 3>, 3> C_inv = {{{-0.6875, 2.9375, -1.75}, {-1.9375, 0.1875, 0.25}, {2.3125, -1.0625, 0.25}}};
  Vec Ct(29., 77., 100.);
  Vec Ct_inv(-31.25, 16.75, -10.25);
  Transformation Texpected(C, C_inv, Ct, Ct_inv);

  assert(Tprod.is_close(Texpected));
}

void test_vec_point_multiplication() {
  std::array<std::array<float, 3>, 3> lin = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec t(4., 8., 7.);
  Vec inv_t(0., 1., -2.);

  Transformation T(lin, inv_lin, t, inv_t);

  Vec input_v(1., 2., 3.);
  Vec expected_v(14., 38., 51.);
  assert(expected_v.is_close(T * input_v));

  Point input_p(1., 2., 3.);
  Point expected_p(18., 46., 58.);
  assert(expected_p.is_close(T * input_p));

  Normal input_n(3., 2., 4.);
  Normal expected_n(-8.75, 7.75, -3.0);
  assert(expected_n.is_close(T * input_n));
}

void test_inverse() {
  std::array<std::array<float, 3>, 3> lin = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec t(4., 8., 7.);
  Vec inv_t(0., 1., -2.);

  Transformation T(lin, inv_lin, t, inv_t);
  Transformation T_inv = T.inverse();

  assert(T_inv.is_consistent());
  assert((T * T_inv).is_close(Transformation()));
}


void test_rotations() {
  assert(rotation_x(0.1).is_consistent());
  assert(rotation_y(0.1).is_consistent());
  assert(rotation_z(0.1).is_consistent());

  assert((rotation_x(0.5*M_PI) * VEC_Y).is_close(VEC_Z));
  assert((rotation_y(0.5*M_PI) * VEC_Z).is_close(VEC_X));
  assert((rotation_z(0.5*M_PI) * VEC_X).is_close(VEC_Y));
}


void test_translations() {
  Transformation tr1 = translation(Vec(1., 2., 3.));
  Transformation tr2 = translation(Vec(4., 6., 8.));

  assert(tr1.is_consistent());
  assert(tr2.is_consistent());


  Transformation prod = tr1 * tr2;
  assert(prod.is_consistent());

  Transformation expected = translation(Vec(5., 8., 11.));
  assert(prod.is_close(expected));
}


void test_scalings() {
  Transformation sc1 = scaling({2., 5., 10.});
  Transformation sc2 = scaling({3., 2., 4.});

  assert(sc1.is_consistent());
  assert(sc2.is_consistent());

  Transformation expected = scaling({6., 10., 40.});

  assert(expected.is_close(sc1 * sc2));
}


// wrapper function to call all tests on transformation
void test_all_transformations() {
  test_is_consistent();
  test_multiplication();
  test_vec_point_multiplication();
  test_inverse();
  test_rotations();
  test_translations();
  test_scalings();
  std::cout << "All transformation tests passed!\n";
}

//-------------------------------------------------------------------------------------------------------------
//----------------- MAIN FUNCTION FOR TESTING -----------------
//-------------------------------------------------------------------------------------------------------------

int main() {

  test_all_geometry();
  test_all_transformations();

  return EXIT_SUCCESS;
}