#include "BresenhamVoxelization.h"

#include <cmath>

namespace voxelization_demo {

using unboxing_engine::CMeshBuffer;

constexpr float kEpsilon = 0.00001f;

float modulo(float val) {
    if (val < 0) {
        return -val;
    }
    return val;
}

float sign(float val) {
    if (val > 0) {
        return 1;
    }
    if (val == 0) {
        return 0;
    }
    return -1;
}

float truncateToCenter(float value, int dim) {
    float num = (static_cast<int>(value) / dim) * dim;
    return num;
}

SVoxel toVoxel(const Vector3f &position, int dim) {
    return {
        static_cast<int>(truncateToCenter(position.x, dim)),
        static_cast<int>(truncateToCenter(position.y, dim)),
        static_cast<int>(truncateToCenter(position.z, dim)),
    };
}

Vector3f meshVertex(const CMeshBuffer &mesh, unsigned int vertexIndex) {
    const std::size_t offset = static_cast<std::size_t>(vertexIndex) * 3;
    return Vector3f(mesh.vertices[offset], mesh.vertices[offset + 1], mesh.vertices[offset + 2]);
}

bool isInsideTriangle(Vector3f v1, Vector3f v2, Vector3f v3, Vector3f intersection) {
    // Ported from Editor/interfacegl.cpp:isInsideTriangle.
    float area = (v2 - v1).CrossProduct(v3 - v1).Length() * 0.5f;
    if (area <= kEpsilon) {
        return false;
    }

    Vector3f vet1 = intersection - v1;
    Vector3f vet2 = v2 - v1;
    float area1 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = intersection - v1;
    vet2 = v3 - v1;
    float area2 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = intersection - v3;
    vet2 = v2 - v3;
    float area3 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    return area1 + area2 + area3 <= 1.00001f;
}

void addIntersectionSorted(std::vector<Vector3f> &intersections, const Vector3f &point, const Vector3f &rayOrigin) {
    auto distanceKey = static_cast<int>((point - rayOrigin).Length() * 1000000.0f);
    auto it = std::lower_bound(intersections.begin(), intersections.end(), distanceKey, [&rayOrigin](const Vector3f &candidate, int key) {
        return static_cast<int>((candidate - rayOrigin).Length() * 1000000.0f) < key;
    });

    if (it != intersections.end() && (*it - point).Length() <= kEpsilon) {
        return;
    }
    if (it != intersections.begin() && (*(it - 1) - point).Length() <= kEpsilon) {
        return;
    }

    intersections.insert(it, point);
}

// Adapted from Editor/interfacegl.cpp:rayCollisionsWithSceneElementOBJ.
//
// Original role:
// - Fetch GLMmodel through Resources.
// - Apply SceneElement position/scale/quaternion to each triangle.
// - Intersect ray with every triangle plane.
// - Keep intersections inside triangle, sorted by distance.
//
// UnboxingEngine adapter:
// - CMeshBuffer replaces GLMmodel and Resources.
// - The example uses already-positioned mesh vertices, so no SceneElement transform is applied here.
std::vector<Vector3f> rayCollisionsWithMeshBufferPorted(const Vector3f &point, const Vector3f &ray, const CMeshBuffer &mesh, bool firstIntersection) {
    std::vector<Vector3f> intersections;

    for (std::size_t l = 0; l + 2 < mesh.triangles.size(); l += 3) {
        Vector3f vert1 = meshVertex(mesh, mesh.triangles[l]);
        Vector3f vert2 = meshVertex(mesh, mesh.triangles[l + 1]);
        Vector3f vert3 = meshVertex(mesh, mesh.triangles[l + 2]);

        Vector3f normal = (vert2 - vert1).CrossProduct(vert3 - vert1).Normalized();
        Vector3f ponto = vert1;

        float a = normal.x;
        float b = normal.y;
        float c = normal.z;
        float d = -(a * ponto.x + b * ponto.y + c * ponto.z);
        float denominator = a * ray.x + b * ray.y + c * ray.z;

        if (modulo(denominator) <= kEpsilon) {
            continue;
        }

        float t = -(d + a * point.x + b * point.y + c * point.z) / denominator;
        if (t <= kEpsilon) {
            continue;
        }

        Vector3f tempIntersec = point + t * ray;
        if (isInsideTriangle(vert1, vert2, vert3, tempIntersec)) {
            addIntersectionSorted(intersections, tempIntersec, point);
            if (firstIntersection) {
                break;
            }
        }
    }

    return intersections;
}

// Adapted from Editor/interfacegl.cpp:drawLine3D.
//
// Original role:
// - Walk through a regular 3D map grid using the dominant ray axis.
// - Query neighboring map cells and sort hit SceneElements by intersection distance.
//
// UnboxingEngine adapter:
// - Return the traversed voxel coordinates instead of querying Map/SceneElement.
std::vector<SVoxel> drawLine3DPorted(Vector3f p, Vector3f ray, int dim, int safetyLimit) {
    std::vector<SVoxel> traversed;
    Vector3f p2 = p + ray;
    float maior = modulo(p2.x - p.x);
    int dominante = 0;

    if (modulo(p2.y - p.y) > maior) {
        maior = modulo(p2.y - p.y);
        dominante = 1;
    }
    if (modulo(p2.z - p.z) > maior) {
        dominante = 2;
    }

    float x = truncateToCenter(p.x, dim);
    float y = truncateToCenter(p.y, dim);
    float z = truncateToCenter(p.z, dim);

    for (int step = 0; step < safetyLimit; ++step) {
        traversed.push_back({static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)});

        if (dominante == 0) {
            x = x + sign(p2.x - p.x) * dim;
            float t = (x - p.x) / ray.x;
            Vector3f pos = p + t * ray;

            float nextZ = z + sign(p2.z - p.z) * dim;
            float d1z = pos.z - z;
            float d2z = nextZ - z;

            float nextY = y + sign(p2.y - p.y) * dim;
            float d1y = pos.y - y;
            float d2y = nextY - y;

            if (modulo(d2z) <= modulo(d1z)) {
                z = nextZ;
            }
            if (modulo(d2y) <= modulo(d1y)) {
                y = nextY;
            }
        }

        if (dominante == 1) {
            y = y + sign(p2.y - p.y) * dim;
            float t = (y - p.y) / ray.y;
            Vector3f pos = p + t * ray;

            float nextZ = z + sign(p2.z - p.z) * dim;
            float d1z = pos.z - z;
            float d2z = nextZ - z;

            float nextX = x + sign(p2.x - p.x) * dim;
            float d1x = pos.x - x;
            float d2x = nextX - x;

            if (modulo(d2z) <= modulo(d1z)) {
                z = nextZ;
            }
            if (modulo(d2x) <= modulo(d1x)) {
                x = nextX;
            }
        }

        if (dominante == 2) {
            z = z + sign(p2.z - p.z) * dim;
            float t = (z - p.z) / ray.z;
            Vector3f pos = p + t * ray;

            float nextX = x + sign(p2.x - p.x) * dim;
            float d1x = pos.x - x;
            float d2x = nextX - x;

            float nextY = y + sign(p2.y - p.y) * dim;
            float d1y = pos.y - y;
            float d2y = nextY - y;

            if (modulo(d2x) <= modulo(d1x)) {
                x = nextX;
            }
            if (modulo(d2y) <= modulo(d1y)) {
                y = nextY;
            }
        }
    }

    return traversed;
}

// Adapted from Editor/interfacegl.cpp:drawVoxels.
//
// Original role:
// - Walk between two mesh intersections.
// - Add one voxel SceneElement to Map at each visited cell.
//
// UnboxingEngine adapter:
// - Add voxel coordinates to a set instead of allocating SceneElement objects.
void drawVoxelsPorted(Vector3f p, Vector3f pFinal, SVoxelMap &map) {
    Vector3f ray = (pFinal - p).Normalized();
    Vector3f p2 = p + ray;
    float maior = modulo(p2.x - p.x);
    int dominante = 0;

    if (modulo(p2.y - p.y) > maior) {
        maior = modulo(p2.y - p.y);
        dominante = 1;
    }
    if (modulo(p2.z - p.z) > maior) {
        dominante = 2;
    }

    float x = truncateToCenter(p.x, map.dim);
    float y = truncateToCenter(p.y, map.dim);
    float z = truncateToCenter(p.z, map.dim);

    while (modulo(x - pFinal.x) > map.dim || modulo(y - pFinal.y) > map.dim || modulo(z - pFinal.z) > map.dim) {
        map.voxels.insert({static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)});

        if (dominante == 0) {
            x = x + sign(p2.x - p.x) * map.dim;
            float t = (x - p.x) / ray.x;
            Vector3f pos = p + t * ray;

            float nextZ = z + sign(p2.z - p.z) * map.dim;
            float d1z = pos.z - z;
            float d2z = nextZ - z;

            float nextY = y + sign(p2.y - p.y) * map.dim;
            float d1y = pos.y - y;
            float d2y = nextY - y;

            if (modulo(d2z) <= modulo(d1z)) {
                z = nextZ;
            }
            if (modulo(d2y) <= modulo(d1y)) {
                y = nextY;
            }
        }

        if (dominante == 1) {
            y = y + sign(p2.y - p.y) * map.dim;
            float t = (y - p.y) / ray.y;
            Vector3f pos = p + t * ray;

            float nextZ = z + sign(p2.z - p.z) * map.dim;
            float d1z = pos.z - z;
            float d2z = nextZ - z;

            float nextX = x + sign(p2.x - p.x) * map.dim;
            float d1x = pos.x - x;
            float d2x = nextX - x;

            if (modulo(d2z) <= modulo(d1z)) {
                z = nextZ;
            }
            if (modulo(d2x) <= modulo(d1x)) {
                x = nextX;
            }
        }

        if (dominante == 2) {
            z = z + sign(p2.z - p.z) * map.dim;
            float t = (z - p.z) / ray.z;
            Vector3f pos = p + t * ray;

            float nextX = x + sign(p2.x - p.x) * map.dim;
            float d1x = pos.x - x;
            float d2x = nextX - x;

            float nextY = y + sign(p2.y - p.y) * map.dim;
            float d1y = pos.y - y;
            float d2y = nextY - y;

            if (modulo(d2x) <= modulo(d1x)) {
                x = nextX;
            }
            if (modulo(d2y) <= modulo(d1y)) {
                y = nextY;
            }
        }
    }
}

// Adapted from Editor/interfacegl.cpp:createVoxelizedScene.
//
// Original role:
// - Sweep a regular grid with rays.
// - Pair consecutive mesh intersections.
// - Fill voxels between each entry/exit pair.
//
// UnboxingEngine adapter:
// - Uses CMeshBuffer triangles and an SVoxelMap result.
SVoxelizationSummary createVoxelizedScenePorted(const CMeshBuffer &mesh, SVoxelMap &map) {
    SVoxelizationSummary summary;

    int limitI = (map.sizeI - 1) * map.dim / 2;
    int limitJ = (map.sizeJ - 1) * map.dim / 2;

    for (int i = -limitI; i < limitI; i = i + map.dim) {
        for (int j = -limitJ; j < limitJ; j = j + map.dim) {
            ++summary.raysCast;
            std::vector<Vector3f> intersections = rayCollisionsWithMeshBufferPorted(Vector3f(static_cast<float>(i), static_cast<float>(j), -1000.0f), Vector3f(0.0f, 0.0f, 1.0f), mesh);
            summary.intersections += static_cast<int>(intersections.size());

            for (std::size_t k = 0; k + 1 < intersections.size(); k += 2) {
                Vector3f p1 = intersections[k];
                Vector3f p2 = intersections[k + 1];
                drawVoxelsPorted(p1, p2, map);
            }
        }
    }

    summary.filledVoxels = map.voxels.size();
    return summary;
}

} // namespace voxelization_demo
