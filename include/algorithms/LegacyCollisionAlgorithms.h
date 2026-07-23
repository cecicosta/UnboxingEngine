#pragma once

#include "BoundingBox.h"
#include "Matrix.h"
#include "MeshBuffer.h"
#include "UVector.h"

#include <limits>
#include <optional>
#include <vector>

namespace unboxing_engine::algorithms {

struct SRay3D {
    Vector3f origin;
    Vector3f direction;
};

struct SSphere3D {
    Vector3f center;
    float radius = 0.0f;
};

struct STriangleHit3D {
    bool hit = false;
    Vector3f intersection;
    Vector3f normal;
    float distance = 0.0f;
    unsigned int triangleIndex = std::numeric_limits<unsigned int>::max();
};

bool isInsideTrianglePorted(
    const Vector3f &v1,
    const Vector3f &v2,
    const Vector3f &v3,
    const Vector3f &intersection);

bool CalculateRayProjectionToTrianglePlane(const SRay3D &ray, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3, Vector3f &normal, Vector3f &intersection);

std::optional<STriangleHit3D> rayWithTrianglePorted(
    const SRay3D &ray,
    const Vector3f &v1,
    const Vector3f &v2,
    const Vector3f &v3,
    unsigned int triangleIndex = std::numeric_limits<unsigned int>::max());

std::vector<STriangleHit3D> rayWithMeshBufferPorted(
    const CMeshBuffer &mesh,
    const SRay3D &ray,
    bool firstIntersection = false,
    const Matrix4f *transformation = nullptr);

std::optional<STriangleHit3D> sphereWithTrianglePorted(
    const SSphere3D &sphere,
    const Vector3f &v1,
    const Vector3f &v2,
    const Vector3f &v3,
    unsigned int triangleIndex = std::numeric_limits<unsigned int>::max());

bool isInsideBoundingBoxPorted(const CBoundingBox3D &box, const Vector3f &point);

std::optional<float> rayWithBoundingBoxPorted(const CBoundingBox3D &box, const SRay3D &ray);

bool sphereWithBoundingBoxPorted(const CBoundingBox3D &box, const SSphere3D &sphere);

std::optional<Vector3f> meshVertexPorted(
    const CMeshBuffer &mesh,
    unsigned int vertexIndex,
    const Matrix4f *transformation = nullptr);

} // namespace unboxing_engine::algorithms
