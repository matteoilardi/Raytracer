// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// TESTS FOR SHAPES
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// -------------LIBRARIES---------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#include "cameras.hpp"
#include "colors.hpp"
#include "geometry.hpp"
#include "shapes.hpp"
#include <gtest/gtest.h>

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR SPHERE-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// test two hits from outside the sphere
TEST(SphereTest, test_outer_hit) {
  Sphere unit_sphere = Sphere();

  Ray ray1 = Ray(Point(0.f, 0.f, 2.f), -VEC_Z);
  std::optional<HitRecord> hit1 = unit_sphere.ray_intersection(ray1);
  HitRecord expected1 = HitRecord(Point(0.f, 0.f, 1.f), VEC_Z.to_normal(), Vec2d(0.f, 0.f), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(3.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit2 = unit_sphere.ray_intersection(ray2);
  HitRecord expected2 = HitRecord(Point(1.f, 0.f, 0.f), VEC_X.to_normal(), Vec2d(0.f, 0.5f), ray2, 2.f);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2.value().is_close(expected2));
}

// test hit from the inside
TEST(SphereTest, test_inner_hit) {
  Sphere unit_sphere = Sphere();

  Ray ray1 = Ray(Point(0.f, 0.f, 0.f), VEC_X);
  std::optional<HitRecord> hit1 = unit_sphere.ray_intersection(ray1);
  HitRecord expected1 = HitRecord(Point(1.f, 0.f, 0.f), -VEC_X.to_normal(), Vec2d(0.f, 0.5f), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));
}

// test hits on a translated sphere, ensuring that there are no intersections with the untranslated sphere
TEST(SphereTest, test_translation) {
  Sphere translated_sphere = Sphere(translation(Vec(10.f, 0.f, 0.f)));

  Ray ray1 = Ray(Point(10.f, 0.f, 2.f), -VEC_Z);
  std::optional<HitRecord> hit1 = translated_sphere.ray_intersection(ray1);
  HitRecord expected1 = HitRecord(Point(10.f, 0.f, 1.f), VEC_Z.to_normal(), Vec2d(), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(13.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit2 = translated_sphere.ray_intersection(ray2);
  HitRecord expected2 = HitRecord(Point(11.f, 0.f, 0.f), VEC_X.to_normal(), Vec2d(0.f, 0.5f), ray2, 2.f);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2.value().is_close(expected2));

  std::optional<HitRecord> hit3 = translated_sphere.ray_intersection(Ray(Point(0.f, 0.f, 2.f), -VEC_Z));
  EXPECT_FALSE(hit3);

  std::optional<HitRecord> hit4 = translated_sphere.ray_intersection(Ray(Point(-10.f, 0.f, 2.f), -VEC_Z));
  EXPECT_FALSE(hit4);
}

// test normals, which are non-trivial when a scaling is performed on the sphere
TEST(SphereTest, test_normals) {
  Sphere sphere1 = Sphere(scaling({2.f, 1.f, 1.f}));

  Ray ray1 = Ray(Point(1.f, 1.f, 0.f), Vec(-1.f, -1.f, 0.f));
  std::optional<HitRecord> hit1 = sphere1.ray_intersection(ray1);

  ASSERT_TRUE(hit1);
  // TODO consider changing the implementation of Normal::normalize(), perhaps it should return a Normal
  // ANSWER Isn't it already returning a Normal? it's a void method within the Normal class, so it should be
  Normal computed_normal = hit1->normal;
  Normal expected_normal = Normal(1.f, 4.f, 0.f);
  computed_normal.normalize();
  expected_normal.normalize();
  EXPECT_TRUE(computed_normal.is_close(expected_normal));
}

// test normal flipping
TEST(SphereTest, test_normal_flipping) {
  // this scaling flips the sphere about the z-x plane, so that in the standard sphere's reference frame the ray is
  // incoming from the left
  Sphere sphere1 = Sphere(scaling({1.f, -1.f, 1.f}));

  Ray ray1 = Ray(Point(0.f, 2.f, 0.f), -VEC_Y);
  std::optional<HitRecord> hit1 = sphere1.ray_intersection(ray1);

  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->normal.is_close(VEC_Y.to_normal()));
}

// test surface coordinates for non-trivial rays
TEST(SphereTest, test_surface_coordinates) {
  Sphere unit_sphere = Sphere();

  // The first four rays hit the unit sphere at the points P1, P2, P3, and P4.
  //
  //                     ^ y
  //                     | P2
  //               , - ~ * ~ - ,
  //           , '       |       ' ,
  //         ,           |           ,
  //        ,            |            ,
  //       ,             |             , P1
  //  -----*-------------+-------------*---------> x
  //    P3 ,             |             ,
  //        ,            |            ,
  //         ,           |           ,
  //           ,         |        , '
  //             ' - , _ * _ ,  '
  //                     | P4
  //
  //  P5 and P6 have same x and y coordinates as P1, but are displaced along z (ray5 in the positive direction, ray6 in
  //  the negative direction) so that the center of the sphere sees both of them at an angle pi/3 with respect to P1.

  Ray ray1 = Ray(Point(2.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit1 = unit_sphere.ray_intersection(ray1);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->surface_point.is_close(Vec2d(0.f, 0.5f)));

  Ray ray2 = Ray(Point(0.f, 2.f, 0.f), -VEC_Y);
  std::optional<HitRecord> hit2 = unit_sphere.ray_intersection(ray2);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2->surface_point.is_close(Vec2d(0.25f, 0.5f)));

  Ray ray3 = Ray(Point(-2.f, 0.f, 0.f), VEC_X);
  std::optional<HitRecord> hit3 = unit_sphere.ray_intersection(ray3);
  ASSERT_TRUE(hit3);
  EXPECT_TRUE(hit3->surface_point.is_close(Vec2d(0.5f, 0.5f)));

  Ray ray4 = Ray(Point(0.f, -2.f, 0.f), VEC_Y);
  std::optional<HitRecord> hit4 = unit_sphere.ray_intersection(ray4);
  ASSERT_TRUE(hit4);
  EXPECT_TRUE(hit4->surface_point.is_close(Vec2d(0.75f, 0.5f)));

  Ray ray5 = Ray(Point(2.f, 0.f, 0.5f), -VEC_X);
  std::optional<HitRecord> hit5 = unit_sphere.ray_intersection(ray5);
  ASSERT_TRUE(hit5);
  EXPECT_TRUE(hit5->surface_point.is_close(Vec2d(0.f, 1.f / 3.f)));

  Ray ray6 = Ray(Point(2.f, 0.f, -0.5f), -VEC_X);
  std::optional<HitRecord> hit6 = unit_sphere.ray_intersection(ray6);
  ASSERT_TRUE(hit6);
  EXPECT_TRUE(hit6->surface_point.is_close(Vec2d(0.f, 2.f / 3.f)));
}

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR PLANE-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// test intersections between default plane and orthogonal and parallel rays
TEST(PlaneTest, test_hit) {
  Plane default_plane = Plane();

  Ray ray1 = Ray(Point(0.f, 0.f, 1.f), -VEC_Z);
  std::optional<HitRecord> hit1 = default_plane.ray_intersection(ray1);
  HitRecord expected1 = HitRecord(Point(), VEC_Z.to_normal(), Vec2d(), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(0.f, 0.f, 1.f), VEC_Z);
  std::optional<HitRecord> hit2 = default_plane.ray_intersection(ray2);

  Ray ray3 = Ray(Point(0.f, 0.f, 1.f), VEC_X);
  std::optional<HitRecord> hit3 = default_plane.ray_intersection(ray3);

  Ray ray4 = Ray(Point(0.f, 0.f, 1.f), VEC_Y);
  std::optional<HitRecord> hit4 = default_plane.ray_intersection(ray4);

  EXPECT_FALSE(hit2);
  EXPECT_FALSE(hit3);
  EXPECT_FALSE(hit4);
}

// test intersections with rotated plane
TEST(PlaneTest, test_rotation) {
  Plane rotated_plane = Plane(rotation_y(std::numbers::pi / 2.f));

  Ray ray1 = Ray(Point(1.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit1 = rotated_plane.ray_intersection(ray1);
  HitRecord expected1 = HitRecord(Point(), VEC_X.to_normal(), Vec2d(), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(1.f, 0.f, 0.f), VEC_X);
  std::optional<HitRecord> hit2 = rotated_plane.ray_intersection(ray2);

  Ray ray3 = Ray(Point(1.f, 0.f, 0.f), VEC_Y);
  std::optional<HitRecord> hit3 = rotated_plane.ray_intersection(ray3);

  Ray ray4 = Ray(Point(1.f, 0.f, 0.f), VEC_Z);
  std::optional<HitRecord> hit4 = rotated_plane.ray_intersection(ray4);

  EXPECT_FALSE(hit2);
  EXPECT_FALSE(hit3);
  EXPECT_FALSE(hit4);
}

// test periodic (u, v) coordinates parametrization
TEST(PlaneTest, test_uv_coordinates) {
  Plane default_plane = Plane();

  Ray ray1 = Ray(Point(0.25f, 0.75f, 1.f), -VEC_Z);
  Ray ray2 = Ray(Point(4.25f, 7.75f, 1.f), -VEC_Z);

  std::optional<HitRecord> hit1 = default_plane.ray_intersection(ray1);
  std::optional<HitRecord> hit2 = default_plane.ray_intersection(ray2);

  ASSERT_TRUE(hit1);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit1.value().surface_point.is_close(Vec2d(0.25f, 0.75f)));
  EXPECT_TRUE(hit2.value().surface_point.is_close(Vec2d(0.25f, 0.75f)));
}

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR WORLD-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// test ray_intersection method
TEST(WorldTest, test_ray_intersection) {
  World world = World();

  auto sphere1 = std::make_shared<Sphere>(translation(Vec(2.f, 0.f, 0.f)));
  auto sphere2 = std::make_shared<Sphere>(translation(Vec(10.f, 0.f, 0.f)));

  world.add_object(sphere1);
  world.add_object(sphere2);

  Ray ray1 = Ray(Point(0.f, 0.f, 0.f), VEC_X);
  std::optional<HitRecord> hit1 = world.ray_intersection(ray1);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->world_point.is_close(Point(1.f, 0.f, 0.f)));

  Ray ray2 = Ray(Point(10.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit2 = world.ray_intersection(ray2);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2->world_point.is_close(Point(9.f, 0.f, 0.f)));
}
