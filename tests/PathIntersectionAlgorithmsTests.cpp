#include <gtest/gtest.h>

#include <algorithms/CollisionAlgorithms.h>
#include <UVector.h>

#include <vector>

namespace {

constexpr float kTolerance = 0.0001f;

std::vector<Vector2f> makeTriangle2D() {
    return {
        Vector2f(0.0f, 0.0f),
        Vector2f(2.0f, 0.0f),
        Vector2f(0.0f, 2.0f),
    };
}

std::vector<Vector3f> makeTriangle3D() {
    return {
        Vector3f(0.0f, 0.0f, 0.0f),
        Vector3f(2.0f, 0.0f, 0.0f),
        Vector3f(0.0f, 2.0f, 0.0f),
    };
}

} // namespace

TEST(PathIntersectionAlgorithms, SegmentCrossesTriangleBoundaryIn2D) {
    const auto triangle = makeTriangle2D();
    const auto result = unboxing_engine::algorithms::checkPathIntersectionWithSegment<float, 2>(
        triangle,
        Vector2f(-0.5f, 0.5f),
        Vector2f(1.5f, 0.5f));

    ASSERT_EQ(result.vertices.size(), 2);
    EXPECT_EQ(result.vertices[0], triangle[2]);
    EXPECT_EQ(result.vertices[1], triangle[0]);
    EXPECT_NEAR(result.intersection.x, 0.0f, kTolerance);
    EXPECT_NEAR(result.intersection.y, 0.5f, kTolerance);
}

TEST(PathIntersectionAlgorithms, SegmentMissesTriangleBoundaryIn2D) {
    const auto triangle = makeTriangle2D();
    const auto result = unboxing_engine::algorithms::checkPathIntersectionWithSegment<float, 2>(
        triangle,
        Vector2f(-0.5f, 2.5f),
        Vector2f(1.5f, 2.5f));

    EXPECT_TRUE(result.vertices.empty());
}

TEST(PathIntersectionAlgorithms, CoplanarSegmentCrossesTriangleBoundaryIn3D) {
    const auto triangle = makeTriangle3D();
    const auto result = unboxing_engine::algorithms::checkPathIntersectionWithSegment<float, 3>(
        triangle,
        Vector3f(-0.5f, 0.5f, 0.0f),
        Vector3f(1.5f, 0.5f, 0.0f));

    ASSERT_EQ(result.vertices.size(), 2);
    EXPECT_EQ(result.vertices[0], triangle[2]);
    EXPECT_EQ(result.vertices[1], triangle[0]);
    EXPECT_NEAR(result.intersection.x, 0.0f, kTolerance);
    EXPECT_NEAR(result.intersection.y, 0.5f, kTolerance);
    EXPECT_NEAR(result.intersection.z, 0.0f, kTolerance);
}

TEST(PathIntersectionAlgorithms, CoplanarSegmentMissesTriangleBoundaryIn3D) {
    const auto triangle = makeTriangle3D();
    const auto result = unboxing_engine::algorithms::checkPathIntersectionWithSegment<float, 3>(
        triangle,
        Vector3f(-0.5f, 2.5f, 0.0f),
        Vector3f(1.5f, 2.5f, 0.0f));

    EXPECT_TRUE(result.vertices.empty());
}
