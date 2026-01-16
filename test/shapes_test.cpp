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

class SphereTest : public ::testing::Test {
protected:
  void SetUp() override { default_material = make_neutral_material(); }

  Material default_material;
};

// Test two hits from outside the sphere
TEST_F(SphereTest, test_outer_hit) {
  auto unit_sphere = std::make_unique<Sphere>(Transformation{}, default_material);

  Ray ray1{Point{0.f, 0.f, 2.f}, -VEC_Z};
  std::optional<HitRecord> hit1 = unit_sphere->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(unit_sphere.get(), Point{0.f, 0.f, 1.f}, VEC_Z.to_normal(), Vec2d{}, ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2{Point{3.f, 0.f, 0.f}, -VEC_X};
  std::optional<HitRecord> hit2 = unit_sphere->ray_intersection(ray2);
  HitRecord expected2 = HitRecord(unit_sphere.get(), Point{1.f, 0.f, 0.f}, VEC_X.to_normal(), Vec2d{0.f, 0.5f}, ray2, 2.f);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2.value().is_close(expected2));
}

// Test hit from the inside
TEST_F(SphereTest, test_inner_hit) {
  auto unit_sphere = std::make_unique<Sphere>(Transformation{}, default_material);

  Ray ray1(Point{}, VEC_X);
  std::optional<HitRecord> hit1 = unit_sphere->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(unit_sphere.get(), Point{1.f, 0.f, 0.f}, -VEC_X.to_normal(), Vec2d{0.f, 0.5f}, ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));
}

// Test hits on a translated sphere, ensuring that there are no intersections with the untranslated sphere
TEST_F(SphereTest, test_translation) {
  auto translated_sphere = std::make_unique<Sphere>(translation{Vec{10.f, 0.f, 0.f}}, default_material);

  Ray ray1{Point{10.f, 0.f, 2.f}, -VEC_Z};
  std::optional<HitRecord> hit1 = translated_sphere->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(translated_sphere.get(), Point{10.f, 0.f, 1.f}, VEC_Z.to_normal(), Vec2d{}, ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2(Point{13.f, 0.f, 0.f}, -VEC_X);
  std::optional<HitRecord> hit2 = translated_sphere->ray_intersection(ray2);
  HitRecord expected2 = HitRecord(translated_sphere.get(), Point{11.f, 0.f, 0.f}, VEC_X.to_normal(), Vec2d{0.f, 0.5f}, ray2, 2.f);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2.value().is_close(expected2));

  std::optional<HitRecord> hit3 = translated_sphere->ray_intersection(Ray{Point{0.f, 0.f, 2.f}, -VEC_Z});
  EXPECT_FALSE(hit3);

  std::optional<HitRecord> hit4 = translated_sphere->ray_intersection(Ray{Point{-10.f, 0.f, 2.f}, -VEC_Z});
  EXPECT_FALSE(hit4);
}

// Test normals, which are non-trivial when a scaling is performed on the sphere
TEST_F(SphereTest, test_normals) {
  auto sphere1 = std::make_unique<Sphere>(scaling{{2.f, 1.f, 1.f}}, default_material);

  Ray ray1{Point{1.f, 1.f, 0.f}, Vec{-1.f, -1.f, 0.f}};
  std::optional<HitRecord> hit1 = sphere1->ray_intersection(ray1);

  ASSERT_TRUE(hit1);
  Normal computed_normal = hit1->normal.normalized();
  Normal expected_normal = Normal{1.f, 4.f, 0.f}.normalized();
  EXPECT_TRUE(computed_normal.is_close(expected_normal));
}

// Test normal flipping
TEST_F(SphereTest, test_normal_flipping) {
  // This scaling flips the sphere about the z-x plane, so that in the standard sphere's reference frame the ray is
  // incoming from the left
  auto sphere1 = std::make_unique<Sphere>(scaling{{1.f, -1.f, 1.f}}, default_material);

  Ray ray1{Point{0.f, 2.f, 0.f}, -VEC_Y};
  std::optional<HitRecord> hit1 = sphere1->ray_intersection(ray1);

  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->normal.is_close(VEC_Y.to_normal()));
}

// Test surface coordinates for non-trivial rays
TEST_F(SphereTest, test_surface_coordinates) {
  auto unit_sphere = std::make_unique<Sphere>(Transformation{}, default_material);

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

  Ray ray1{Point{2.f, 0.f, 0.f}, -VEC_X};
  std::optional<HitRecord> hit1 = unit_sphere->ray_intersection(ray1);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->surface_point.is_close(Vec2d{0.f, 0.5f}));

  Ray ray2{Point{0.f, 2.f, 0.f}, -VEC_Y};
  std::optional<HitRecord> hit2 = unit_sphere->ray_intersection(ray2);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2->surface_point.is_close(Vec2d{0.25f, 0.5f}));

  Ray ray3{Point{-2.f, 0.f, 0.f}, VEC_X};
  std::optional<HitRecord> hit3 = unit_sphere->ray_intersection(ray3);
  ASSERT_TRUE(hit3);
  EXPECT_TRUE(hit3->surface_point.is_close(Vec2d{0.5f, 0.5f}));

  Ray ray4{Point{0.f, -2.f, 0.f}, VEC_Y};
  std::optional<HitRecord> hit4 = unit_sphere->ray_intersection(ray4);
  ASSERT_TRUE(hit4);
  EXPECT_TRUE(hit4->surface_point.is_close(Vec2d{0.75f, 0.5f}));

  Ray ray5{Point{2.f, 0.f, 0.5f}, -VEC_X};
  std::optional<HitRecord> hit5 = unit_sphere->ray_intersection(ray5);
  ASSERT_TRUE(hit5);
  EXPECT_TRUE(hit5->surface_point.is_close(Vec2d{0.f, 1.f / 3.f}));

  Ray ray6{Point{2.f, 0.f, -0.5f}, -VEC_X};
  std::optional<HitRecord> hit6 = unit_sphere->ray_intersection(ray6);
  ASSERT_TRUE(hit6);
  EXPECT_TRUE(hit6->surface_point.is_close(Vec2d{0.f, 2.f / 3.f}));
}

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR PLANE-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

class PlaneTest : public ::testing::Test {
protected:
  void SetUp() override { default_material = make_neutral_material(); }

  Material default_material;
};

// Test intersections between default plane and orthogonal and parallel rays
TEST_F(PlaneTest, test_hit) {
  auto default_plane = std::make_unique<Plane>(Transformation{}, default_material);

  Ray ray1{Point{0.f, 0.f, 1.f}, -VEC_Z};
  std::optional<HitRecord> hit1 = default_plane->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(default_plane.get(), Point{}, VEC_Z.to_normal(), Vec2d{}, ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2{Point{0.f, 0.f, 1.f}, VEC_Z};
  std::optional<HitRecord> hit2 = default_plane->ray_intersection(ray2);

  Ray ray3{Point{0.f, 0.f, 1.f}, VEC_X};
  std::optional<HitRecord> hit3 = default_plane->ray_intersection(ray3);

  Ray ray4{Point{0.f, 0.f, 1.f}, VEC_Y};
  std::optional<HitRecord> hit4 = default_plane->ray_intersection(ray4);

  EXPECT_FALSE(hit2);
  EXPECT_FALSE(hit3);
  EXPECT_FALSE(hit4);
}

// Test intersections with rotated plane
TEST_F(PlaneTest, test_rotation) {
  auto rotated_plane = std::make_unique<Plane>(rotation_y{std::numbers::pi_v<float> / 2.f}, default_material);

  Ray ray1{Point{1.f, 0.f, 0.f}, -VEC_X};
  std::optional<HitRecord> hit1 = rotated_plane->ray_intersection(ray1);
  HitRecord expected1 = HitRecord(rotated_plane.get(), Point{}, VEC_X.to_normal(), Vec2d{}, ray1, 1.f);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1.value().is_close(expected1));

  Ray ray2{Point{1.f, 0.f, 0.f}, VEC_X};
  std::optional<HitRecord> hit2 = rotated_plane->ray_intersection(ray2);

  Ray ray3{Point{1.f, 0.f, 0.f}, VEC_Y};
  std::optional<HitRecord> hit3 = rotated_plane->ray_intersection(ray3);

  Ray ray4{Point{1.f, 0.f, 0.f}, VEC_Z};
  std::optional<HitRecord> hit4 = rotated_plane->ray_intersection(ray4);

  EXPECT_FALSE(hit2);
  EXPECT_FALSE(hit3);
  EXPECT_FALSE(hit4);
}

// Test periodic surface coordinates parametrization
TEST_F(PlaneTest, test_surface_coordinates) {
  auto default_plane = std::make_unique<Plane>(Transformation{}, default_material);

  Ray ray1{Point{0.25f, 0.75f, 1.f}, -VEC_Z};
  Ray ray2{Point{4.25f, 7.75f, 1.f}, -VEC_Z};

  std::optional<HitRecord> hit1 = default_plane->ray_intersection(ray1);
  std::optional<HitRecord> hit2 = default_plane->ray_intersection(ray2);

  ASSERT_TRUE(hit1);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit1.value().surface_point.is_close(Vec2d{0.25f, 0.75f}));
  EXPECT_TRUE(hit2.value().surface_point.is_close(Vec2d{0.25f, 0.75f}));
}

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR CSGOBJECT -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

class CSGSphereTest : public ::testing::Test {
protected:
  std::unique_ptr<Sphere> sphere1;
  std::unique_ptr<Sphere> sphere2;
  std::unique_ptr<CSGObject> csg;
  Ray ray1;
  Ray ray2;
  Ray ray3;
  Material default_material;

  void SetUp() override {
    sphere1 = std::make_unique<Sphere>(Transformation{}, default_material);
    sphere2 = std::make_unique<Sphere>(translation{VEC_X}, default_material);
    csg = std::make_unique<CSGObject>(std::move(sphere1), std::move(sphere2), CSGObject::Operation::UNION);

    ray1 = Ray{Point{-2.f, 0.f, 0.f}, VEC_X};
    ray2 = Ray{Point{0.f, 0.f, -2.f}, VEC_Z};
    ray3 = Ray{Point{1.f, 0.f, -2.f}, VEC_Z};

    default_material = make_neutral_material();
  }
};

TEST_F(CSGSphereTest, test_union) {
  csg->operation = CSGObject::Operation::UNION;

  auto hits1 = csg->all_ray_intersections(ray1);
  ASSERT_EQ(hits1.size(), 4);
  EXPECT_TRUE(are_close(hits1[0].t, 1.f));
  EXPECT_TRUE(are_close(hits1[1].t, 2.f));
  EXPECT_TRUE(are_close(hits1[2].t, 3.f));
  EXPECT_TRUE(are_close(hits1[3].t, 4.f));

  auto hits2 = csg->all_ray_intersections(ray2);
  ASSERT_EQ(hits2.size(), 2);
  EXPECT_TRUE(are_close(hits2[0].t, 1.f));
  EXPECT_TRUE(are_close(hits2[1].t, 3.f));

  auto hits3 = csg->all_ray_intersections(ray3);
  ASSERT_EQ(hits3.size(), 2);
  EXPECT_TRUE(are_close(hits3[0].t, 1.f));
  EXPECT_TRUE(are_close(hits3[1].t, 3.f));
}

TEST_F(CSGSphereTest, test_intersection) {
  csg->operation = CSGObject::Operation::INTERSECTION;

  auto hits1 = csg->all_ray_intersections(ray1);
  ASSERT_EQ(hits1.size(), 2);
  EXPECT_TRUE(are_close(hits1[0].t, 2.f));
  EXPECT_TRUE(are_close(hits1[1].t, 3.f));

  auto hits2 = csg->all_ray_intersections(ray2);
  ASSERT_EQ(hits2.size(), 0);

  auto hits3 = csg->all_ray_intersections(ray3);
  ASSERT_EQ(hits3.size(), 0);
}

TEST_F(CSGSphereTest, test_difference) {
  csg->operation = CSGObject::Operation::DIFFERENCE;

  auto hits1 = csg->all_ray_intersections(ray1);
  ASSERT_EQ(hits1.size(), 2);
  EXPECT_TRUE(are_close(hits1[0].t, 1.f));
  EXPECT_TRUE(are_close(hits1[1].t, 2.f));

  auto hits2 = csg->all_ray_intersections(ray2);
  ASSERT_EQ(hits2.size(), 2);
  EXPECT_TRUE(are_close(hits2[0].t, 1.f));
  EXPECT_TRUE(are_close(hits2[1].t, 3.f));

  auto hits3 = csg->all_ray_intersections(ray3);
  ASSERT_EQ(hits3.size(), 0);
}

TEST_F(CSGSphereTest, test_fusion) {
  csg->operation = CSGObject::Operation::FUSION;

  auto hits1 = csg->all_ray_intersections(ray1);
  ASSERT_EQ(hits1.size(), 2);
  EXPECT_TRUE(are_close(hits1[0].t, 1.f));
  EXPECT_TRUE(are_close(hits1[1].t, 4.f));

  auto hits2 = csg->all_ray_intersections(ray2);
  ASSERT_EQ(hits2.size(), 2);
  EXPECT_TRUE(are_close(hits2[0].t, 1.f));
  EXPECT_TRUE(are_close(hits2[1].t, 3.f));

  auto hits3 = csg->all_ray_intersections(ray3);
  ASSERT_EQ(hits3.size(), 2);
  EXPECT_TRUE(are_close(hits3[0].t, 1.f));
  EXPECT_TRUE(are_close(hits3[1].t, 3.f));
}

TEST(CSGTest, test_triple_csg) {
  auto sphere1 = std::make_unique<Sphere>(Transformation{}, make_neutral_material());
  auto sphere2 = std::make_unique<Sphere>(translation{VEC_X}, make_neutral_material());
  auto plane = std::make_unique<Plane>(translation{-0.5f * VEC_Z}, make_neutral_material());

  auto sphere_intersection =
      std::make_unique<CSGObject>(std::move(sphere1), std::move(sphere2), CSGObject::Operation::INTERSECTION);
  auto spearhead =
      std::make_unique<CSGObject>(std::move(sphere_intersection), std::move(plane), CSGObject::Operation::DIFFERENCE);

  Ray ray{Point{-2.f, 0.f, 0.f}, VEC_X};
  auto hits1 = spearhead->all_ray_intersections(ray);
  ASSERT_EQ(hits1.size(), 2);
  EXPECT_TRUE(are_close(hits1[0].t, 2.f));
  EXPECT_TRUE(are_close(hits1[1].t, 3.f));

  Ray ray2{Point{0.f, 0.f, -2.f}, VEC_Z};
  auto hits2 = spearhead->all_ray_intersections(ray2);
  ASSERT_EQ(hits2.size(), 0);

  Ray ray3{Point{1.f, 0.f, -2.f}, VEC_Z};
  auto hits3 = spearhead->all_ray_intersections(ray3);
  ASSERT_EQ(hits3.size(), 0);

  Ray ray4{Point{0.5f, 0.f, 2.f}, -VEC_Z};
  auto hits4 = spearhead->all_ray_intersections(ray4);
  ASSERT_EQ(hits4.size(), 1);
  EXPECT_TRUE(are_close(hits4[0].t, 2.5f));
  // Note that mathematically this is wrong: there is another intersection above the detected one at t = 2 - sqrt(3)/2,
  // where the spheres intersect. However, the fact that the code doesn't detect it comes to no suprise: it is a
  // consequence of the logic of hit_on_1_is_valid() (and its tween method) and of how is_point_inside() is implemented
  // in general. In fact, is_point_inside() excludes the surface without allowing for any tolerance (r < 1 for the sphere).
  // If we allowed for some small tollerance here, we would likely get duplicate hits... which we could remove by scanning
  // the array once at the end. But even if we did so, the change would break the case of Operation::FUSION and we would
  // end up with a problem worse than the original one. Another solution would be keeping is_point_inside() as is and
  // define a new, very similar method allowing points to be on the surface with some error tolerance. We could then call
  // one or the other in different circumstances.
  // However, we think that the solution to the problem doesn't justify this added complexity.
}

TEST(CSGTest, test_csg_transformation) {
  auto sphere = std::make_unique<Sphere>(Transformation{}, make_neutral_material());
  auto plane = std::make_unique<Plane>(scaling{{1.f, 1.f, -1.f}}, make_neutral_material());
  auto hemisphere = std::make_unique<CSGObject>(std::move(sphere), std::move(plane), CSGObject::Operation::INTERSECTION,
                                                translation{2.f * VEC_X});

  Ray ray1{Point{0.f, 0.f, 2.f}, -VEC_Z};
  auto hits1 = hemisphere->all_ray_intersections(ray1);
  ASSERT_EQ(hits1.size(), 0);

  Ray ray2{Point{2.f, 0.f, 2.f}, -VEC_Z};
  auto hits2 = hemisphere->all_ray_intersections(ray2);
  ASSERT_EQ(hits2.size(), 2);
  EXPECT_TRUE(are_close(hits2[0].t, 1.f));
  EXPECT_TRUE(are_close(hits2[1].t, 2.f));

  ASSERT_FALSE(hemisphere->is_point_inside(Point{0.f, 0.f, 0.5f}));
  ASSERT_TRUE(hemisphere->is_point_inside(Point{2.f, 0.f, 0.5f}));
  ASSERT_FALSE(hemisphere->is_point_inside(Point{2.f, 0.f, -0.5f}));
}

// ------------------------------------------------------------------------------------------------------------
// -------------TESTS FOR WORLD-------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

class WorldTest : public ::testing::Test {
protected:
  void SetUp() override { default_material = make_neutral_material(); }

  Material default_material;
};

// Test ray_intersection method
TEST_F(WorldTest, test_ray_intersection) {
  World world{};

  auto sphere1 = std::make_unique<Sphere>(translation{Vec{2.f, 0.f, 0.f}}, default_material);
  auto sphere2 = std::make_unique<Sphere>(translation{Vec{10.f, 0.f, 0.f}}, default_material);

  world.add_object(std::move(sphere1));
  world.add_object(std::move(sphere2));

  Ray ray1{Point{}, VEC_X};
  std::optional<HitRecord> hit1 = world.ray_intersection(ray1);
  ASSERT_TRUE(hit1);
  EXPECT_TRUE(hit1->world_point.is_close(Point{1.f, 0.f, 0.f}));

  Ray ray2{Point{10.f, 0.f, 0.f}, -VEC_X};
  std::optional<HitRecord> hit2 = world.ray_intersection(ray2);
  ASSERT_TRUE(hit2);
  EXPECT_TRUE(hit2->world_point.is_close(Point{9.f, 0.f, 0.f}));
}

TEST_F(WorldTest, test_offset_if_visible) {
  World world{};

  auto sphere1 = std::make_unique<Sphere>(translation{2.f * VEC_X}, default_material);
  auto sphere2 = std::make_unique<Sphere>(translation{8.f * VEC_X}, default_material);
  world.add_object(std::move(sphere1));
  world.add_object(std::move(sphere2));

  EXPECT_FALSE(world.offset_if_visible(Point{}, Point{10.f, 0.f, 0.f}, Normal{-1.f, 0.f, 0.f}));
  EXPECT_FALSE(world.offset_if_visible(Point{}, Point{5.f, 0.f, 0.f}, Normal{-1.f, 0.f, 0.f}));

  std::optional<Vec> v1 = world.offset_if_visible(Point{4.f, 0.f, 0.f}, Point{5.f, 0.f, 0.f}, Normal{-1.f, 0.f, 0.f});
  EXPECT_TRUE(v1.has_value());
  EXPECT_TRUE(v1.value().is_close(VEC_X));

  std::optional<Vec> v2 = world.offset_if_visible(Point{}, Point{0.5f, 0.f, 0.f}, Normal{-1.f, 0.f, 0.f});
  EXPECT_TRUE(v2.has_value());
  EXPECT_TRUE(v2.value().is_close(0.5f * VEC_X));

  std::optional<Vec> v3 = world.offset_if_visible(Point{}, Point{0.f, 10.f, 0.f}, Normal{0.f, -1.f, 0.f});
  EXPECT_TRUE(v3.has_value());
  EXPECT_TRUE(v3.value().is_close(10.f * VEC_Y));

  std::optional<Vec> v4 = world.offset_if_visible(Point{0.f, 0.f, 0.f}, Point{0.f, 0.f, 10.f}, Normal{0.f, 0.f, -1.f});
  EXPECT_TRUE(v4.has_value());
  EXPECT_TRUE(v4.value().is_close(10.f * VEC_Z));
}
