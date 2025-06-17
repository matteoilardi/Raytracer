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

// Test UniformPigment
TEST(PigmentsTest, test_uniform_pigment) {
  Color color = Color(1.f, 2.f, 3.f);
  UniformPigment pigment = UniformPigment(color);

  EXPECT_TRUE(pigment(Vec2d(0.f, 0.f)).is_close(color));
  EXPECT_TRUE(pigment(Vec2d(0.f, 1.f)).is_close(color));
  EXPECT_TRUE(pigment(Vec2d(1.f, 0.f)).is_close(color));
  EXPECT_TRUE(pigment(Vec2d(1.f, 1.f)).is_close(color));
}

// Test CheckeredPigment
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

  EXPECT_TRUE(pigment(Vec2d(0.25f, 0.25f)).is_close(color1));
  EXPECT_TRUE(pigment(Vec2d(0.25f, 0.75f)).is_close(color2));
  EXPECT_TRUE(pigment(Vec2d(0.75f, 0.25f)).is_close(color2));
  EXPECT_TRUE(pigment(Vec2d(0.75f, 0.75f)).is_close(color1));
}

// Test ImagePigment
TEST(PigmentsTest, test_image_pigment) {
  HdrImage image(2, 2);
  image.set_pixel(0, 0, Color(1.f, 2.f, 3.f));
  image.set_pixel(1, 0, Color(2.f, 3.f, 1.f));
  image.set_pixel(0, 1, Color(2.f, 1.f, 3.f));
  image.set_pixel(1, 1, Color(3.f, 2.f, 1.f));
  ImagePigment pigment = ImagePigment(image);

  EXPECT_TRUE(pigment(Vec2d(0.f, 0.f)).is_close(Color(1.f, 2.f, 3.f)));
  EXPECT_TRUE(pigment(Vec2d(0.5f, 0.f)).is_close(Color(2.f, 3.f, 1.f)));
  EXPECT_TRUE(pigment(Vec2d(0.f, 0.5f)).is_close(Color(2.f, 1.f, 3.f)));
  EXPECT_TRUE(pigment(Vec2d(0.5f, 0.5f)).is_close(Color(3.f, 2.f, 1.f)));
}

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR BRDF -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// Test SpecularBRDF

TEST(BRDFTest, test_specular_brdf_reflection) {
  // Setup: incident direction and surface normal
  Vec in_dir = Vec(0.f, -1.f, -1.f);
  in_dir = in_dir.normalize();
  Normal normal = Normal(0.f, 0.f, 1.f);
  Point hit_point = Point(0.f, 0.f, 0.f); // arbitrary intersection point

  // Expected reflected direction
  Vec expected_out = Vec(0.f, -1.f, 1.f);
  expected_out = expected_out.normalize();

  // Build the BRDF
  auto pigment = std::make_shared<UniformPigment>(Color(0.5f, 0.5f, 0.5f));
  SpecularBRDF brdf(pigment);
  auto pcg = std::make_shared<PCG>();

  // Call scatter_ray
  Ray scattered = brdf.scatter_ray(pcg, in_dir, hit_point, normal, 1);
  Vec out_dir = scattered.direction;
  out_dir = out_dir.normalize(); // Ensure the direction is normalized

  // Test reflected direction
  EXPECT_TRUE(out_dir.is_close(expected_out));

  // Test eval returns correct color
  Color reflected_color = brdf.eval(normal, in_dir, expected_out, Vec2d(0.5f, 0.5f));
  EXPECT_TRUE(reflected_color.is_close(Color(0.5f, 0.5f, 0.5f)));

  // Should return black for incorrect out direction
  Color wrong_color_1 = brdf.eval(normal, in_dir, VEC_X, Vec2d(0.5f, 0.5f));
  EXPECT_TRUE(wrong_color_1.is_close(BLACK));

  // Should return black for ray hitting from the inside (incidence angle>pi/2)
  Color wrong_color_2 = brdf.eval(normal, VEC_X, -VEC_X, Vec2d(0.5f, 0.5f));
  EXPECT_TRUE(wrong_color_2.is_close(BLACK));
}
