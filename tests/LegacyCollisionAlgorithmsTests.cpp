#include <gtest/gtest.h>

#include <algorithms/LegacyCollisionAlgorithms.h>
#include <algorithms/LegacyOctreeAlgorithms.h>

namespace {

using namespace unboxing_engine::algorithms;
using unboxing_engine::CMeshBuffer;

constexpr float kTolerance = 0.0001f;

CMeshBuffer makeSingleTriangleMesh() {
    CMeshBuffer mesh;
    mesh.vertices = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f};
    mesh.triangles = {0, 1, 2};
    mesh.boundingBox = CBoundingBox3D({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f});
    mesh.nvertices = 3;
    mesh.nfaces = 1;
    return mesh;
}

CMeshBuffer makeTwoPlaneMesh() {
    CMeshBuffer mesh;
    mesh.vertices = {
        0.0f, 0.0f, 1.0f,
        2.0f, 0.0f, 1.0f,
        0.0f, 2.0f, 1.0f,
        0.0f, 0.0f, 3.0f,
        2.0f, 0.0f, 3.0f,
        0.0f, 2.0f, 3.0f};
    mesh.triangles = {3, 4, 5, 0, 1, 2};
    mesh.boundingBox = CBoundingBox3D({0.0f, 0.0f, 1.0f}, {2.0f, 2.0f, 3.0f});
    mesh.nvertices = 6;
    mesh.nfaces = 2;
    return mesh;
}

CMeshBuffer makeCubeMesh() {
    CMeshBuffer mesh;
    mesh.boundingBox = CBoundingBox3D({0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 2.0f});
    mesh.vertices = mesh.boundingBox.GetVertices();
    mesh.triangles = mesh.boundingBox.GetTriangles();
    mesh.nvertices = 8;
    mesh.nfaces = 12;
    return mesh;
}

} // namespace

TEST(LegacyCollisionAlgorithms, DetectsPointInsideTriangle) {
    EXPECT_TRUE(isInsideTrianglePorted(
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.25f, 0.25f, 0.0f}));
}

TEST(LegacyCollisionAlgorithms, RejectsPointOutsideTriangle) {
    EXPECT_FALSE(isInsideTrianglePorted(
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.75f, 0.75f, 0.0f}));
}

TEST(LegacyCollisionAlgorithms, RayHitsTriangle) {
    const auto hit = rayWithTrianglePorted(
        {{0.25f, 0.25f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f});

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->intersection.x, 0.25f, kTolerance);
    EXPECT_NEAR(hit->intersection.y, 0.25f, kTolerance);
    EXPECT_NEAR(hit->intersection.z, 0.0f, kTolerance);
    EXPECT_NEAR(hit->distance, 1.0f, kTolerance);
}

TEST(LegacyCollisionAlgorithms, RayMissesTriangle) {
    const auto hit = rayWithTrianglePorted(
        {{1.25f, 1.25f, -1.0f}, {0.0f, 0.0f, 1.0f}},
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f});

    EXPECT_FALSE(hit.has_value());
}

TEST(LegacyCollisionAlgorithms, RayHitsMeshBuffer) {
    const CMeshBuffer mesh = makeSingleTriangleMesh();
    const auto hits = rayWithMeshBufferPorted(mesh, {{0.25f, 0.25f, -1.0f}, {0.0f, 0.0f, 1.0f}});

    ASSERT_EQ(hits.size(), 1);
    EXPECT_EQ(hits[0].triangleIndex, 0);
}

TEST(LegacyCollisionAlgorithms, MeshRayHitsAreSortedByDistance) {
    const CMeshBuffer mesh = makeTwoPlaneMesh();
    const auto hits = rayWithMeshBufferPorted(mesh, {{0.25f, 0.25f, 0.0f}, {0.0f, 0.0f, 1.0f}});

    ASSERT_EQ(hits.size(), 2);
    EXPECT_NEAR(hits[0].distance, 1.0f, kTolerance);
    EXPECT_NEAR(hits[1].distance, 3.0f, kTolerance);
}

TEST(LegacyCollisionAlgorithms, FirstMeshRayHitReturnsNearestIntersection) {
    const CMeshBuffer mesh = makeTwoPlaneMesh();
    const auto hits = rayWithMeshBufferPorted(mesh, {{0.25f, 0.25f, 0.0f}, {0.0f, 0.0f, 1.0f}}, true);

    ASSERT_EQ(hits.size(), 1);
    EXPECT_NEAR(hits[0].distance, 1.0f, kTolerance);
}

TEST(LegacyCollisionAlgorithms, SphereHitsTriangle) {
    const auto hit = sphereWithTrianglePorted(
        {{0.25f, 0.25f, 0.2f}, 0.25f},
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f});

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->intersection.z, 0.0f, kTolerance);
}

TEST(LegacyCollisionAlgorithms, SphereMissesTriangle) {
    const auto hit = sphereWithTrianglePorted(
        {{0.25f, 0.25f, 0.2f}, 0.1f},
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f});

    EXPECT_FALSE(hit.has_value());
}

TEST(LegacyCollisionAlgorithms, BoundingBoxHelpersDetectPointRayAndSphere) {
    const CBoundingBox3D box({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f});

    EXPECT_TRUE(isInsideBoundingBoxPorted(box, {0.5f, 0.5f, 0.5f}));
    EXPECT_FALSE(isInsideBoundingBoxPorted(box, {1.5f, 0.5f, 0.5f}));

    const auto rayHit = rayWithBoundingBoxPorted(box, {{0.5f, 0.5f, -2.0f}, {0.0f, 0.0f, 1.0f}});
    ASSERT_TRUE(rayHit.has_value());
    EXPECT_NEAR(*rayHit, 2.0f, kTolerance);

    EXPECT_TRUE(sphereWithBoundingBoxPorted(box, {{0.5f, 0.5f, 1.2f}, 0.25f}));
    EXPECT_FALSE(sphereWithBoundingBoxPorted(box, {{2.0f, 2.0f, 2.0f}, 0.1f}));
}

TEST(LegacyOctreeAlgorithms, BuildsOctreeAndQueriesRayCandidates) {
    const CMeshBuffer mesh = makeCubeMesh();
    const auto octree = buildMeshOctreePorted(mesh, 2, 3);

    ASSERT_TRUE(octree.root != nullptr);

    const auto candidates = queryMeshOctreeRayPorted(octree, {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}});
    EXPECT_FALSE(candidates.empty());
    EXPECT_LE(candidates.size(), mesh.triangles.size() / 3);
}

TEST(LegacyOctreeAlgorithms, RayHitsMeshThroughOctree) {
    const CMeshBuffer mesh = makeCubeMesh();
    const auto octree = buildMeshOctreePorted(mesh, 2, 3);

    const auto hits = rayWithMeshOctreePorted(mesh, octree, {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}}, true);
    ASSERT_EQ(hits.size(), 1);
    EXPECT_NEAR(hits[0].distance, 1.0f, kTolerance);
}
