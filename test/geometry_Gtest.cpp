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
#include <gtest/gtest.h>
#include <cmath>
#include <iostream>

//------------------------------------------------------------------------------------------------------------
//----------------- TESTS FOR VEC, POINT AND NORMAL -----------------
//-------------------------------------------------------------------------------------------------------------

TEST(GeometryTest, test_vectors) {
  Vec a(1.0f, 2.0f, 3.0f);
  Vec b(4.0f, 6.0f, 8.0f);

  EXPECT_TRUE(a.is_close(a));
  EXPECT_FALSE(a.is_close(b));
}

TEST(GeometryTest, test_vector_operations) {
  Vec a(1.0f, 2.0f, 3.0f);
  Vec b(4.0f, 6.0f, 8.0f);

  EXPECT_TRUE((-a).is_close(Vec(-1.0f, -2.0f, -3.0f)));
  EXPECT_TRUE((a + b).is_close(Vec(5.0f, 8.0f, 11.0f)));
  EXPECT_TRUE((b - a).is_close(Vec(3.0f, 4.0f, 5.0f)));
  EXPECT_TRUE((a * 2.0f).is_close(Vec(2.0f, 4.0f, 6.0f)));

  EXPECT_TRUE(are_close(a * b, 40.0f));
  EXPECT_TRUE((a ^ b).is_close(Vec(-2.0f, 4.0f, -2.0f)));
  EXPECT_TRUE((b ^ a).is_close(Vec(2.0f, -4.0f, 2.0f)));

  EXPECT_TRUE(are_close(a.squared_norm(), 14.0f));
  EXPECT_TRUE(are_close(std::pow(a.norm(), 2.0f), 14.0f));
}

TEST(GeometryTest, test_points) {
  Point a(1.0f, 2.0f, 3.0f);
  Point b(4.0f, 6.0f, 8.0f);

  EXPECT_TRUE(a.is_close(a));
  EXPECT_FALSE(a.is_close(b));
}

TEST(GeometryTest, test_point_operations) {
  Point p1(1.0f, 2.0f, 3.0f);
  Vec v(4.0f, 6.0f, 8.0f);
  Point p2(4.0f, 6.0f, 8.0f);

  EXPECT_TRUE((p1 * 2.0f).is_close(Point(2.0f, 4.0f, 6.0f)));
  EXPECT_TRUE((p1 + v).is_close(Point(5.0f, 8.0f, 11.0f)));
  EXPECT_TRUE((p2 - p1).is_close(Vec(3.0f, 4.0f, 5.0f)));
  EXPECT_TRUE((p1 - v).is_close(Point(-3.0f, -4.0f, -5.0f)));
}

// ------------------------------------------------------------------------------------------------------------
// -------------------------TESTS FOR TRANSFORMATIONS-----------------
// ------------------------------------------------------------------------------------------------------------

TEST(TransformationTest, test_is_consistent) {
  std::array<std::array<float, 3>, 3> lin = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec t(4., 8., 7.);
  Vec inv_t(0., 1., -2.);

  Transformation T(lin, inv_lin, t, inv_t);
  EXPECT_TRUE(T.is_consistent());

  // Modify copy and check inconsistency
  Transformation T_bad = T;
  T_bad.hom_matrix.linear_part[2][2] += 1;
  EXPECT_FALSE(T.is_close(T_bad));
  EXPECT_FALSE(T_bad.is_consistent());
}

TEST(TransformationTest, test_multiplication) {
  std::array<std::array<float, 3>, 3> A = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> A_inv = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec At(4., 8., 7.);
  Vec At_inv(0., 1., -2.);
  Transformation T1(A, A_inv, At, At_inv);
  EXPECT_TRUE(T1.is_consistent());

  std::array<std::array<float, 3>, 3> B = {{{2., 6., 4.}, {0., 3., 5.}, {1., 2., 1.}}};
  std::array<std::array<float, 3>, 3> B_inv = {{{-1.75, 0.5, 4.5}, {1.25, -0.5, -2.5}, {-0.75, 0.5, 1.5}}};
  Vec Bt(3., 2., 6.);
  Vec Bt_inv(-22.75, 12.25, -7.75);
  Transformation T2(B, B_inv, Bt, Bt_inv);
  EXPECT_TRUE(T2.is_consistent());

  Transformation Tprod = T1 * T2;
  EXPECT_TRUE(Tprod.is_consistent());

  std::array<std::array<float, 3>, 3> C = {{{5., 18., 17.}, {17., 62., 57.}, {26., 97., 89.}}};
  std::array<std::array<float, 3>, 3> C_inv = {
      {{-0.6875, 2.9375, -1.75}, {-1.9375, 0.1875, 0.25}, {2.3125, -1.0625, 0.25}}};
  Vec Ct(29., 77., 100.);
  Vec Ct_inv(-31.25, 16.75, -10.25);
  Transformation Texpected(C, C_inv, Ct, Ct_inv);

  EXPECT_TRUE(Tprod.is_close(Texpected));
}

TEST(TransformationTest, test_vec_point_multiplication) {
  std::array<std::array<float, 3>, 3> lin = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec t(4., 8., 7.);
  Vec inv_t(0., 1., -2.);

  Transformation T(lin, inv_lin, t, inv_t);

  Vec input_v(1., 2., 3.);
  Vec expected_v(14., 38., 51.);
  EXPECT_TRUE(expected_v.is_close(T * input_v));

  Point input_p(1., 2., 3.);
  Point expected_p(18., 46., 58.);
  EXPECT_TRUE(expected_p.is_close(T * input_p));

  Normal input_n(3., 2., 4.);
  Normal expected_n(-8.75, 7.75, -3.0);
  EXPECT_TRUE(expected_n.is_close(T * input_n)); // this test initially failed since original implementation of T*normal
                                                 // automatically normalized the result (now nomore)
}

TEST(TransformationTest, test_inverse) {
  std::array<std::array<float, 3>, 3> lin = {{{1., 2., 3.}, {5., 6., 7.}, {9., 9., 8.}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75, 2.75, -1.}, {5.75, -4.75, 2.0}, {-2.25, 2.25, -1.0}}};
  Vec t(4., 8., 7.);
  Vec inv_t(0., 1., -2.);

  Transformation T(lin, inv_lin, t, inv_t);
  Transformation T_inv = T.inverse();

  EXPECT_TRUE(T_inv.is_consistent());
  EXPECT_TRUE((T * T_inv).is_close(Transformation()));
}

TEST(TransformationTest, test_rotations) {
  EXPECT_TRUE(rotation_x(0.1).is_consistent());
  EXPECT_TRUE(rotation_y(0.1).is_consistent());
  EXPECT_TRUE(rotation_z(0.1).is_consistent());

  EXPECT_TRUE((rotation_x(0.5 * std::numbers::pi) * VEC_Y).is_close(VEC_Z));
  EXPECT_TRUE((rotation_y(0.5 * std::numbers::pi) * VEC_Z).is_close(VEC_X));
  EXPECT_TRUE((rotation_z(0.5 * std::numbers::pi) * VEC_X).is_close(VEC_Y));
}

TEST(TransformationTest, test_translations) {
  Transformation tr1 = translation(Vec(1., 2., 3.));
  Transformation tr2 = translation(Vec(4., 6., 8.));

  EXPECT_TRUE(tr1.is_consistent());
  EXPECT_TRUE(tr2.is_consistent());

  Transformation prod = tr1 * tr2;
  EXPECT_TRUE(prod.is_consistent());

  Transformation expected = translation(Vec(5., 8., 11.));
  EXPECT_TRUE(prod.is_close(expected));
}

TEST(TransformationTest, test_scalings) {
  Transformation sc1 = scaling({2., 5., 10.});
  Transformation sc2 = scaling({3., 2., 4.});

  EXPECT_TRUE(sc1.is_consistent());
  EXPECT_TRUE(sc2.is_consistent());

  Transformation expected = scaling({6., 10., 40.});

  EXPECT_TRUE(expected.is_close(sc1 * sc2));
}
