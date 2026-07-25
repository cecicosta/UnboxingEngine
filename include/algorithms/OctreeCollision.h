#pragma once

#include "COctree.h"

#include <span>

using namespace unboxing_engine;

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


inline void GetTriangleFromArray(const std::vector<float>& vertices, unsigned triangleIndex, Vector3f& outV1, Vector3f& outV2, Vector3f& outV3) {
    Vector3f* triangle[] = {&outV1, &outV2, &outV3};
    // Iterate over the triangle vertices
    for (int i = 0; i < 3; ++i) {
        float x = vertices[triangleIndex + i*3];
        float y = vertices[triangleIndex + i*3 + 1];
        float z = vertices[triangleIndex + i*3 + 2];
        *triangle[i] = Vector3f(x, y, z);
    }

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
    for (unsigned triangleIndex: box.GetTriangles()) {
        Vector3f v1, v2, v3;
        GetTriangleFromArray(vertices, triangleIndex, v1, v2, v3);
        Vector3f normal = (v2 - v1).CrossProduct(v3 - v1).Normalized();
        Vector3f ref = v1;

        if (const auto [hit, t, point] = IntersectionRayWithTriangle(v1, v2, v3, start, direction); hit) {
            orderedIntersections.insert({t, point});
        }
    }

    if (!orderedIntersections.empty()) {
        firstHit = orderedIntersections.begin()->second;
        return true;
    }

    return false;
}


inline SRayTriangleHit RayWithOctree(const Vector3f &start, const Vector3f &direction, const COctree &octree) {

    std::vector<SOctreeNode*> nodes;
    Vector3f firstHitWithBox;
    if (IntersectionRayWithBox(octree.root->box, start, direction, firstHitWithBox)) {
        nodes.push_back(octree.root.get());
    }

    std::map<float, SRayTriangleHit> orderedIntersections;
    while (!nodes.empty()) {
        SOctreeNode *u = nodes[0];
        nodes.erase(nodes.begin());
        for (auto && child: u->child) {
            if (child && IntersectionRayWithBox(child->box, start, direction, firstHitWithBox)) {
                nodes.push_back(child.get());
            }
        }

        for (auto && triangleIndex: u->faces) {
            Vector3f v1, v2, v3;
            GetTriangleFromArray(octree.mesh.vertices, triangleIndex, v1, v2, v3);

            const auto [hit, t, intersection] = IntersectionRayWithTriangle(v1, v2, v3, start, direction);
            if (hit) {
                SRayTriangleHit hitTriangle{true, t, intersection, v1, v2, v3};
                orderedIntersections.insert({t, hitTriangle});
            }
        }
    }
    if (!orderedIntersections.empty()) {
        return orderedIntersections.begin()->second;
    }

    return {};
}