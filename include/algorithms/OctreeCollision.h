#pragma once

#include "COctree.h"

#include <span>

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


struct SRayPointHit {
    bool hit;
    float t;
    Vector3f point;
};

struct SRayTriangleHit {
    bool hit = false;
    float t = 0.0f;
    Vector3f intersection;
    Vector3f vertices[3];
};

inline bool IsPointInsideTriangle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, const Vector3f& point) {
    const float area = (v2 - v1).CrossProduct(v3 - v1).Length() * 0.5f;
    Vector3f vet1 = point - v1;
    Vector3f vet2 = v2 - v1;
    const float area1 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = point - v1;
    vet2 = v3 - v1;
    const float area2 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = point - v3;
    vet2 = v2 - v3;
    const float area3 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    return area1 + area2 + area3 <= 1.0f;
}

inline std::vector<Vector3f> GetTriangleVertices(const CMeshBuffer& mesh, uint32_t id) {
    std::vector<Vector3f> vertices(3);
    for (uint j = 0; j < 3; ++j) {
        const uint32_t vertexIndex = mesh.triangles[id * 3 + j];
        const uint32_t vertexOffset = vertexIndex * 3;

        vertices[j].x = mesh.vertices[vertexOffset];
        vertices[j].y = mesh.vertices[vertexOffset + 1];
        vertices[j].z = mesh.vertices[vertexOffset + 2];
    }
    return vertices;
}

inline SRayPointHit IntersectionRayWithTriangle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, const Vector3f& start, const Vector3f& direction) {
    const Vector3f normal = (v2 - v1).CrossProduct(v3 - v1).Normalized();
    const Vector3f ref = v1;

    // Use the plane equation definition to obtain a 't' for when the ray intersects the plane
    const float a = normal.x;
    const float b = normal.y;
    const float c = normal.z;
    const float d = -(a * ref.x + b * ref.y + c * ref.z);

    const float t = -(d + a * start.x + b * start.y + c * start.z) / (a * direction.x + b * direction.y + c * direction.z);

    Vector3f planeRayIntersection = start + t * direction;
    if (IsPointInsideTriangle(v1, v2, v3, planeRayIntersection)) {
        return {true, t, planeRayIntersection};
    }
    return {false, t, planeRayIntersection};
}

inline bool IntersectionRayWithBox(const CBoundingBox3D& box, const Vector3f& start, const Vector3f& direction, Vector3f &firstHit) {
    std::map<float, Vector3f> orderedIntersections;

    const auto vertices = box.GetVertices();
    const auto triangles = box.GetTriangles();
    for (unsigned triangleIndex: triangles) {

        std::vector<Vector3f> triangleVertices(3);
        for (uint j = 0; j < 3; ++j) {
            const uint32_t vertexIndex = triangles[triangleIndex * 3 + j];
            const uint32_t vertexOffset = vertexIndex * 3;

            triangleVertices[j].x = vertices[vertexOffset];
            triangleVertices[j].y = vertices[vertexOffset + 1];
            triangleVertices[j].z = vertices[vertexOffset + 2];
        }

        Vector3f normal = (triangleVertices[1] - triangleVertices[0]).CrossProduct(triangleVertices[2] - triangleVertices[0]).Normalized();
        if (const auto [hit, t, point] = IntersectionRayWithTriangle(triangleVertices[0], triangleVertices[1], triangleVertices[2], start, direction); hit) {
            orderedIntersections.insert({t, point});
        }
    }

    if (!orderedIntersections.empty()) {
        firstHit = orderedIntersections.begin()->second;
        return true;
    }

    return false;
}


inline SRayTriangleHit RayWithOctree(const Vector3f &start, const Vector3f &direction, const COctree &octree, CMeshBuffer *boxesDebugPathMesh = nullptr) {

    std::vector<SOctreeNode*> nodes;
    Vector3f firstHitWithBox;
    if (IntersectionRayWithBox(octree.root->box, start, direction, firstHitWithBox)) {
        if (boxesDebugPathMesh) {
            DebugHelpers::AddBoxToMesh(octree.root->box, *boxesDebugPathMesh);
        }
        nodes.push_back(octree.root.get());
    }

    std::map<float, SRayTriangleHit> orderedIntersections;
    while (!nodes.empty()) {
        SOctreeNode *u = nodes[0];
        nodes.erase(nodes.begin());
        for (auto && child: u->child) {
            if (child && IntersectionRayWithBox(child->box, start, direction, firstHitWithBox)) {
                if (boxesDebugPathMesh) {
                    DebugHelpers::AddBoxToMesh(child->box, *boxesDebugPathMesh);
                }
                nodes.push_back(child.get());
            }
        }

        for (auto && triangleIndex: u->faces) {
            auto vertices = GetTriangleVertices(octree.mesh, triangleIndex);

            const auto [hit, t, intersection] = IntersectionRayWithTriangle(vertices[0], vertices[1], vertices[2], start, direction);
            if (hit) {
                SRayTriangleHit hitTriangle{true, t, intersection, vertices[0], vertices[1], vertices[2]};
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