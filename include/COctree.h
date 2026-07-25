#pragma once

#include "Matrix.h"
#include "MeshBuffer.h"

#include "UVector.h"
#include "BoundingBox.h"

using namespace unboxing_engine;

struct SOctreeNode {
    SOctreeNode(const Vector3f& first, const Vector3f& second);

    // Subdivide this node into 8 octant, for a new cctree branch
    void Subdivide();

    CBoundingBox3D box;
    std::vector<std::unique_ptr<SOctreeNode>> child{8};
    std::vector<uint32_t> faces;
};

class COctree {
public:
    const CMeshBuffer& mesh;
    std::unique_ptr<SOctreeNode> root;
    COctree(const CMeshBuffer& mesh, uint32_t maxTriangles, uint32_t maxDepth);

    [[nodiscard]] std::unique_ptr<CMeshBuffer> Debug_CreateBoundingBoxesMesh(int fromLevel = -1, int toLevel = 999) const;
private:
    static bool isInsideOct(const Vector3f& vertice, const SOctreeNode& node);
    void createOctree(SOctreeNode &node, uint32_t depth, const std::vector<uint32_t> &triangleIds);

    uint32_t maxTriangles;
    uint32_t maxDepth;
};
