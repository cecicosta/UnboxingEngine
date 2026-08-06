#include "MeshBuffer.h"
#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "algorithms/LegacyCollisionAlgorithms.h"
#include "algorithms/LegacyOctreeAlgorithms.h"
#include "internal_components/RenderComponent.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace unboxing_engine;
using namespace unboxing_engine::algorithms;

namespace {

constexpr float kEpsilon = 0.00001f;

struct SVoxel {
    int x = 0;
    int y = 0;
    int z = 0;

    bool operator<(const SVoxel &other) const {
        if (x != other.x) {
            return x < other.x;
        }
        if (y != other.y) {
            return y < other.y;
        }
        return z < other.z;
    }

    bool operator==(const SVoxel &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct SVoxelMap {
    int dim = 1;
    int sizeI = 6;
    int sizeJ = 6;
    std::set<SVoxel> voxels;
};

struct SOctreeInspection {
    std::size_t nodes = 0;
    std::size_t leaves = 0;
    std::size_t storedTriangleReferences = 0;
    unsigned int maxDepth = 0;
};

struct SVoxelizationSummary {
    int raysCast = 0;
    int intersections = 0;
    std::size_t filledVoxels = 0;
    std::size_t meshTriangles = 0;
    std::size_t bruteForceTriangleTests = 0;
    std::size_t octreeCandidateTriangleTests = 0;
    SOctreeInspection octree;
};

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

std::vector<Vector3f> uniqueIntersectionsFromHits(const std::vector<STriangleHit3D> &hits, const Vector3f &rayOrigin) {
    std::vector<Vector3f> intersections;
    for (const STriangleHit3D &hit : hits) {
        addIntersectionSorted(intersections, hit.intersection, rayOrigin);
    }
    return intersections;
}

// Adapted from Editor/interfacegl.cpp:drawLine3D.
//
// This remains the historical voxel-grid traversal. The optimization in this
// example is only in the mesh intersection stage.
std::vector<SVoxel> drawLine3DPorted(Vector3f p, Vector3f ray, int dim, int safetyLimit = 1024) {
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

void inspectOctreeNode(const SMeshOctreeNode &node, unsigned int depth, SOctreeInspection &inspection) {
    ++inspection.nodes;
    inspection.maxDepth = std::max(inspection.maxDepth, depth);
    inspection.storedTriangleReferences += node.triangleIndices.size();

    bool hasChild = false;
    for (const auto &child : node.children) {
        if (child) {
            hasChild = true;
            inspectOctreeNode(*child, depth + 1, inspection);
        }
    }

    if (!hasChild) {
        ++inspection.leaves;
    }
}

SOctreeInspection inspectOctree(const SMeshOctree &octree) {
    SOctreeInspection inspection;
    if (octree.root) {
        inspectOctreeNode(*octree.root, 0, inspection);
    }
    return inspection;
}

// Optimized adapter for the old voxelization raycast stage.
//
// Historical core:
// - Editor/interfacegl.cpp:createVoxelizedScene still sweeps regular Z rays.
// - Editor/interfacegl.cpp:drawVoxels still fills cells between paired hits.
//
// New engine optimization:
// - Legacy/octree.cpp style octree returns candidate triangle indices.
// - rayWithMeshOctreePorted combines that query with the source/collisionutil.cpp
//   style rayWithTrianglePorted collision test.
std::vector<Vector3f> rayCollisionsWithMeshOctreePorted(
    const Vector3f &point,
    const Vector3f &ray,
    const CMeshBuffer &mesh,
    const SMeshOctree &octree,
    std::size_t &candidateTriangleTests) {

    const SRay3D ray3D{point, ray};
    const std::vector<STriangleHit3D> hits = rayWithMeshOctreePorted(mesh, octree, ray3D, &candidateTriangleTests);
    return uniqueIntersectionsFromHits(hits, point);
}

// Adapted from Editor/interfacegl.cpp:createVoxelizedScene.
SVoxelizationSummary createVoxelizedSceneOctreePorted(const CMeshBuffer &mesh, const SMeshOctree &octree, SVoxelMap &map) {
    SVoxelizationSummary summary;
    summary.meshTriangles = mesh.triangles.size() / 3;
    summary.octree = inspectOctree(octree);

    int limitI = (map.sizeI - 1) * map.dim / 2;
    int limitJ = (map.sizeJ - 1) * map.dim / 2;

    for (int i = -limitI; i < limitI; i = i + map.dim) {
        for (int j = -limitJ; j < limitJ; j = j + map.dim) {
            ++summary.raysCast;
            summary.bruteForceTriangleTests += summary.meshTriangles;

            std::vector<Vector3f> intersections = rayCollisionsWithMeshOctreePorted(
                Vector3f(static_cast<float>(i), static_cast<float>(j), -1000.0f),
                Vector3f(0.0f, 0.0f, 1.0f),
                mesh,
                octree,
                summary.octreeCandidateTriangleTests);
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

std::unique_ptr<CMeshBuffer> makeTranslatedCubeMesh(float minCoord, float maxCoord) {
    std::unique_ptr<CMeshBuffer> cube(primitive_utils::Cube());
    for (std::size_t i = 0; i + 2 < cube->vertices.size(); i += 3) {
        cube->vertices[i] = cube->vertices[i] * (maxCoord - minCoord) + (maxCoord + minCoord) * 0.5f;
        cube->vertices[i + 1] = cube->vertices[i + 1] * (maxCoord - minCoord) + (maxCoord + minCoord) * 0.5f;
        cube->vertices[i + 2] = cube->vertices[i + 2] * (maxCoord - minCoord) + (maxCoord + minCoord) * 0.5f;
    }
    cube->boundingBox = CBoundingBox3D(Vector3f(minCoord, minCoord, minCoord), Vector3f(maxCoord, maxCoord, maxCoord));
    return cube;
}

std::unique_ptr<CMeshBuffer> makeRayMesh(const Vector3f &start, const Vector3f &end) {
    auto mesh = primitive_utils::Lines(start, end);
    mesh->boundingBox = CBoundingBox3D(start, end);
    return mesh;
}

std::unique_ptr<CMeshBuffer> makeVoxelMesh(const SVoxel &voxel, int dim, float scaleFactor = 0.82f) {
    std::unique_ptr<CMeshBuffer> mesh(primitive_utils::Cube());
    const float scale = static_cast<float>(dim) * scaleFactor;
    const float centerX = static_cast<float>(voxel.x) + static_cast<float>(dim) * 0.5f;
    const float centerY = static_cast<float>(voxel.y) + static_cast<float>(dim) * 0.5f;
    const float centerZ = static_cast<float>(voxel.z) + static_cast<float>(dim) * 0.5f;

    for (std::size_t i = 0; i + 2 < mesh->vertices.size(); i += 3) {
        mesh->vertices[i] = mesh->vertices[i] * scale + centerX;
        mesh->vertices[i + 1] = mesh->vertices[i + 1] * scale + centerY;
        mesh->vertices[i + 2] = mesh->vertices[i + 2] * scale + centerZ;
    }
    mesh->boundingBox = CBoundingBox3D(
        Vector3f(centerX - scale * 0.5f, centerY - scale * 0.5f, centerZ - scale * 0.5f),
        Vector3f(centerX + scale * 0.5f, centerY + scale * 0.5f, centerZ + scale * 0.5f));
    return mesh;
}

SMaterial material(float r, float g, float b, float a) {
    SMaterial mat{};
    mat.materialDif[0] = r;
    mat.materialDif[1] = g;
    mat.materialDif[2] = b;
    mat.materialDif[3] = a;
    mat.enable = true;
    return mat;
}

class CMeshSceneObject : public CSceneComposite {
public:
    CMeshSceneObject(std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, EPolygonMode polygonMode = EPolygonMode::Line)
        : mMesh(std::move(mesh)) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        render->SetMaterial(mat);
        render->SetPolygonMode(polygonMode);
        AddComponent<IRenderComponent>(std::move(render));
    }

private:
    std::unique_ptr<CMeshBuffer> mMesh;
};

bool expect(bool condition, const std::string &message) {
    if (!condition) {
        std::cerr << "Self-test failed: " << message << "\n";
        return false;
    }
    return true;
}

bool runSelfTest() {
    bool ok = true;

    std::vector<SVoxel> line = drawLine3DPorted(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), 1, 5);
    ok = expect(line.size() == 5, "drawLine3DPorted should return five sampled cells with safetyLimit=5") && ok;
    ok = expect(line.front() == SVoxel{0, 0, 0}, "drawLine3DPorted start cell mismatch") && ok;
    ok = expect(line.back() == SVoxel{4, 0, 0}, "drawLine3DPorted final sampled cell mismatch") && ok;

    std::unique_ptr<CMeshBuffer> cube = makeTranslatedCubeMesh(-2.0f, 2.0f);
    SMeshOctree cubeOctree = buildMeshOctreePorted(*cube, 2, 4);

    std::size_t cubeCandidateTests = 0;
    std::vector<Vector3f> cubeHits = rayCollisionsWithMeshOctreePorted(
        Vector3f(0.0f, 0.0f, -1000.0f),
        Vector3f(0.0f, 0.0f, 1.0f),
        *cube,
        cubeOctree,
        cubeCandidateTests);
    ok = expect(cubeHits.size() == 2, "central octree ray should find two unique cube intersections") && ok;

    SVoxelMap cubeMap;
    SVoxelizationSummary cubeSummary = createVoxelizedSceneOctreePorted(*cube, cubeOctree, cubeMap);
    ok = expect(cubeSummary.raysCast == 16, "default map sweep should cast 16 rays") && ok;
    ok = expect(cubeSummary.intersections == 32, "default cube sweep should find 32 unique intersections") && ok;
    ok = expect(cubeSummary.filledVoxels == 48, "octree fill should produce 48 voxel cells for the default cube") && ok;
    ok = expect(cubeMap.voxels.count({0, 0, -2}) == 1, "expected central lower voxel missing") && ok;
    ok = expect(cubeMap.voxels.count({0, 0, -1}) == 1, "expected central middle voxel missing") && ok;
    ok = expect(cubeMap.voxels.count({0, 0, 0}) == 1, "expected central upper sampled voxel missing") && ok;
    ok = expect(cubeMap.voxels.count({0, 0, 1}) == 0, "ported fill should stop one voxel before the exit point") && ok;
    ok = expect(cubeMap.voxels.count({3, 0, 0}) == 0, "outside voxel should not be filled") && ok;

    std::unique_ptr<CMeshBuffer> torus = primitive_utils::Torus(4.0f, 1.25f, 24, 8);
    SMeshOctree torusOctree = buildMeshOctreePorted(*torus, 8, 6);

    std::vector<STriangleHit3D> bruteHits = rayWithMeshBufferPorted(
        *torus,
        {{4.0f, 0.0f, -1000.0f}, {0.0f, 0.0f, 1.0f}});
    std::vector<Vector3f> bruteIntersections = uniqueIntersectionsFromHits(bruteHits, {4.0f, 0.0f, -1000.0f});

    std::size_t torusCandidateTests = 0;
    std::vector<Vector3f> octreeIntersections = rayCollisionsWithMeshOctreePorted(
        Vector3f(4.0f, 0.0f, -1000.0f),
        Vector3f(0.0f, 0.0f, 1.0f),
        *torus,
        torusOctree,
        torusCandidateTests);
    ok = expect(octreeIntersections.size() == bruteIntersections.size(), "octree and brute-force ray should return the same unique torus intersections") && ok;
    ok = expect(torusCandidateTests < torus->triangles.size() / 3, "octree ray should test fewer triangles than brute force") && ok;

    SVoxelMap torusMap;
    torusMap.sizeI = 12;
    torusMap.sizeJ = 12;
    SVoxelizationSummary torusSummary = createVoxelizedSceneOctreePorted(*torus, torusOctree, torusMap);
    ok = expect(torusSummary.intersections > 0, "torus sweep should produce mesh intersections") && ok;
    ok = expect(torusSummary.filledVoxels > 0, "torus sweep should fill voxels") && ok;
    ok = expect(torusSummary.octreeCandidateTriangleTests < torusSummary.bruteForceTriangleTests, "octree sweep should reduce triangle tests") && ok;

    return ok;
}

void printSummary(const SVoxelizationSummary &summary) {
    float reduction = 0.0f;
    if (summary.bruteForceTriangleTests > 0) {
        reduction = 100.0f * (1.0f - static_cast<float>(summary.octreeCandidateTriangleTests) / static_cast<float>(summary.bruteForceTriangleTests));
    }

    std::cout << "Rays cast: " << summary.raysCast << "\n";
    std::cout << "Mesh triangles: " << summary.meshTriangles << "\n";
    std::cout << "Mesh intersections: " << summary.intersections << "\n";
    std::cout << "Filled voxels: " << summary.filledVoxels << "\n";
    std::cout << "Brute-force triangle tests baseline: " << summary.bruteForceTriangleTests << "\n";
    std::cout << "Octree candidate triangle tests: " << summary.octreeCandidateTriangleTests << "\n";
    std::cout << "Triangle-test reduction: " << std::fixed << std::setprecision(2) << reduction << "%\n";
    std::cout << "Octree nodes: " << summary.octree.nodes << "\n";
    std::cout << "Octree leaves: " << summary.octree.leaves << "\n";
    std::cout << "Octree max depth: " << summary.octree.maxDepth << "\n";
    std::cout << "Octree stored triangle references: " << summary.octree.storedTriangleReferences << "\n";
}

void runVisualExample(bool enterMainLoop) {
    std::unique_ptr<CMeshBuffer> sourceMesh = primitive_utils::Torus(9.0f, 5.0f);
    SMeshOctree sourceOctree = buildMeshOctreePorted(*sourceMesh, 24, 7);

    SVoxelMap map;
    map.dim = 1;
    map.sizeI = 30;
    map.sizeJ = 30;
    SVoxelizationSummary summary = createVoxelizedSceneOctreePorted(*sourceMesh, sourceOctree, map);
    printSummary(summary);

    CCore engine(1200, 720, 32);
    Camera camera(1200, 720, 70.0f, 1.0f, 1.0f);
    camera.SetOrthographicIsometric(24.0f, 15.0f, 30.0f, Vector3f(0.0f, 0.0f, 0.0f), -45.0f, 35.0f);
    engine.SetCamera(camera);
    engine.Start();

    CSceneComposite sourceGroup;
    CSceneComposite voxelGroup;
    sourceGroup.SetPosition(Vector3f(-3.8f, -3.8f, 0.0f));
    voxelGroup.SetPosition(Vector3f(3.8f, 3.8f, 0.0f));

    std::vector<std::unique_ptr<CMeshSceneObject>> sceneObjects;
    auto addSceneObject = [&engine, &sceneObjects](std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, EPolygonMode polygonMode, CSceneNode *parent, const Vector3f &scale) {
        auto sceneObject = std::make_unique<CMeshSceneObject>(std::move(mesh), mat, polygonMode);
        sceneObject->SetParent(parent);
        sceneObject->SetScale(scale);
        sceneObjects.push_back(std::move(sceneObject));
        engine.RegisterSceneElement(*sceneObjects.back());
    };

    addSceneObject(std::move(sourceMesh), material(0.35f, 0.24f, 0.23f, 1.0f), EPolygonMode::Fill, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f) / 3);
    addSceneObject(primitive_utils::Torus(9.0f, 5.0f), material(0.85f, 0.72f, 0.64f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.012f, 1.012f, 1.012f) / 3);

    addSceneObject(makeRayMesh(Vector3f(0.0f, 0.0f, -2.2f), Vector3f(0.0f, 0.0f, 2.2f)), material(0.95f, 0.78f, 0.18f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f) / 3);
    addSceneObject(makeRayMesh(Vector3f(3.0f, 0.0f, -2.2f), Vector3f(3.0f, 0.0f, 2.2f)), material(0.95f, 0.78f, 0.18f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f) / 3);
    addSceneObject(makeRayMesh(Vector3f(2.0f, 2.0f, -2.2f), Vector3f(2.0f, 2.0f, 2.2f)), material(0.95f, 0.78f, 0.18f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f) / 3);

    for (const SVoxel &voxel : map.voxels) {
        addSceneObject(makeVoxelMesh(voxel, map.dim, 0.82f), material(0.10f, 0.43f, 0.98f, 1.0f), EPolygonMode::Fill, &voxelGroup, Vector3f(1.0f, 1.0f, 1.0f) / 3);
        addSceneObject(makeVoxelMesh(voxel, map.dim, 0.90f), material(0.02f, 0.08f, 0.18f, 1.0f), EPolygonMode::Line, &voxelGroup, Vector3f(1.0f, 1.0f, 1.0f) / 3);
    }

    if (enterMainLoop) {
        engine.Run();
    } else {
        for (const auto &sceneObject : sceneObjects) {
            if (auto render = sceneObject->GetComponent<IRenderComponent>()) {
                render->OnRender();
            }
        }
    }

    for (auto it = sceneObjects.rbegin(); it != sceneObjects.rend(); ++it) {
        engine.UnregisterSceneElement(**it);
    }
    sceneObjects.clear();
    engine.Release();
}

} // namespace

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--self-test") {
            if (!runSelfTest()) {
                return 1;
            }
            std::cout << "Self-test passed.\n";
            return 0;
        }
        if (arg == "--render-smoke-test") {
            runVisualExample(false);
            return 0;
        }
        if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--self-test] [--render-smoke-test]\n";
            return 0;
        }

        std::cerr << "Unknown argument: " << arg << "\n";
        std::cerr << "Usage: " << argv[0] << " [--self-test] [--render-smoke-test]\n";
        return 2;
    }

    runVisualExample(true);
    return 0;
}
