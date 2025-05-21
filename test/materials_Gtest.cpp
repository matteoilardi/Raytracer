// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR MATERIALS
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// -------------LIBRARIES---------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#include "materials.hpp"
#include <gtest/gtest.h>

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR PIGMENTS-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// test UniformPigment
TEST(PigmentsTest, test_uniform_pigment) {
  Color color = Color(1.f, 2.f, 3.f);
  UniformPigment pigment = UniformPigment(color);

  EXPECT_TRUE(pigment(Vec2d(0.f, 0.f)).is_close_to(color));
  EXPECT_TRUE(pigment(Vec2d(0.f, 1.f)).is_close_to(color));
  EXPECT_TRUE(pigment(Vec2d(1.f, 0.f)).is_close_to(color));
  EXPECT_TRUE(pigment(Vec2d(1.f, 1.f)).is_close_to(color));
}

// test CheckeredPigment
TEST(PigmentsTest, test_checkered_pigment) {
  Color color1 = Color(1.f, 2.f, 3.f);
  Color color2 = Color(10.f, 20.f, 30.f);

  CheckeredPigment pigment = CheckeredPigment(color1, color2, 2);

  // With n_intervals == 2, the pattern should be the following:
  //
  //              (0.5, 0)
  //   (0, 0) +------+------+ (1, 0)
  //          |      |      |
  //          | col1 | col2 |
  //          |      |      |
  // (0, 0.5) +------+------+ (1, 0.5)
  //          |      |      |
  //          | col2 | col1 |
  //          |      |      |
  //   (0, 1) +------+------+ (1, 1)
  //              (0.5, 1)

  EXPECT_TRUE(pigment(Vec2d(0.25f, 0.25f)).is_close_to(color1));
  EXPECT_TRUE(pigment(Vec2d(0.25f, 0.75f)).is_close_to(color2));
  EXPECT_TRUE(pigment(Vec2d(0.75f, 0.25f)).is_close_to(color2));
  EXPECT_TRUE(pigment(Vec2d(0.75f, 0.75f)).is_close_to(color1));
}
