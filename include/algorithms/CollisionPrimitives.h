#pragma once

#include "BoundingBox.h"
#include "MeshBuffer.h"
#include "UVector.h"

#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>

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

struct SCircleEdgeHit {
    float distance;
    Vector3f v1;
    Vector3f v2;
    Vector3f normal;
    Vector3f centerProjection;
};
struct SCircleTriangleHit {
    bool hit = false;
    std::vector<SCircleEdgeHit> edges;
    Vector3f planeNormal;
    Vector3f sCPotP; // Sphere's center projection on the plane
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

/**
 * Extend a ray from its start, in the given direction, until the plane defined by the 3 given vectors is hit.
 * Obtain the parameter t, as output, for the direction multiplier, in order for the ray to hit the plane.
 */
inline void CrossRayWithPlane(const Vector3f& start, const Vector3f& direction, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, float &t) {
    // Simplified logic from the one using the plane equation.
    // Project everything on the plane normal, calculate the respective distances
    // and obtain the multiplier factor t, for the ray reaching the plane
    const Vector3f normal = (v2 - v1).CrossProduct(v3 - v2).Normalized();
    //const float distanceFromOrigin = v1.DotProduct(normal);
    //t = (distanceFromOrigin - start.DotProduct(normal)) / (direction.DotProduct(normal));

    t = v1.DotProduct(normal) - start.DotProduct(normal) / direction.DotProduct(normal);

    // Keeping calculation from plane equation for future reference
//    const float a = normal.x;
//    const float b = normal.y;
//    const float c = normal.z;
//    const float d = -(a * v1.x + b * v1.y + c * v1.z);

//    // Use the plane equation definition to obtain a 't' for when the ray intersects the plane
//    const float t = -(d + a * start.x + b * start.y + c * start.z) / (a * direction.x + b * direction.y + c * direction.z);

}

inline SRayPointHit IntersectionRayWithTriangle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, const Vector3f& start, const Vector3f& direction) {
    float t = 0;
    CrossRayWithPlane(start, direction, v1, v2, v3, t  );

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

/**
 * The algorithm strategy is to calculate the orthogonal distance of the sphere to the plane containing the triangle.
 * This distance, will define the closest point in the plane that can collide with the sphere.
 * If the point is inside the sphere, then the collision happened and this point would have been the first contact.
 *
 * Keep in mind that, for collision purposes, if the closest point of the sphere is behind the plane, it
 * could mean a small portion of the sphere crossed to behind plane, the sphere is coming from behind
 * the plane (not yet colliding) or the sphere is coming from behind the plane, collided and more than half, crossed over.
 * A direction analysis would be necessary to determine if and from where the collision occurred. As well as if it should
 * have occurred prior to the full extent of the detected intersection.
 */
inline SCircleTriangleHit IntersectionSphereWithTriangle(const Vector3f& center, float radius, const Vector3f& v1, const Vector3f& v2, const Vector3f& v3) {
    const Vector3f normal = (v2 - v1).CrossProduct(v3 - v1).Normalized();
    float t = 0;
    CrossRayWithPlane(center, normal, v1, v2, v3, t  );
    auto sCPotP = center + normal * t;

    // Verify the sphere is crossing the plane defined by the face (distance from plane less than the radius)
    if (std::abs(t) > radius) {
        return {false};
    }

    // Radius of the sphere cross-section intersecting the plane
    // Must be careful where using the regular or sqr version
    float crossSectionRadiusSqr = radius*radius - t*t; // Having the sqr version promotes faster comparison when used with SqrLength
    float crossSectionRadius = std::sqrtf(crossSectionRadiusSqr);

    SCircleTriangleHit hit{false};
    hit.planeNormal = normal;
    hit.sCPotP = sCPotP;

    Vector3f centroid = (v1 + v2 + v3) / 3.0f;
    // Find the distance from the centroid to the furthest vertex
    float furthestVertice = std::fmax((v1 - centroid).SqrLength(), (v2 - centroid).SqrLength());
    furthestVertice = std::fmax(furthestVertice, (v3 - centroid).SqrLength());

    // If the SCPonP + radius is further from the centroid than the furthest vertex, then no collision could have occurred.
    // Can't use SqrLength, as a^2 - b^2 > c^2 does not directly imply a - b > c. Maybe can be improved later exploring other
    // mathematical relations
    if ((sCPotP - centroid).Length() - crossSectionRadius > sqrt(furthestVertice)) {
        return {false};
    }

    std::vector<std::pair<Vector3f, Vector3f>> triangleEdges = {{v1, v2}, {v2, v3}, {v3, v1}};
    unsigned edgesFacingCenter = 0;
    for (const auto& edge: triangleEdges) {
        Vector3f v1ToSCPotP = sCPotP - edge.first;

        auto edgeDirection = (edge.second - edge.first).Normalized();
        auto sCPotE = edge.first + (sCPotP - edge.first).DotProduct(edgeDirection) * edgeDirection;
        auto projectionsDistance = sCPotP - sCPotE;

        auto edgeNormal = edgeDirection.CrossProduct(normal).Normalized();
        SCircleEdgeHit candidateEdge;
        candidateEdge.distance = projectionsDistance.DotProduct(edgeNormal); // Remember this is actual distance and can be negative
        candidateEdge.v1 = edge.first;
        candidateEdge.v2 = edge.second;
        candidateEdge.normal = edgeNormal;
        candidateEdge.centerProjection = sCPotE;
        hit.edges.emplace_back(candidateEdge);

        // Equivalent to say the point has a positive projection over the edge's normal
        if (candidateEdge.distance > 0) {
            edgesFacingCenter++;
        }
    }

    // Sort edges, so the ones with greater orthogonal distance from the SCPonP and the SCPonE, come first.
    std::sort(hit.edges.begin(), hit.edges.end(), [](const SCircleEdgeHit& edge1, const SCircleEdgeHit& edge2) {
        return edge1.distance > edge2.distance;
    });

    if (edgesFacingCenter == 0) {
        // Sphere center projection on the plane (SCPotP) is inside the triangle
        hit.hit = true;
    } else if (edgesFacingCenter == 1) {
        // SCPotP is over a single edge of the triangle
        // For the intersection to occur, the distance from the SCPotP and the sphere center projection on the edge
        // (SCPotE) must be smaller than the radius.
        hit.hit = hit.edges[0].distance <= crossSectionRadius;
    } else if (edgesFacingCenter == 2) {
        // For the intersection to have occurred, the vertex shared by the 2 edges must be inside the SCPotP.
        bool isVerticeInside = (hit.edges[0].v1 - sCPotP).SqrLength() <= crossSectionRadiusSqr;
        isVerticeInside = isVerticeInside ? true : (hit.edges[0].v2 - sCPotP).SqrLength() <= crossSectionRadiusSqr;
        hit.hit = isVerticeInside;
    }
    return hit;
}

inline bool IntersectionSphereWithBox(const Vector3f& center, float radius, const CBoundingBox3D& box) {

    const std::vector<std::pair<EFace, EFace>> faces {{EFace::TOP, EFace::BOTTOM}, {EFace::RIGHT, EFace::LEFT}, {EFace::FRONT, EFace::BACK}};
    for (const auto& face : faces) {
        Vector3f fV1 = box.GetVertex(face.first, 0);
        Vector3f fV2 = box.GetVertex(face.first, 1);
        Vector3f fV3 = box.GetVertex(face.first, 2);

        const Vector3f fNormal = (fV2 - fV1).CrossProduct(fV3 - fV1).Normalized();
        float ft = 0;
        CrossRayWithPlane(center, fNormal, fV1, fV2, fV3, ft  );

        float fClosestToSphere = (center + fNormal * radius * (ft/abs(ft))).Length();

        // Intersecting first plane
        if (std::abs(ft) <= radius) {

        }

        Vector3f sV1 = box.GetVertex(face.first, 0);
        Vector3f sV2 = box.GetVertex(face.first, 1);
        Vector3f sV3 = box.GetVertex(face.first, 2);

        const Vector3f sNormal = (sV2 - sV1).CrossProduct(sV3 - sV1).Normalized();
        float st = 0;
        CrossRayWithPlane(center, sNormal, sV1, sV2, sV3, st  );

        float sClosestToSphere = (center + sNormal * radius * (st/abs(st))).Length();

        // Intersecting second plane
        if (std::abs(st) <= radius) {

        }

        // Totally between planes
        if (ft * st < 0) {

        }
    }



    return false;
}

} // namespace collision_primitives
