#include <gtest/gtest.h>

#include <MeshPrimitivesUtils.h>

#include <algorithm>
#include <cmath>
#include <memory>

namespace {

constexpr float kTolerance = 0.0001f;

struct SRange {
    float min = 0.0f;
    float max = 0.0f;
};

struct SBoxRanges {
    SRange x;
    SRange y;
    SRange z;
};

SBoxRanges rangesFor(const CBoundingBox3D &box) {
    const Vector3f first = box.getFirst();
    const Vector3f second = box.getSecond();
    return {
            {std::min(first.x, second.x), std::max(first.x, second.x)},
            {std::min(first.y, second.y), std::max(first.y, second.y)},
            {std::min(first.z, second.z), std::max(first.z, second.z)}};
}

void expectRangeNear(const SRange &actual, const SRange &expected) {
    EXPECT_NEAR(actual.min, expected.min, kTolerance);
    EXPECT_NEAR(actual.max, expected.max, kTolerance);
}

void expectBoxRangesNear(const CBoundingBox3D &box, const SBoxRanges &expected) {
    const SBoxRanges actual = rangesFor(box);
    expectRangeNear(actual.x, expected.x);
    expectRangeNear(actual.y, expected.y);
    expectRangeNear(actual.z, expected.z);
}

void expectTriangleIndicesValid(const unboxing_engine::CMeshBuffer &mesh) {
    for (unsigned int index : mesh.triangles) {
        EXPECT_LT(index, mesh.nvertices);
    }
}

}// namespace

TEST(MeshPrimitivesUtilsCylinder, CreatesClosedCylinderBetweenCenters) {
    const std::unique_ptr<unboxing_engine::CMeshBuffer> mesh =
            unboxing_engine::primitive_utils::Cylinder(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 2.0f), 0.5f, 0.5f, 8);

    ASSERT_NE(mesh, nullptr);
    EXPECT_EQ(mesh->nvertices, 18u);
    EXPECT_EQ(mesh->vertices.size(), mesh->nvertices * 3u);
    EXPECT_EQ(mesh->nfaces, 32u);
    EXPECT_EQ(mesh->triangles.size(), mesh->nfaces * 3u);
    EXPECT_EQ(mesh->nnormals, 0u);
    EXPECT_EQ(mesh->ntexcoords, 0u);
    expectTriangleIndicesValid(*mesh);
    expectBoxRangesNear(mesh->boundingBox, {{-0.5f, 0.5f}, {-0.5f, 0.5f}, {0.0f, 2.0f}});
}

TEST(MeshPrimitivesUtilsCylinder, CreatesConeWhenOneRadiusIsZero) {
    const std::unique_ptr<unboxing_engine::CMeshBuffer> mesh =
            unboxing_engine::primitive_utils::Cylinder(Vector3f(1.0f, 2.0f, 3.0f), Vector3f(1.0f, 2.0f, 5.0f), 0.5f, 0.0f, 8);

    ASSERT_NE(mesh, nullptr);
    EXPECT_EQ(mesh->nvertices, 10u);
    EXPECT_EQ(mesh->vertices.size(), mesh->nvertices * 3u);
    EXPECT_EQ(mesh->nfaces, 16u);
    EXPECT_EQ(mesh->triangles.size(), mesh->nfaces * 3u);
    expectTriangleIndicesValid(*mesh);
    expectBoxRangesNear(mesh->boundingBox, {{0.5f, 1.5f}, {1.5f, 2.5f}, {3.0f, 5.0f}});
}

TEST(MeshPrimitivesUtilsCylinder, SupportsNonZAlignedAxis) {
    const std::unique_ptr<unboxing_engine::CMeshBuffer> mesh =
            unboxing_engine::primitive_utils::Cylinder(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(2.0f, 0.0f, 0.0f), 1.0f, 1.0f, 8);

    ASSERT_NE(mesh, nullptr);
    EXPECT_EQ(mesh->nvertices, 18u);
    EXPECT_EQ(mesh->nfaces, 32u);
    expectTriangleIndicesValid(*mesh);
    expectBoxRangesNear(mesh->boundingBox, {{0.0f, 2.0f}, {-1.0f, 1.0f}, {-1.0f, 1.0f}});
}

TEST(MeshPrimitivesUtilsSphere, CreatesSphereWithRadius) {
    const std::unique_ptr<unboxing_engine::CMeshBuffer> mesh =
            unboxing_engine::primitive_utils::Sphere(2.0f, 8, 4);

    ASSERT_NE(mesh, nullptr);
    EXPECT_EQ(mesh->nvertices, 26u);
    EXPECT_EQ(mesh->vertices.size(), mesh->nvertices * 3u);
    EXPECT_EQ(mesh->nfaces, 48u);
    EXPECT_EQ(mesh->triangles.size(), mesh->nfaces * 3u);
    EXPECT_EQ(mesh->nnormals, 0u);
    EXPECT_EQ(mesh->ntexcoords, 0u);
    expectTriangleIndicesValid(*mesh);
    expectBoxRangesNear(mesh->boundingBox, {{-2.0f, 2.0f}, {-2.0f, 2.0f}, {-2.0f, 2.0f}});

    for (std::size_t vertexOffset = 0; vertexOffset < mesh->vertices.size(); vertexOffset += 3) {
        const float x = mesh->vertices[vertexOffset];
        const float y = mesh->vertices[vertexOffset + 1];
        const float z = mesh->vertices[vertexOffset + 2];
        EXPECT_NEAR(std::sqrt(x * x + y * y + z * z), 2.0f, kTolerance);
    }
}
