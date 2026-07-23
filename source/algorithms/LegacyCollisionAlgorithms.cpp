#include "algorithms/LegacyCollisionAlgorithms.h"

#include <algorithm>
#include <cmath>

namespace unboxing_engine::algorithms {

namespace {

constexpr float kEpsilon = 0.00001f;

float absValue(float value) {
    return std::abs(value);
}

Vector3f minCorner(const CBoundingBox3D &box) {
    const Vector3f first = box.getFirst();
    const Vector3f second = box.getSecond();
    return {
        std::min(first.x, second.x),
        std::min(first.y, second.y),
        std::min(first.z, second.z)};
}

Vector3f maxCorner(const CBoundingBox3D &box) {
    const Vector3f first = box.getFirst();
    const Vector3f second = box.getSecond();
    return {
        std::max(first.x, second.x),
        std::max(first.y, second.y),
        std::max(first.z, second.z)};
}

Vector3f applyTransformation(const Matrix4f &transformation, const Vector3f &vertex) {
    return {
        transformation.at(0, 0) * vertex.x + transformation.at(0, 1) * vertex.y + transformation.at(0, 2) * vertex.z + transformation.at(0, 3),
        transformation.at(1, 0) * vertex.x + transformation.at(1, 1) * vertex.y + transformation.at(1, 2) * vertex.z + transformation.at(1, 3),
        transformation.at(2, 0) * vertex.x + transformation.at(2, 1) * vertex.y + transformation.at(2, 2) * vertex.z + transformation.at(2, 3)};
}

void addHitSorted(std::vector<STriangleHit3D> &hits, const STriangleHit3D &hit) {
    auto it = std::lower_bound(hits.begin(), hits.end(), hit.distance, [](const STriangleHit3D &candidate, float distance) {
        return candidate.distance < distance;
    });
    hits.insert(it, hit);
}

} // namespace

// Ported from CCollisionManager::isInsideTriangle in source/collisionutil.cpp.
//
// The original code compares the sum of sub-triangle areas against the full
// triangle area. This keeps that same test and only replaces legacy vector API
// calls with Vector3f::Length().
bool isInsideTrianglePorted(
    const Vector3f &v1,
    const Vector3f &v2,
    const Vector3f &v3,
    const Vector3f &intersection) {

    const float area = (v2 - v1).CrossProduct(v3 - v1).Length() * 0.5f;
    if (area <= kEpsilon) {
        return false;
    }

    Vector3f vet1 = intersection - v1;
    Vector3f vet2 = v2 - v1;
    const float area1 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = intersection - v1;
    vet2 = v3 - v1;
    const float area2 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = intersection - v3;
    vet2 = v2 - v3;
    const float area3 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    return area1 + area2 + area3 <= 1.00001f;
}

bool CalculateRayProjectionToTrianglePlane(const SRay3D &ray, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3, Vector3f &normal, Vector3f &intersection) {
    const Vector3f normalRaw = (v2 - v1).CrossProduct(v3 - v1);
    const float normalLength = normalRaw.Length();
    if (normalLength <= kEpsilon) {
        return false;
    }

    normal = normalRaw / normalLength;
    const Vector3f ponto = v1;

    const float a = normal.x;
    const float b = normal.y;
    const float c = normal.z;
    const float d = -(a * ponto.x + b * ponto.y + c * ponto.z);
    const float denominator = a * ray.direction.x + b * ray.direction.y + c * ray.direction.z;
    if (absValue(denominator) <= kEpsilon) {
        return false;
    }

    float t = -(d + a * ray.origin.x + b * ray.origin.y + c * ray.origin.z) / denominator;
    if (t < 0.0f) {
        return false;
    }

    intersection = ray.origin + t * ray.direction;
    return true;
}
// Ported from CCollisionManager::rayWithTriangle in source/collisionutil.cpp.
//
// The plane intersection math is intentionally kept close to the old code.
// The added denominator and forward-ray guards prevent undefined division and
// hits behind the ray origin.
std::optional<STriangleHit3D> rayWithTrianglePorted(
    const SRay3D &ray,
    const Vector3f &v1,
    const Vector3f &v2,
    const Vector3f &v3,
    unsigned int triangleIndex) {

    Vector3f normal;
    Vector3f intersection;

    if (!CalculateRayProjectionToTrianglePlane(ray, v1, v2, v3, normal, intersection)) {
        return std::nullopt;
    }
    if (!isInsideTrianglePorted(v1, v2, v3, intersection)) {
        return std::nullopt;
    }

    STriangleHit3D hit;
    hit.hit = true;
    hit.intersection = intersection;
    hit.normal = normal;
    hit.distance = (intersection - ray.origin).Length();
    hit.triangleIndex = triangleIndex;
    return hit;
}

std::optional<Vector3f> meshVertexPorted(
    const CMeshBuffer &mesh,
    unsigned int vertexIndex,
    const Matrix4f *transformation) {

    const std::size_t offset = static_cast<std::size_t>(vertexIndex) * 3;
    if (offset + 2 >= mesh.vertices.size()) {
        return std::nullopt;
    }

    Vector3f vertex{
        mesh.vertices[offset],
        mesh.vertices[offset + 1],
        mesh.vertices[offset + 2]};

    if (transformation != nullptr) {
        vertex = applyTransformation(*transformation, vertex);
    }

    return vertex;
}

// Ported from CCollisionManager::rayWithObject in source/collisionutil.cpp.
//
// The old function traversed an octree and sorted intersections. This raw mesh
// variant keeps the triangle-plane loop and ordered hits, leaving octree
// acceleration to LegacyOctreeAlgorithms.
std::vector<STriangleHit3D> rayWithMeshBufferPorted(
    const CMeshBuffer &mesh,
    const SRay3D &ray,
    bool firstIntersection,
    const Matrix4f *transformation) {

    std::vector<STriangleHit3D> hits;

    for (std::size_t triangleOffset = 0; triangleOffset + 2 < mesh.triangles.size(); triangleOffset += 3) {
        const auto v1 = meshVertexPorted(mesh, mesh.triangles[triangleOffset], transformation);
        const auto v2 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 1], transformation);
        const auto v3 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 2], transformation);
        if (!v1 || !v2 || !v3) {
            continue;
        }

        auto hit = rayWithTrianglePorted(ray, *v1, *v2, *v3, static_cast<unsigned int>(triangleOffset / 3));
        if (hit) {
            addHitSorted(hits, *hit);
        }
    }

    if (firstIntersection && hits.size() > 1) {
        hits.resize(1);
    }

    return hits;
}

// Ported from CCollisionManager::sphereWithTriangle in source/collisionutil.cpp.
//
// This intentionally preserves the original scope: project the sphere center
// onto the triangle plane and accept the hit only if the projection is inside
// both the sphere and the triangle.
std::optional<STriangleHit3D> sphereWithTrianglePorted(
    const SSphere3D &sphere,
    const Vector3f &v1,
    const Vector3f &v2,
    const Vector3f &v3,
    unsigned int triangleIndex) {

    const Vector3f normalRaw = (v2 - v1).CrossProduct(v3 - v1);
    const float normalLength = normalRaw.Length();
    if (normalLength <= kEpsilon) {
        return std::nullopt;
    }

    const Vector3f normal = normalRaw / normalLength;
    const float a = normal.x;
    const float b = normal.y;
    const float c = normal.z;
    const float d = -a * v1.x - b * v1.y - c * v1.z;

    const Vector3f p = sphere.center;
    const float lambda = (-d - p.DotProduct(normal)) / normal.DotProduct(normal);
    const Vector3f projection = p + normal * lambda;
    const float distance = (projection - sphere.center).Length();

    if (distance > sphere.radius || !isInsideTrianglePorted(v1, v2, v3, projection)) {
        return std::nullopt;
    }

    STriangleHit3D hit;
    hit.hit = true;
    hit.intersection = projection;
    hit.normal = normal;
    hit.distance = distance;
    hit.triangleIndex = triangleIndex;
    return hit;
}

// Axis-aligned replacement for the old BoundingBox::isInside helper.
bool isInsideBoundingBoxPorted(const CBoundingBox3D &box, const Vector3f &point) {
    const Vector3f min = minCorner(box);
    const Vector3f max = maxCorner(box);

    return point.x >= min.x - kEpsilon && point.x <= max.x + kEpsilon &&
           point.y >= min.y - kEpsilon && point.y <= max.y + kEpsilon &&
           point.z >= min.z - kEpsilon && point.z <= max.z + kEpsilon;
}

// Current-type bounding-box ray helper for the old intersectionWithRay calls.
std::optional<float> rayWithBoundingBoxPorted(const CBoundingBox3D &box, const SRay3D &ray) {
    const Vector3f min = minCorner(box);
    const Vector3f max = maxCorner(box);

    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    auto updateRange = [&](float origin, float direction, float axisMin, float axisMax) {
        if (absValue(direction) <= kEpsilon) {
            return origin >= axisMin - kEpsilon && origin <= axisMax + kEpsilon;
        }

        float t1 = (axisMin - origin) / direction;
        float t2 = (axisMax - origin) / direction;
        if (t1 > t2) {
            std::swap(t1, t2);
        }

        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        return tMin <= tMax + kEpsilon;
    };

    if (!updateRange(ray.origin.x, ray.direction.x, min.x, max.x)) {
        return std::nullopt;
    }
    if (!updateRange(ray.origin.y, ray.direction.y, min.y, max.y)) {
        return std::nullopt;
    }
    if (!updateRange(ray.origin.z, ray.direction.z, min.z, max.z)) {
        return std::nullopt;
    }

    return tMin * ray.direction.Length();
}

// Ported from BoundingBox::intersectionWithSphere in source/collisionutil.cpp.
bool sphereWithBoundingBoxPorted(const CBoundingBox3D &box, const SSphere3D &sphere) {
    if (isInsideBoundingBoxPorted(box, sphere.center)) {
        return true;
    }

    const auto vertices = box.GetVertices();
    const auto triangles = box.GetTriangles();
    CMeshBuffer mesh;
    mesh.vertices = vertices;
    mesh.triangles = triangles;

    for (std::size_t triangleOffset = 0; triangleOffset + 2 < mesh.triangles.size(); triangleOffset += 3) {
        const auto v1 = meshVertexPorted(mesh, mesh.triangles[triangleOffset]);
        const auto v2 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 1]);
        const auto v3 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 2]);
        if (v1 && v2 && v3 && sphereWithTrianglePorted(sphere, *v1, *v2, *v3)) {
            return true;
        }
    }

    return false;
}

} // namespace unboxing_engine::algorithms
