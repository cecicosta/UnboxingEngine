#include <gtest/gtest.h>

#include <COctree.h>
#include <MeshPrimitivesUtils.h>
#include <algorithms/LegacyCollisionAlgorithms.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

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

void collectTriangleIds(const SOctreeNode *node, std::vector<uint32_t> &triangleIds) {
    if (node == nullptr) {
        return;
    }

    triangleIds.insert(triangleIds.end(), node->faces.begin(), node->faces.end());
    for (const auto &child : node->child) {
        collectTriangleIds(child.get(), triangleIds);
    }
}

std::size_t countNodes(const SOctreeNode *node) {
    if (node == nullptr) {
        return 0;
    }

    std::size_t count = 1;
    for (const auto &child : node->child) {
        count += countNodes(child.get());
    }
    return count;
}

std::size_t countTriangleIdsInSubtree(const SOctreeNode *node) {
    if (node == nullptr) {
        return 0;
    }

    std::size_t count = node->faces.size();
    for (const auto &child : node->child) {
        count += countTriangleIdsInSubtree(child.get());
    }
    return count;
}

void expectNoEmptyLeafOrEmptySubtree(const SOctreeNode *node) {
    ASSERT_NE(node, nullptr);

    bool hasChild = false;
    for (const auto &child : node->child) {
        if (!child) {
            continue;
        }

        hasChild = true;
        EXPECT_GT(countTriangleIdsInSubtree(child.get()), 0u);
        expectNoEmptyLeafOrEmptySubtree(child.get());
    }

    if (!hasChild) {
        EXPECT_FALSE(node->faces.empty());
    }
}

std::unique_ptr<CMeshBuffer> makeSmallTorusMesh() {
    return unboxing_engine::primitive_utils::Torus(4.0f, 1.15f, 12, 6);
}

void collectNodesInLevelRange(
        const SOctreeNode *node,
        int currentLevel,
        int fromLevel,
        int toLevel,
        std::vector<const SOctreeNode*> &nodes) {
    if (node == nullptr || currentLevel > toLevel) {
        return;
    }

    if (currentLevel >= fromLevel) {
        nodes.push_back(node);
    }

    for (const auto &child : node->child) {
        collectNodesInLevelRange(child.get(), currentLevel + 1, fromLevel, toLevel, nodes);
    }
}

std::vector<const SOctreeNode*> collectNodesInLevelRange(const SOctreeNode *root, int fromLevel, int toLevel) {
    std::vector<const SOctreeNode*> nodes;
    collectNodesInLevelRange(root, 0, fromLevel, toLevel, nodes);
    return nodes;
}

bool verticesNear(const std::vector<float> &actual, const std::vector<float> &expected) {
    if (actual.size() != expected.size()) {
        return false;
    }

    for (std::size_t i = 0; i < actual.size(); ++i) {
        if (std::abs(actual[i] - expected[i]) > kTolerance) {
            return false;
        }
    }
    return true;
}

void expectDebugMeshMatchesNodes(const CMeshBuffer &debugMesh, const std::vector<const SOctreeNode*> &nodes) {
    EXPECT_EQ(debugMesh.nvertices, nodes.size() * 8);
    EXPECT_EQ(debugMesh.vertices.size(), debugMesh.nvertices * 3);
    EXPECT_EQ(debugMesh.nfaces, nodes.size() * 12);
    EXPECT_EQ(debugMesh.triangles.size(), debugMesh.nfaces * 3);

    for (uint32_t index : debugMesh.triangles) {
        EXPECT_LT(index, debugMesh.nvertices);
    }

    std::vector<bool> matched(nodes.size(), false);
    for (std::size_t boxIndex = 0; boxIndex < nodes.size(); ++boxIndex) {
        const uint32_t baseVertex = static_cast<uint32_t>(boxIndex * 8);
        const std::size_t vertexOffset = boxIndex * 24;
        ASSERT_LE(vertexOffset + 24, debugMesh.vertices.size());

        std::vector<float> actualVertices(
                debugMesh.vertices.begin() + vertexOffset,
                debugMesh.vertices.begin() + vertexOffset + 24);

        bool foundMatch = false;
        for (std::size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex) {
            if (matched[nodeIndex]) {
                continue;
            }

            if (verticesNear(actualVertices, nodes[nodeIndex]->box.GetVertices())) {
                matched[nodeIndex] = true;
                foundMatch = true;
                break;
            }
        }
        EXPECT_TRUE(foundMatch) << "debug box " << boxIndex << " did not match any octree node";

        const std::size_t triangleOffset = boxIndex * 36;
        for (std::size_t i = triangleOffset; i < triangleOffset + 36; ++i) {
            ASSERT_LT(i, debugMesh.triangles.size());
            EXPECT_GE(debugMesh.triangles[i], baseVertex);
            EXPECT_LT(debugMesh.triangles[i], baseVertex + 8);
        }
    }

    for (std::size_t nodeIndex = 0; nodeIndex < matched.size(); ++nodeIndex) {
        EXPECT_TRUE(matched[nodeIndex]) << "octree node " << nodeIndex << " was missing from debug mesh";
    }
}

} // namespace

TEST(COctree, SubdivideCreatesEightHalfSizedChildrenCoveringParent) {
    SOctreeNode node(Vector3f(-2.0f, -4.0f, -6.0f), Vector3f(10.0f, 20.0f, 30.0f));
    node.Subdivide();

    const SBoxRanges expected[] = {
        {{4.0f, 10.0f}, {8.0f, 20.0f}, {12.0f, 30.0f}},
        {{4.0f, 10.0f}, {8.0f, 20.0f}, {-6.0f, 12.0f}},
        {{-2.0f, 4.0f}, {8.0f, 20.0f}, {-6.0f, 12.0f}},
        {{-2.0f, 4.0f}, {8.0f, 20.0f}, {12.0f, 30.0f}},
        {{4.0f, 10.0f}, {-4.0f, 8.0f}, {12.0f, 30.0f}},
        {{4.0f, 10.0f}, {-4.0f, 8.0f}, {-6.0f, 12.0f}},
        {{-2.0f, 4.0f}, {-4.0f, 8.0f}, {-6.0f, 12.0f}},
        {{-2.0f, 4.0f}, {-4.0f, 8.0f}, {12.0f, 30.0f}}};

    for (std::size_t i = 0; i < node.child.size(); ++i) {
        ASSERT_NE(node.child[i], nullptr);
        expectBoxRangesNear(node.child[i]->box, expected[i]);
    }
}

TEST(COctree, StoresEveryOriginalTriangleExactlyOnce) {
    const std::unique_ptr<CMeshBuffer> mesh = makeSmallTorusMesh();
    const std::size_t triangleCount = mesh->triangles.size() / 3;
    const COctree octree(*mesh, 8, 4);

    ASSERT_NE(octree.root, nullptr);
    EXPECT_GT(countNodes(octree.root.get()), 1u);

    std::vector<uint32_t> triangleIds;
    collectTriangleIds(octree.root.get(), triangleIds);

    ASSERT_EQ(triangleIds.size(), triangleCount);

    std::vector<unsigned int> counts(triangleCount, 0);
    for (uint32_t triangleId : triangleIds) {
        ASSERT_LT(triangleId, triangleCount);
        ++counts[triangleId];
    }

    for (std::size_t triangleId = 0; triangleId < counts.size(); ++triangleId) {
        EXPECT_EQ(counts[triangleId], 1u) << "triangle id " << triangleId;
    }
}

TEST(COctree, DoesNotKeepRedundantEmptyOctants) {
    const std::unique_ptr<CMeshBuffer> mesh = makeSmallTorusMesh();
    const COctree octree(*mesh, 8, 4);

    ASSERT_NE(octree.root, nullptr);
    EXPECT_GT(countNodes(octree.root.get()), 1u);
    expectNoEmptyLeafOrEmptySubtree(octree.root.get());
}

TEST(COctreeDebugMesh, EmitsOneValidBoxMeshPerOctreeNode) {
    const std::unique_ptr<CMeshBuffer> mesh = makeSmallTorusMesh();
    const COctree octree(*mesh, 8, 4);
    const std::vector<const SOctreeNode*> nodes = collectNodesInLevelRange(octree.root.get(), 0, 999);
    ASSERT_GT(nodes.size(), 1u);

    const std::unique_ptr<CMeshBuffer> debugMesh = octree.Debug_CreateBoundingBoxesMesh();

    ASSERT_NE(debugMesh, nullptr);
    expectDebugMeshMatchesNodes(*debugMesh, nodes);
}

TEST(COctreeDebugMesh, EmitsOnlyRequestedLevelRange) {
    const std::unique_ptr<CMeshBuffer> mesh = makeSmallTorusMesh();
    const COctree octree(*mesh, 8, 4);
    const std::vector<const SOctreeNode*> levelOneNodes = collectNodesInLevelRange(octree.root.get(), 1, 1);
    ASSERT_FALSE(levelOneNodes.empty());

    const std::unique_ptr<CMeshBuffer> debugMesh = octree.Debug_CreateBoundingBoxesMesh(1, 1);

    ASSERT_NE(debugMesh, nullptr);
    expectDebugMeshMatchesNodes(*debugMesh, levelOneNodes);
}
