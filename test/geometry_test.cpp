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
#include "random.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>

//------------------------------------------------------------------------------------------------------------
//----------------- TESTS FOR VEC, POINT AND NORMAL -----------------
//-------------------------------------------------------------------------------------------------------------

TEST(GeometryTest, test_vectors) {
  Vec a{1.f, 2.f, 3.f};
  Vec b{4.f, 6.f, 8.f};

  EXPECT_TRUE(a.is_close(a));
  EXPECT_FALSE(a.is_close(b));
}

TEST(GeometryTest, test_vector_operations) {
  Vec a{1.f, 2.f, 3.f};
  Vec b{4.f, 6.f, 8.f};

  EXPECT_TRUE((-a).is_close(Vec{-1.f, -2.f, -3.f}));
  EXPECT_TRUE((a + b).is_close(Vec{5.f, 8.f, 11.f}));
  EXPECT_TRUE((b - a).is_close(Vec{3.f, 4.f, 5.f}));
  EXPECT_TRUE((a * 2.f).is_close(Vec{2.f, 4.f, 6.f}));

  EXPECT_TRUE(are_close(a * b, 40.f));
  EXPECT_TRUE((a ^ b).is_close(Vec{-2.f, 4.f, -2.f}));
  EXPECT_TRUE((b ^ a).is_close(Vec{2.f, -4.f, 2.f}));

  EXPECT_TRUE(are_close(a.squared_norm(), 14.f));
  EXPECT_TRUE(are_close(std::pow(a.norm(), 2.f), 14.f));
}

TEST(GeometryTest, test_points) {
  Point a{1.f, 2.f, 3.f};
  Point b{4.f, 6.f, 8.f};

  EXPECT_TRUE(a.is_close(a));
  EXPECT_FALSE(a.is_close(b));
}

TEST(GeometryTest, test_point_operations) {
  Point p1{1.f, 2.f, 3.f};
  Vec v{4.f, 6.f, 8.f};
  Point p2{4.f, 6.f, 8.f};

  EXPECT_TRUE((p1 * 2.f).is_close(Point{2.f, 4.f, 6.f}));
  EXPECT_TRUE((p1 + v).is_close(Point{5.f, 8.f, 11.f}));
  EXPECT_TRUE((p2 - p1).is_close(Vec{3.f, 4.f, 5.f}));
  EXPECT_TRUE((p1 - v).is_close(Point{-3.f, -4.f, -5.f}));
}

// ------------------------------------------------------------------------------------------------------------
// -------------------------TESTS FOR TRANSFORMATIONS-----------------
// ------------------------------------------------------------------------------------------------------------

TEST(TransformationTest, test_is_consistent) {
  std::array<std::array<float, 3>, 3> lin = {{{1.f, 2.f, 3.f}, {5.f, 6.f, 7.f}, {9.f, 9.f, 8.f}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75f, 2.75f, -1.f}, {5.75f, -4.75f, 2.f}, {-2.25f, 2.25f, -1.f}}};
  Vec t{4.f, 8.f, 7.f};
  Vec inv_t{0.f, 1.f, -2.f};

  Transformation T{lin, inv_lin, t, inv_t};
  EXPECT_TRUE(T.is_consistent());

  // Modify copy and check inconsistency
  Transformation T_bad = T;
  T_bad.hom_matrix.linear_part[2][2] += 1.f;
  EXPECT_FALSE(T.is_close(T_bad));
  EXPECT_FALSE(T_bad.is_consistent());
}

TEST(TransformationTest, test_multiplication) {
  std::array<std::array<float, 3>, 3> A = {{{1.f, 2.f, 3.f}, {5.f, 6.f, 7.f}, {9.f, 9.f, 8.f}}};
  std::array<std::array<float, 3>, 3> A_inv = {{{-3.75f, 2.75f, -1.f}, {5.75f, -4.75f, 2.f}, {-2.25f, 2.25f, -1.f}}};
  Vec At{4.f, 8.f, 7.f};
  Vec At_inv{0.f, 1.f, -2.f};
  Transformation T1{A, A_inv, At, At_inv};
  EXPECT_TRUE(T1.is_consistent());

  std::array<std::array<float, 3>, 3> B = {{{2.f, 6.f, 4.f}, {0.f, 3.f, 5.f}, {1.f, 2.f, 1.f}}};
  std::array<std::array<float, 3>, 3> B_inv = {{{-1.75f, 0.5f, 4.5f}, {1.25f, -0.5f, -2.5f}, {-0.75f, 0.5f, 1.5f}}};
  Vec Bt{3.f, 2.f, 6.f};
  Vec Bt_inv{-22.75f, 12.25f, -7.75f};
  Transformation T2{B, B_inv, Bt, Bt_inv};
  EXPECT_TRUE(T2.is_consistent());

  Transformation Tprod = T1 * T2;
  EXPECT_TRUE(Tprod.is_consistent());

  std::array<std::array<float, 3>, 3> C = {{{5.f, 18.f, 17.f}, {17.f, 62.f, 57.f}, {26.f, 97.f, 89.f}}};
  std::array<std::array<float, 3>, 3> C_inv = {
      {{-0.6875f, 2.9375f, -1.75f}, {-1.9375f, 0.1875f, 0.25f}, {2.3125f, -1.0625f, 0.25f}}};
  Vec Ct{29.f, 77.f, 100.f};
  Vec Ct_inv{-31.25f, 16.75f, -10.25f};
  Transformation Texpected{C, C_inv, Ct, Ct_inv};

  EXPECT_TRUE(Tprod.is_close(Texpected));
}

TEST(TransformationTest, test_vec_point_multiplication) {
  std::array<std::array<float, 3>, 3> lin = {{{1.f, 2.f, 3.f}, {5.f, 6.f, 7.f}, {9.f, 9.f, 8.f}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75f, 2.75f, -1.f}, {5.75f, -4.75f, 2.f}, {-2.25f, 2.25f, -1.f}}};
  Vec t{4.f, 8.f, 7.f};
  Vec inv_t{0.f, 1.f, -2.f};

  Transformation T{lin, inv_lin, t, inv_t};

  Vec input_v{1.f, 2.f, 3.f};
  Vec expected_v{14.f, 38.f, 51.f};
  EXPECT_TRUE(expected_v.is_close(T * input_v));

  Point input_p{1.f, 2.f, 3.f};
  Point expected_p{18.f, 46.f, 58.f};
  EXPECT_TRUE(expected_p.is_close(T * input_p));

  Normal input_n{3.f, 2.f, 4.f};
  Normal expected_n{-8.75f, 7.75f, -3.f};
  EXPECT_TRUE(expected_n.is_close(T * input_n)); // this test initially failed since original implementation of T*normal
                                                 // automatically normalized the result (now nomore)
}

TEST(TransformationTest, test_inverse) {
  std::array<std::array<float, 3>, 3> lin = {{{1.f, 2.f, 3.f}, {5.f, 6.f, 7.f}, {9.f, 9.f, 8.f}}};
  std::array<std::array<float, 3>, 3> inv_lin = {{{-3.75f, 2.75f, -1.f}, {5.75f, -4.75f, 2.f}, {-2.25f, 2.25f, -1.f}}};
  Vec t{4.f, 8.f, 7.f};
  Vec inv_t{0.f, 1.f, -2.f};

  Transformation T{lin, inv_lin, t, inv_t};
  Transformation T_inv = T.inverse();

  EXPECT_TRUE(T_inv.is_consistent());
  EXPECT_TRUE((T * T_inv).is_close(Transformation{}));
}

TEST(TransformationTest, test_rotations) {
  EXPECT_TRUE(rotation_x{0.1f}.is_consistent());
  EXPECT_TRUE(rotation_y{0.1f}.is_consistent());
  EXPECT_TRUE(rotation_z{0.1f}.is_consistent());

  EXPECT_TRUE((rotation_x{0.5f * std::numbers::pi_v<float>} * VEC_Y).is_close(VEC_Z));
  EXPECT_TRUE((rotation_y{0.5f * std::numbers::pi_v<float>} * VEC_Z).is_close(VEC_X));
  EXPECT_TRUE((rotation_z{0.5f * std::numbers::pi_v<float>} * VEC_X).is_close(VEC_Y));
}

TEST(TransformationTest, test_translations) {
        Transformation tr1 = translation{Vec{1.f, 2.f, 3.f}};
        Transformation tr2 = translation{Vec{4.f, 6.f, 8.f}};

        EXPECT_TRUE(tr1.is_consistent());
        EXPECT_TRUE(tr2.is_consistent());

        Transformation prod = tr1 * tr2;
        EXPECT_TRUE(prod.is_consistent());

        Transformation expected = translation{Vec{5.f, 8.f, 11.f}};
        EXPECT_TRUE(prod.is_close(expected));
}

TEST(TransformationTest, test_scalings) {
        Transformation sc1 = scaling{{2.f, 5.f, 10.f}};
        Transformation sc2 = scaling{{3.f, 2.f, 4.f}};

        EXPECT_TRUE(sc1.is_consistent());
        EXPECT_TRUE(sc2.is_consistent());

        Transformation expected = scaling{{6.f, 10.f, 40.f}};

        EXPECT_TRUE(expected.is_close(sc1 * sc2));
}

// ------------------------------------------------------------------------------------------------------------
// -------------------------TESTS FOR ONB-----------------
// ------------------------------------------------------------------------------------------------------------

TEST(ONBTest, test_is_consistent) {
        ONB world_onb;
        EXPECT_TRUE(world_onb.is_consistent());

        ONB wrong_onb1{VEC_X, VEC_Y, VEC_Y};
        EXPECT_FALSE(wrong_onb1.is_consistent());

        ONB wrong_onb2{1.1f * VEC_X, VEC_Y, VEC_Z};
        EXPECT_FALSE(wrong_onb2.is_consistent());
}

TEST(ONBTest, test_ONB_from_Duff) {
        PCG pcg;

        // perform random testing on 1000 inputs
        for (int i = 0; i < 1e4; i++) {
          auto [theta, phi] = pcg.random_unif_hemisphere(); // uniform sampling of the hemisphere
          Vec v = Vec{theta, phi};
          ONB onb{v};

          EXPECT_TRUE(v.is_close(onb.e3)); // check if e3 is indeed the input vector
          EXPECT_TRUE(onb.is_consistent());
        }
}
