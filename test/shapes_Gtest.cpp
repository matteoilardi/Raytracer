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
#include "renderers.hpp"
#include "shapes.hpp"
#include <gtest/gtest.h>

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR SPHERE-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// test two hits from outside the sphere
TEST(SphereTest, test_outer_hit) {
  auto unit_sphere = std::make_shared<Sphere>();

  Ray ray1 = Ray(Point(0.f, 0.f, 2.f), -VEC_Z);
  std::optional<HitRecord> hit1 = unit_sphere->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(unit_sphere, Point(0.f, 0.f, 1.f), VEC_Z.to_normal(), Vec2d(0.f, 0.f), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(3.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit2 = unit_sphere->ray_intersection(ray2);
  HitRecord expected2 = HitRecord(unit_sphere, Point(1.f, 0.f, 0.f), VEC_X.to_normal(), Vec2d(0.f, 0.5f), ray2, 2.f);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2.value().is_close(expected2));
}

// test hit from the inside
TEST(SphereTest, test_inner_hit) {
  auto unit_sphere = std::make_shared<Sphere>();

  Ray ray1 = Ray(Point(0.f, 0.f, 0.f), VEC_X);
  std::optional<HitRecord> hit1 = unit_sphere->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(unit_sphere, Point(1.f, 0.f, 0.f), -VEC_X.to_normal(), Vec2d(0.f, 0.5f), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));
}

// test hits on a translated sphere, ensuring that there are no intersections with the untranslated sphere
TEST(SphereTest, test_translation) {
  auto translated_sphere = std::make_shared<Sphere>(translation(Vec(10.f, 0.f, 0.f)));

  Ray ray1 = Ray(Point(10.f, 0.f, 2.f), -VEC_Z);
  std::optional<HitRecord> hit1 = translated_sphere->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(translated_sphere, Point(10.f, 0.f, 1.f), VEC_Z.to_normal(), Vec2d(), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(13.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit2 = translated_sphere->ray_intersection(ray2);
  HitRecord expected2 = HitRecord(translated_sphere, Point(11.f, 0.f, 0.f), VEC_X.to_normal(), Vec2d(0.f, 0.5f), ray2, 2.f);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2.value().is_close(expected2));

  std::optional<HitRecord> hit3 = translated_sphere->ray_intersection(Ray(Point(0.f, 0.f, 2.f), -VEC_Z));
  EXPECT_FALSE(hit3);

  std::optional<HitRecord> hit4 = translated_sphere->ray_intersection(Ray(Point(-10.f, 0.f, 2.f), -VEC_Z));
  EXPECT_FALSE(hit4);
}

// test normals, which are non-trivial when a scaling is performed on the sphere
// NOTE in this and the following tests it is absoluteley necessary to create the shapes with shared pointers, otherwise
// if Sphere is on the stack an exception is raised.
TEST(SphereTest, test_normals) {
  auto sphere1 = std::make_shared<Sphere>(scaling({2.f, 1.f, 1.f}));

  Ray ray1 = Ray(Point(1.f, 1.f, 0.f), Vec(-1.f, -1.f, 0.f));
  std::optional<HitRecord> hit1 = sphere1->ray_intersection(ray1);

  ASSERT_TRUE(hit1);
  Normal computed_normal = (hit1->normal).normalize();
  Normal expected_normal = (Normal(1.f, 4.f, 0.f)).normalize();
  EXPECT_TRUE(computed_normal.is_close(expected_normal));
}

// test normal flipping
TEST(SphereTest, test_normal_flipping) {
  // this scaling flips the sphere about the z-x plane, so that in the standard sphere's reference frame the ray is
  // incoming from the left
  auto sphere1 = std::make_shared<Sphere>(scaling({1.f, -1.f, 1.f}));

  Ray ray1 = Ray(Point(0.f, 2.f, 0.f), -VEC_Y);
  std::optional<HitRecord> hit1 = sphere1->ray_intersection(ray1);

  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->normal.is_close(VEC_Y.to_normal()));
}

// test surface coordinates for non-trivial rays
TEST(SphereTest, test_surface_coordinates) {
  auto unit_sphere = std::make_shared<Sphere>();

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
  std::optional<HitRecord> hit1 = unit_sphere->ray_intersection(ray1);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->surface_point.is_close(Vec2d(0.f, 0.5f)));

  Ray ray2 = Ray(Point(0.f, 2.f, 0.f), -VEC_Y);
  std::optional<HitRecord> hit2 = unit_sphere->ray_intersection(ray2);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2->surface_point.is_close(Vec2d(0.25f, 0.5f)));

  Ray ray3 = Ray(Point(-2.f, 0.f, 0.f), VEC_X);
  std::optional<HitRecord> hit3 = unit_sphere->ray_intersection(ray3);
  ASSERT_TRUE(hit3);
  EXPECT_TRUE(hit3->surface_point.is_close(Vec2d(0.5f, 0.5f)));

  Ray ray4 = Ray(Point(0.f, -2.f, 0.f), VEC_Y);
  std::optional<HitRecord> hit4 = unit_sphere->ray_intersection(ray4);
  ASSERT_TRUE(hit4);
  EXPECT_TRUE(hit4->surface_point.is_close(Vec2d(0.75f, 0.5f)));

  Ray ray5 = Ray(Point(2.f, 0.f, 0.5f), -VEC_X);
  std::optional<HitRecord> hit5 = unit_sphere->ray_intersection(ray5);
  ASSERT_TRUE(hit5);
  EXPECT_TRUE(hit5->surface_point.is_close(Vec2d(0.f, 1.f / 3.f)));

  Ray ray6 = Ray(Point(2.f, 0.f, -0.5f), -VEC_X);
  std::optional<HitRecord> hit6 = unit_sphere->ray_intersection(ray6);
  ASSERT_TRUE(hit6);
  EXPECT_TRUE(hit6->surface_point.is_close(Vec2d(0.f, 2.f / 3.f)));
}

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR PLANE-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// test intersections between default plane and orthogonal and parallel rays
TEST(PlaneTest, test_hit) {
  auto default_plane = std::make_shared<Plane>();

  Ray ray1 = Ray(Point(0.f, 0.f, 1.f), -VEC_Z);
  std::optional<HitRecord> hit1 = default_plane->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(default_plane, Point(), VEC_Z.to_normal(), Vec2d(), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(0.f, 0.f, 1.f), VEC_Z);
  std::optional<HitRecord> hit2 = default_plane->ray_intersection(ray2);

  Ray ray3 = Ray(Point(0.f, 0.f, 1.f), VEC_X);
  std::optional<HitRecord> hit3 = default_plane->ray_intersection(ray3);

  Ray ray4 = Ray(Point(0.f, 0.f, 1.f), VEC_Y);
  std::optional<HitRecord> hit4 = default_plane->ray_intersection(ray4);

  EXPECT_FALSE(hit2);
  EXPECT_FALSE(hit3);
  EXPECT_FALSE(hit4);
}

// test intersections with rotated plane
TEST(PlaneTest, test_rotation) {
  auto rotated_plane = std::make_shared<Plane>(rotation_y(std::numbers::pi / 2.f));

  Ray ray1 = Ray(Point(1.f, 0.f, 0.f), -VEC_X);
  std::optional<HitRecord> hit1 = rotated_plane->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(rotated_plane, Point(), VEC_X.to_normal(), Vec2d(), ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2 = Ray(Point(1.f, 0.f, 0.f), VEC_X);
  std::optional<HitRecord> hit2 = rotated_plane->ray_intersection(ray2);

  Ray ray3 = Ray(Point(1.f, 0.f, 0.f), VEC_Y);
  std::optional<HitRecord> hit3 = rotated_plane->ray_intersection(ray3);

  Ray ray4 = Ray(Point(1.f, 0.f, 0.f), VEC_Z);
  std::optional<HitRecord> hit4 = rotated_plane->ray_intersection(ray4);

  EXPECT_FALSE(hit2);
  EXPECT_FALSE(hit3);
  EXPECT_FALSE(hit4);
}

// test periodic surface coordinates parametrization
TEST(PlaneTest, test_surface_coordinates) {
  auto default_plane = std::make_shared<Plane>();

  Ray ray1 = Ray(Point(0.25f, 0.75f, 1.f), -VEC_Z);
  Ray ray2 = Ray(Point(4.25f, 7.75f, 1.f), -VEC_Z);

  std::optional<HitRecord> hit1 = default_plane->ray_intersection(ray1);
  std::optional<HitRecord> hit2 = default_plane->ray_intersection(ray2);

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

TEST(WorldTest, test_offset_if_visible) {
  World world = World();

  auto sphere1 = std::make_shared<Sphere>(translation(2.f*VEC_X));
  auto sphere2 = std::make_shared<Sphere>(translation(8.f*VEC_X));
  world.add_object(sphere1);
  world.add_object(sphere2);

  EXPECT_FALSE(world.offset_if_visible(Point(0.f, 0.f, 0.f), Point(10.f, 0.f, 0.f), Normal(-1.f, 0.f, 0.f)));
  EXPECT_FALSE(world.offset_if_visible(Point(0.f, 0.f, 0.f), Point(5.f, 0.f, 0.f), Normal(-1.f, 0.f, 0.f)));

  std::optional<Vec> v1 = world.offset_if_visible(Point(4.f, 0.f, 0.f), Point(5.f, 0.f, 0.f), Normal(-1.f, 0.f, 0.f));
  EXPECT_TRUE(v1.has_value());
  EXPECT_TRUE(v1.value().is_close(VEC_X));

  std::optional<Vec> v2 = world.offset_if_visible(Point(0.f, 0.f, 0.f), Point(0.5f, 0.f, 0.f), Normal(-1.f, 0.f, 0.f));
  EXPECT_TRUE(v2.has_value());
  EXPECT_TRUE(v2.value().is_close(0.5f*VEC_X));

  std::optional<Vec> v3 = world.offset_if_visible(Point(0.f, 0.f, 0.f), Point(0.f, 10.f, 0.f), Normal(0.f, -1.f, 0.f));
  EXPECT_TRUE(v3.has_value());
  EXPECT_TRUE(v3.value().is_close(10.f*VEC_Y));

  std::optional<Vec> v4 = world.offset_if_visible(Point(0.f, 0.f, 0.f), Point(0.f, 0.f, 10.f), Normal(0.f, 0.f, -1.f));
  EXPECT_TRUE(v4.has_value());
  EXPECT_TRUE(v4.value().is_close(10.f*VEC_Z));
}
