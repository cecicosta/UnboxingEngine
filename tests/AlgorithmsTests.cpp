#include <gtest/gtest.h>

#include <UVector.h>
#include <BoundingBox.h>

bool CheckBoxRayIntersection(const BoundingBox& boundingBox, const Vector3f& raySrc, const Vector3f& rayDir) {
    //Derivative from H >= p1, H >= p2, H between p1 and p2 from the box and H = O + Dt, from the ray, with H representing any point of intersection
    Vector3f upper = (boundingBox.getFirst() + boundingBox.getSecond()) - 2*raySrc;
    upper = Vector3f(upper.x/rayDir.x, upper.y/rayDir.y, upper.z/rayDir.z);
    return upper.x > 0 && upper.y > 0 && upper.z > 0;
}

TEST(CubeRayIntersectionTest, DetectIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(2, 4, 2);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(1, 1, -1);

    ASSERT_TRUE(CheckBoxRayIntersection(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}

TEST(CubeRayIntersectionTest, NoIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(2, 4, 2);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(1, 1, 4);

    ASSERT_FALSE(CheckBoxRayIntersection(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}

TEST(CubeRayIntersectionTest, CloseCallNoIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(1, 1, 1);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(-0.1, 1, -4);

    ASSERT_FALSE(CheckBoxRayIntersection(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}

TEST(CubeRayIntersectionTest, TangentIntersection) {
    //Box first point is in the origin
    Vector3f first(0, 0, 0);
    Vector3f second(1, 1, 1);
    Vector3f raySrc(0, -1, 3);
    Vector3f rayEnd(0, 1, -4);

    ASSERT_TRUE(CheckBoxRayIntersection(BoundingBox(first, second), raySrc, rayEnd - raySrc));
}