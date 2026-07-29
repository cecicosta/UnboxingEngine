#pragma once

#include "BoundingBox.h"
#include "MeshBuffer.h"
#include "UVector.h"

#include <cstdint>
#include <map>
#include <vector>

namespace collision_primitives {

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

inline std::vector<Vector3f> GetTriangleVertices(const unboxing_engine::CMeshBuffer& mesh, uint32_t id) {
    std::vector<Vector3f> vertices(3);
    for (unsigned int j = 0; j < 3; ++j) {
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
        for (unsigned int j = 0; j < 3; ++j) {
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

} // namespace collision_primitives
