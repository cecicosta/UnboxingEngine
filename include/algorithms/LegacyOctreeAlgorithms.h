#pragma once

#include "BoundingBox.h"
#include "MeshBuffer.h"
#include "algorithms/LegacyCollisionAlgorithms.h"

#include <array>
#include <memory>
#include <vector>

namespace unboxing_engine::algorithms {

struct SMeshOctreeNode {
    CBoundingBox3D box;
    std::vector<unsigned int> triangleIndices;
    std::array<std::unique_ptr<SMeshOctreeNode>, 8> children;

    [[nodiscard]] bool IsLeaf() const;
};

struct SMeshOctree {
    std::unique_ptr<SMeshOctreeNode> root;
    unsigned int maxFacesPerNode = 10000;
    unsigned int maxDepth = 8;
};

SMeshOctree buildMeshOctreePorted(
    const CMeshBuffer &mesh,
    unsigned int maxFacesPerNode = 10000,
    unsigned int maxDepth = 8);

std::vector<unsigned int> queryMeshOctreeRayPorted(
    const SMeshOctree &octree,
    const SRay3D &ray);

std::vector<unsigned int> queryMeshOctreeSpherePorted(
    const SMeshOctree &octree,
    const SSphere3D &sphere);

std::vector<STriangleHit3D> rayWithMeshOctreePorted(
    const CMeshBuffer &mesh,
    const SMeshOctree &octree,
    const SRay3D &ray,
    bool firstIntersection = false,
    std::size_t *candidateTriangleCount = nullptr);

} // namespace unboxing_engine::algorithms
