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
#include <gtest/gtest.h>

// ------------------------------------------------------------------------------------------------------------
// -------------------------TESTS FOR RAY-----------------
// ------------------------------------------------------------------------------------------------------------

// test is_close
TEST(RayTest, test_is_close) {
  Ray ray1 = Ray(Point(1.f, 2.f, 3.f), Vec(5.f, 4.f, -1.f));
  Ray ray2 = Ray(Point(1.f, 2.f, 3.f), Vec(5.f, 4.f, -1.f));
  Ray ray3 = Ray(Point(5.f, 1.f, 4.f), Vec(3.f, 9.f, 4.f));

  EXPECT_TRUE(ray1.is_close(ray2));
  EXPECT_FALSE(ray1.is_close(ray3));
}

// test at method
TEST(RayTest, test_at) {
  Ray ray4 = Ray(Point(1.f, 2.f, 4.f), Vec(4.f, 2.f, 1.f));

  EXPECT_TRUE(ray4.at(0.f).is_close(ray4.origin));
  EXPECT_TRUE(ray4.at(1.f).is_close(Point(5.f, 4.f, 5.f)));
  EXPECT_TRUE(ray4.at(2.f).is_close(Point(9.f, 6.f, 6.f)));
}

// test transform method
TEST(RayTest, test_ray_transformation) {
  Ray ray = Ray(Point(1.f, 2.f, 3.f), Vec(6.f, 5.f, 4.f));
  Transformation T = translation(Vec(10.f, 11.f, 12.f)) *
                     rotation_x(0.5f * (float)std::numbers::pi); // Beware that std::sin accepts the angle measured in rads
  Ray transformed = ray.transform(T);

  EXPECT_TRUE(transformed.origin.is_close(Point(11.f, 8.f, 14.f)));
  EXPECT_TRUE(transformed.direction.is_close(Vec(6.f, -4.f, 5.f)));
}

// ------------------------------------------------------------------------------------------------------------
// -------------------------TESTS FOR CAMERA-----------------
// ------------------------------------------------------------------------------------------------------------

// test firing rays from orthogonal camera
TEST(CameraTest, test_orthogonal_camera) {
  OrthogonalCamera cam1(2.f); // aspect ratio 2, transformation set to identity by default constructor
  Ray ray1 = cam1.fire_ray(0.f, 0.f);
  Ray ray2 = cam1.fire_ray(1.f, 0.f);
  Ray ray3 = cam1.fire_ray(0.f, 1.f);
  Ray ray4 = cam1.fire_ray(1.f, 1.f);

  // verify rays from orthogonal camera are all parallel by checking that cross products of directions vanish
  EXPECT_TRUE(are_close((ray1.direction ^ ray2.direction).squared_norm(), 0.f));
  EXPECT_TRUE(are_close((ray1.direction ^ ray3.direction).squared_norm(), 0.f));
  EXPECT_TRUE(are_close((ray1.direction ^ ray4.direction).squared_norm(), 0.f));

  // verify that rays hitting the corners of the screen have the right coordinates
  // (orthogonal camera is at distance -1 from the screen and ray directions have x-component 1 by default)
  EXPECT_TRUE(ray1.at(1.f).is_close(Point(0.f, 2.f, -1.f)));
  EXPECT_TRUE(ray2.at(1.f).is_close(Point(0.f, -2.f, -1.f)));
  EXPECT_TRUE(ray3.at(1.f).is_close(Point(0.f, 2.f, 1.f)));
  EXPECT_TRUE(ray4.at(1.f).is_close(Point(0.f, -2.f, 1.f)));
}

// test transformation to orient orthogonal camera according to the observer
TEST(CameraTest, test_orthogonal_camera_transformation) {
  OrthogonalCamera cam2{1.f, translation(-VEC_Y * 2.f) * rotation_z(0.5f * (float)std::numbers::pi)};
  Ray ray5 = cam2.fire_ray(0.5f, 0.5f);

  // check ray fired after transformation is at the expected coordinates
  // (orthogonal camera is at distance -1 from the screen by default)
  EXPECT_TRUE(ray5.at(1.0f).is_close(Point(0.f, -2.f, 0.f)));
}

// test firing rays from perspective camera
TEST(CameraTest, test_perspective_camera) {
  PerspectiveCamera cam1(1.f, 2.f); // distance 1, aspect ratio 2, identity transformation by default
  Ray ray1 = cam1.fire_ray(0.f, 0.f);
  Ray ray2 = cam1.fire_ray(1.f, 0.f);
  Ray ray3 = cam1.fire_ray(0.f, 1.f);
  Ray ray4 = cam1.fire_ray(1.f, 1.f);

  // verify rays from perspective camera all start from the same point (cf. case of orthogonal camera)
  EXPECT_TRUE(ray1.origin.is_close(ray2.origin));
  EXPECT_TRUE(ray1.origin.is_close(ray3.origin));
  EXPECT_TRUE(ray1.origin.is_close(ray4.origin));

  // verify rays hitting the corners of the screen have the right coordinates
  // (perspective camera is at distance -d from screen, but ray directions have x-component equal to d)
  EXPECT_TRUE(ray1.at(1.f).is_close(Point(0.f, 2.f, -1.f)));
  EXPECT_TRUE(ray2.at(1.f).is_close(Point(0.f, -2.f, -1.f)));
  EXPECT_TRUE(ray3.at(1.f).is_close(Point(0.f, 2.f, 1.f)));
  EXPECT_TRUE(ray4.at(1.f).is_close(Point(0.f, -2.f, 1.f)));
}

// test transformation to orient perspective camera according to the observer
TEST(CameraTest, test_perspective_camera_transformation) {
  PerspectiveCamera cam2{1.f, 1.f, translation(-VEC_Y * 2.f) * rotation_z(0.5f * (float)std::numbers::pi)};
  Ray ray5 = cam2.fire_ray(0.5f, 0.5f);
  PerspectiveCamera cam3{1.f, 1.f, translation(-VEC_Z * 3.f) * rotation_y(0.5f * (float)std::numbers::pi)};
  Ray ray6 = cam3.fire_ray(0.5f, 0.5f);

  // check ray fired after transformation is at the expected coordinates
  //  (perspective camera is at distance -d from the screen, but ray directions have x-component equal to d)
  EXPECT_TRUE(ray5.at(1.f).is_close(Point(0.f, -2.f, 0.f)));
  EXPECT_TRUE(ray6.at(1.f).is_close(Point(0.f, 0.f, -3.f)));
}

// ------------------------------------------------------------------------------------------------------------
// -------------------------TESTS FOR IMAGETRACER-----------------
// ------------------------------------------------------------------------------------------------------------

class ImageTracerTest : public ::testing::Test {
protected:
  std::unique_ptr<ImageTracer> tracer;

  void SetUp() override {
    std::unique_ptr<HdrImage> img = std::make_unique<HdrImage>(4, 2);
    std::unique_ptr<Camera> cam = std::make_unique<PerspectiveCamera>(1.f, 2.f);
    tracer = std::make_unique<ImageTracer>(std::move(img), std::move(cam));
  }
};

// test u_pixel and v_pixel correctly offset the point at which the ray hits the screen with respect to the top left
// corner of the pixel
TEST_F(ImageTracerTest, test_uv_submapping) {
  // choose on purpose to fire the first ray at the pixel (0,0), not at the center coordinates (0.5,0.5)
  // but rather well outside the pixel boundaries so as to hit the center of the pixel (2,1)
  Ray ray1 = tracer->fire_ray(0, 0, 2.5f, 1.5f);

  // fire the second ray at the center of the pixel (2,1)
  Ray ray2 = tracer->fire_ray(2, 1);
  EXPECT_TRUE(ray1.is_close(ray2));
}

// test all pixel are hit by method fire_all_rays
TEST_F(ImageTracerTest, test_pixel_coverage) {
  tracer->fire_all_rays([](Ray ray) -> Color { return Color(1.f, 2.f, 3.f); });

  for (int col = 0; col < tracer->image->width; ++col) {
    for (int row = 0; row < tracer->image->height; ++row) {
      EXPECT_TRUE(are_close(tracer->image->get_pixel(col, row), Color(1.f, 2.f, 3.f)));
    }
  }
}

// test image orientation (see issue #4)
TEST_F(ImageTracerTest, test_image_orientation) {
  // fire a ray against top left corner of the screen
  // you expect this ray to hit the screen at (x=0, y=2, z=1) but using the original code it would hit (x=0, y=2, z=-1)
  // (see issue #4) since v coordinates increase upwards while HdrImage rows are ordered top to bottom
  Ray top_left_ray = tracer->fire_ray(0, 0, 0.f, 0.f);
  EXPECT_TRUE(Point(0.f, 2.f, 1.f).is_close(top_left_ray.at(1.f)));

  // fire a ray against bottom right corner of the screen
  // you expect this ray to hit the screen at (x=0, y=-2, z=-1) but with original code it hits (x=0, y=-3.333, z=3)
  // (see issue #4) since we divided by width-1/height-1 rather than width/height
  // & since coordinates increase upwards while HdrImage rows are ordered top to bottom
  Ray bottom_right_ray = tracer->fire_ray(3, 1, 1.f, 1.f);
  EXPECT_TRUE(Point(0.f, -2.f, -1.f).is_close(bottom_right_ray.at(1.f)));
}
