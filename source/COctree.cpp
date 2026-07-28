#include "COctree.h"

#include "algorithms/LegacyCollisionAlgorithms.h"
#include "algorithms/OctreeCollision.h"

#include <iostream>
#include <ostream>

SOctreeNode::SOctreeNode(const Vector3f& first, const Vector3f& second) : box(CBoundingBox3D(first, second)) {}

COctree::COctree(const CMeshBuffer& mesh, uint32_t maxTriangles, uint32_t maxDepth)
    : mesh(mesh)
    , root(std::make_unique<SOctreeNode>(mesh.boundingBox.getSecond(), mesh.boundingBox.getFirst()))
    , maxTriangles(maxTriangles)
    , maxDepth(maxDepth) {
    std::vector<uint32_t> triangleIds;
    triangleIds.reserve(mesh.triangles.size() / 3);

    for (uint32_t triangleId = 0; triangleId < mesh.triangles.size() / 3; ++triangleId) {
        triangleIds.push_back(triangleId);
    }

    createOctree(*root, 0, triangleIds);
}

void COctree::createOctree(SOctreeNode &node, uint32_t depth, const std::vector<uint32_t> &triangleIds) {
    if (triangleIds.size() < maxTriangles || maxDepth == depth) {
        for (uint32_t id: triangleIds) {
            node.faces.push_back(id);
        }
        return;
    }

    node.Subdivide();

    std::vector<uint32_t> childrenTriangles[8];
    for (uint32_t id: triangleIds) {

        // Pick up the triangle vertices
        //GetTriangleFromArray(mesh.vertices, id, vertice[0], vertice[1], vertice[2]);
        auto vertice = GetTriangleVertices(mesh, id);

        bool vertexForwarded = false;
        // Must verify if one of the node's children fully contain the triangle
        for (int i = 0; i < 8; ++i) {
            // Assume the triangle vertices are all inside the child's box
            bool fullyContained = true;
            for (const Vector3f &v: vertice) {
                if (!isInsideOct(v, *node.child[i].get())) {
                    // If one vertex is not inside the child's box, stop the test and check the next child
                    fullyContained = false;
                    break;
                }
            }
            // If the triangle is fully inside the child's box, reserve the vertex for that child in the next recursion level and skip to the next triangle
            if (fullyContained) {
                childrenTriangles[i].push_back(id);
                vertexForwarded = true;
                break;
            }
        }

        // If the vertex is not fully contained in one of the children boxes, assign to the parent.
        if (!vertexForwarded) {
            node.faces.push_back(id);
        }
    }
    for (int i = 0; i < 8; ++i) {
        if (!childrenTriangles[i].empty()) {
            createOctree(*node.child[i].get(), depth + 1, childrenTriangles[i]);
        } else {
            node.child[i].reset();
        }
    }
}

bool COctree::isInsideOct(const Vector3f& vertice, const SOctreeNode& node) {
    return algorithms::isInsideBoundingBoxPorted(node.box, vertice);
}


void SOctreeNode::Subdivide() {
    float xSize, ySize, zSize;
    Vector3f omphalos, maximum, minimum;
    Vector3f center = box.getPivot();
    Vector3f max = box.getSecond();
    Vector3f min = box.getFirst();

    //Create first octant
    xSize = max.x - center.x;
    ySize = max.y - center.y;
    zSize = max.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    minimum = omphalos - Vector3f(xSize / 2, ySize / 2, zSize / 2);
    child[0] = std::make_unique<SOctreeNode>(minimum, maximum);

    //Create second octant
    xSize = max.x - center.x;
    ySize = max.y - center.y;
    zSize = min.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(xSize / 2, ySize / 2, -zSize / 2);
    minimum = omphalos - Vector3f(xSize / 2, ySize / 2, -zSize / 2);
    child[1] = std::make_unique<SOctreeNode>(minimum, maximum);

    //Create third octant
    xSize = min.x - center.x;
    ySize = max.y - center.y;
    zSize = min.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(-xSize / 2, ySize / 2, -zSize / 2);
    minimum = omphalos - Vector3f(-xSize / 2, ySize / 2, -zSize / 2);
    child[2] = std::make_unique<SOctreeNode>(minimum, maximum);

    //Create forth octant
    xSize = min.x - center.x;
    ySize = max.y - center.y;
    zSize = max.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(-xSize / 2, ySize / 2, zSize / 2);
    minimum = omphalos - Vector3f(-xSize / 2, ySize / 2, zSize / 2);
    child[3] = std::make_unique<SOctreeNode>(minimum, maximum);

    //Create fifth octant
    xSize = max.x - center.x;
    ySize = min.y - center.y;
    zSize = max.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(xSize / 2, -ySize / 2, zSize / 2);
    minimum = omphalos - Vector3f(xSize / 2, -ySize / 2, zSize / 2);
    child[4] = std::make_unique<SOctreeNode>(minimum, maximum);

    //Create sixth octant
    xSize = max.x - center.x;
    ySize = min.y - center.y;
    zSize = min.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(xSize / 2, -ySize / 2, -zSize / 2);
    minimum = omphalos - Vector3f(xSize / 2, -ySize / 2, -zSize / 2);
    child[5] = std::make_unique<SOctreeNode>(minimum, maximum);

    //seventh
    xSize = min.x - center.x;
    ySize = min.y - center.y;
    zSize = min.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(-xSize / 2, -ySize / 2, -zSize / 2);
    minimum = omphalos - Vector3f(-xSize / 2, -ySize / 2, -zSize / 2);
    child[6] = std::make_unique<SOctreeNode>(minimum, maximum);

    //Create eighth octant
    xSize = min.x - center.x;
    ySize = min.y - center.y;
    zSize = max.z - center.z;
    omphalos = center + Vector3f(xSize / 2, ySize / 2, zSize / 2);
    maximum = omphalos + Vector3f(-xSize / 2, -ySize / 2, zSize / 2);
    minimum = omphalos - Vector3f(-xSize / 2, -ySize / 2, zSize / 2);
    child[7] = std::make_unique<SOctreeNode>(minimum, maximum);
}

void static CreateBoundingBoxesMesh(const SOctreeNode * node, CMeshBuffer& octreeMeshBuffer, int fromLevel, int toLevel) {
    if (node == nullptr || toLevel < 0) {
        return;
    }

    if (fromLevel <= 0 ) {
        DebugHelpers::AddBoxToMesh(node->box, octreeMeshBuffer);
    }

    for (auto && child: node->child) {
        if (child == nullptr) {
            continue;
        }
        CreateBoundingBoxesMesh(child.get(), octreeMeshBuffer, fromLevel-1, toLevel-1);
    }
}

std::unique_ptr<CMeshBuffer> COctree::Debug_CreateBoundingBoxesMesh(int fromLevel, int toLevel) const {
    std::unique_ptr<CMeshBuffer> octreeMeshBuffer = std::make_unique<CMeshBuffer>();
    CreateBoundingBoxesMesh(root.get(), *octreeMeshBuffer, fromLevel, toLevel );

    octreeMeshBuffer->nvertices += octreeMeshBuffer->vertices.size() / 3;
    octreeMeshBuffer->nfaces += octreeMeshBuffer->triangles.size() / 3;

    return std::move(octreeMeshBuffer);
}


