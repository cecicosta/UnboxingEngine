#pragma once

#include "COctree.h"
#include "algorithms/CollisionPrimitives.h"

#include <cstdint>
#include <map>
#include <vector>

using namespace unboxing_engine;

namespace DebugHelpers {
inline void AddBoxToMesh(const CBoundingBox3D& box, CMeshBuffer& meshBuffer) {
    auto vertices = box.GetVertices();
    auto triangles = box.GetTriangles();
    const uint32_t baseVertex = meshBuffer.vertices.size() / 3;
    meshBuffer.vertices.insert(meshBuffer.vertices.end(), vertices.begin(), vertices.end());
    for (uint32_t index : triangles) {
        meshBuffer.triangles.push_back(baseVertex + index);
    }
}

}


inline collision_primitives::SRayTriangleHit RayWithOctree(const Vector3f &start, const Vector3f &direction, const COctree &octree, CMeshBuffer *boxesDebugPathMesh = nullptr) {

    std::vector<SOctreeNode*> nodes;
    Vector3f firstHitWithBox;
    if (collision_primitives::IntersectionRayWithBox(octree.root->box, start, direction, firstHitWithBox)) {
        if (boxesDebugPathMesh) {
            DebugHelpers::AddBoxToMesh(octree.root->box, *boxesDebugPathMesh);
        }
        nodes.push_back(octree.root.get());
    }

    std::map<float, collision_primitives::SRayTriangleHit> orderedIntersections;
    while (!nodes.empty()) {
        SOctreeNode *u = nodes[0];
        nodes.erase(nodes.begin());
        for (auto && child: u->child) {
            if (child && collision_primitives::IntersectionRayWithBox(child->box, start, direction, firstHitWithBox)) {
                if (boxesDebugPathMesh) {
                    DebugHelpers::AddBoxToMesh(child->box, *boxesDebugPathMesh);
                }
                nodes.push_back(child.get());
            }
        }

        for (auto && triangleIndex: u->faces) {
            auto vertices = collision_primitives::GetTriangleVertices(octree.mesh, triangleIndex);

            const auto [hit, t, intersection] = collision_primitives::IntersectionRayWithTriangle(vertices[0], vertices[1], vertices[2], start, direction);
            if (hit) {
                collision_primitives::SRayTriangleHit hitTriangle{true, t, intersection, vertices[0], vertices[1], vertices[2]};
                orderedIntersections.insert({t, hitTriangle});
            }
        }
    }
    if (!orderedIntersections.empty()) {
        if (boxesDebugPathMesh) {
            boxesDebugPathMesh->nvertices += boxesDebugPathMesh->vertices.size() / 3;
            boxesDebugPathMesh->nfaces += boxesDebugPathMesh->triangles.size() / 3;
        }
        return orderedIntersections.begin()->second;
    }

    return {};
}


inline void SphereWithOctree(const Vector3f &center, float radius, CMeshBuffer &meshBuffer) {


}