#include <gtest/gtest.h>

#include <BoundingBox.h>

TEST(BoundingBoxTest, GetBottomFaceVertex) {
    BoundingBox box(Vector3f(0,0,0), Vector3f(3, 4, 5));
    Vector3f v4(0,0,5);
    Vector3f v = box.GetVertex(EFace::BOTTOM, 4);
    ASSERT_EQ(v4, v);
}

TEST(BoundingBoxTest, GetTopFaceVertex) {
    BoundingBox box(Vector3f(0,0,0), Vector3f(3, 4, 5));
    Vector3f v5(0,4,0);
    Vector3f v = box.GetVertex(EFace::TOP, 5);
    ASSERT_EQ(v5, v);
}

TEST(BoundingBoxTest, GetFrontFaceVertex) {
    BoundingBox box(Vector3f(0,0,0), Vector3f(3, 4, 5));
    Vector3f v0(0,0,5);
    Vector3f v = box.GetVertex(EFace::FRONT, 0);
    ASSERT_EQ(v0, v);
}

TEST(BoundingBoxTest, GetBackFaceVertex) {
    BoundingBox box(Vector3f(0,0,0), Vector3f(3, 4, 5));
    Vector3f v2(3,4,0);
    Vector3f v = box.GetVertex(EFace::BACK, 2);
    ASSERT_EQ(v2, v);
}

TEST(BoundingBoxTest, GetRightFaceVertex) {
    BoundingBox box(Vector3f(0,0,0), Vector3f(3, 4, 5));
    Vector3f v0(3,0,0);
    Vector3f v = box.GetVertex(EFace::RIGHT, 0);
    ASSERT_EQ(v0, v);
}

TEST(BoundingBoxTest, GetTopLeftVertex) {
    BoundingBox box(Vector3f(0,0,0), Vector3f(3, 4, 5));
    Vector3f v5(0,0,0);
    Vector3f v = box.GetVertex(EFace::LEFT, 5);
    ASSERT_EQ(v5, v);
}