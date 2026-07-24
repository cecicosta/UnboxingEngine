#include "CoreEvents.h"
#include "MeshBuffer.h"
#include "RadiosityCornellBox.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "algorithms/LegacyCollisionAlgorithms.h"
#include "algorithms/LegacyOctreeAlgorithms.h"
#include "internal_components/RenderComponent.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace unboxing_engine;
using namespace unboxing_engine::algorithms;

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kEpsilon = 0.00001f;
constexpr float kOcclusionEpsilon = 0.001f;
constexpr float kFormFactorContributionThreshold = 0.00025f;

using radiosity_demo::SCornellBoxConfig;
using radiosity_demo::SCornellMeshElement;
using radiosity_demo::SRgb;
using radiosity_demo::makeCornellBoxMeshElements;

struct SViewedPatch {
    int id = -1;
    float formFactor = 0.0f;
};

struct SRadiosityPatch {
    int id = -1;
    std::string name;
    std::array<Vector3f, 3> vertex;
    Vector3f centroid;
    Vector3f normal;
    float area = 0.0f;
    float coverage = 0.0f;
    SRgb emissivity;
    SRgb radiosity;
    SRgb reflectance;
    std::vector<SViewedPatch> patches;
};

struct SVisibilityCollisionScene {
    CMeshBuffer mesh;
    std::vector<int> patchIdByTriangleIndex;
    SMeshOctree octree;
};

struct SRadiositySummary {
    int patchCount = 0;
    int nonZeroFormFactors = 0;
    int iterations = 0;
    int normalVisiblePairs = 0;
    int directOccludedPairs = 0;
    std::size_t formFactorSkippedPairs = 0;
    std::size_t occlusionPairsTested = 0;
    float maxFormFactor = 0.0f;
    float maxRowSum = 0.0f;
    float maxDelta = 0.0f;
    std::size_t bruteForceOcclusionTriangleTests = 0;
    std::size_t octreeCandidateTriangleTests = 0;
};

SRgb rgb(float r, float g, float b) {
    return {r, g, b};
}

SRgb operator+(const SRgb &a, const SRgb &b) {
    return {a.r + b.r, a.g + b.g, a.b + b.b};
}

SRgb &operator+=(SRgb &a, const SRgb &b) {
    a.r += b.r;
    a.g += b.g;
    a.b += b.b;
    return a;
}

SRgb operator*(const SRgb &a, float value) {
    return {a.r * value, a.g * value, a.b * value};
}

SRgb multiply(const SRgb &a, const SRgb &b) {
    return {a.r * b.r, a.g * b.g, a.b * b.b};
}

float maxChannel(const SRgb &color) {
    return std::max({color.r, color.g, color.b});
}

float maxAbsDifference(const SRgb &a, const SRgb &b) {
    return std::max({std::fabs(a.r - b.r), std::fabs(a.g - b.g), std::fabs(a.b - b.b)});
}

float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

SRgb toneMap(const SRgb &color, float exposure = 0.55f) {
    return {
        clamp01(1.0f - std::exp(-std::max(0.0f, color.r) * exposure)),
        clamp01(1.0f - std::exp(-std::max(0.0f, color.g) * exposure)),
        clamp01(1.0f - std::exp(-std::max(0.0f, color.b) * exposure)),
    };
}

SMaterial materialFromColor(const SRgb &color) {
    SMaterial mat{};
    mat.materialDif[0] = clamp01(color.r);
    mat.materialDif[1] = clamp01(color.g);
    mat.materialDif[2] = clamp01(color.b);
    mat.materialDif[3] = 1.0f;
    mat.enable = true;
    return mat;
}

float triangleArea2D(float x0, float y0, float x1, float y1, float x2, float y2) {
    return std::fabs((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) * 0.5f;
}

Vector3f minVector(const Vector3f &a, const Vector3f &b) {
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

Vector3f maxVector(const Vector3f &a, const Vector3f &b) {
    return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

SCornellBoxConfig makeOctreeDemoCornellConfig() {
    SCornellBoxConfig config;
    config.subdivision.floorColumns = 8;
    config.subdivision.floorRows = 8;
    config.subdivision.wallColumns = 8;
    config.subdivision.wallRows = 7;
    config.subdivision.ceilingColumns = 8;
    config.subdivision.ceilingRows = 8;
    config.subdivision.lightColumns = 4;
    config.subdivision.lightRows = 4;
    config.lightEmissionScale = 2.8f;
    config.torus.enabled = true;
    config.torus.majorRadius = 0.52f;
    config.torus.minorRadius = 0.20f;
    config.torus.majorSegments = 12;
    config.torus.minorSegments = 4;
    config.torus.center = Vector3f(0.15f, -0.10f, 1.35f);
    return config;
}

SCornellBoxConfig makeSelfTestCornellConfig() {
    SCornellBoxConfig config;
    config.subdivision.floorColumns = 5;
    config.subdivision.floorRows = 5;
    config.subdivision.wallColumns = 5;
    config.subdivision.wallRows = 4;
    config.subdivision.ceilingColumns = 5;
    config.subdivision.ceilingRows = 5;
    config.subdivision.lightColumns = 2;
    config.subdivision.lightRows = 2;
    config.lightEmissionScale = 2.8f;
    config.torus.enabled = true;
    config.torus.majorRadius = 0.52f;
    config.torus.minorRadius = 0.20f;
    config.torus.majorSegments = 8;
    config.torus.minorSegments = 4;
    config.torus.center = Vector3f(0.15f, -0.10f, 1.35f);
    return config;
}

SCornellBoxConfig makeRenderSmokeCornellConfig() {
    SCornellBoxConfig config = makeSelfTestCornellConfig();
    config.torus.majorSegments = 6;
    config.torus.minorSegments = 4;
    return config;
}

// Adapted from Editor/radiosity/original/patch.cpp:patch::patch.
SRadiosityPatch makePatchPorted(
    int id,
    const std::string &name,
    const Vector3f &v0,
    const Vector3f &v1,
    const Vector3f &v2,
    const SRgb &reflectance,
    const SRgb &emissivity = {}) {
    SRadiosityPatch patch;
    patch.id = id;
    patch.name = name;
    patch.vertex = {v0, v1, v2};
    patch.centroid = (v0 + v1 + v2) / 3.0f;

    Vector3f edge1 = v1 - v0;
    Vector3f edge2 = v2 - v0;
    Vector3f normal = edge1.CrossProduct(edge2);
    patch.area = normal.Length() * 0.5f;
    patch.normal = patch.area > kEpsilon ? normal.Normalized() : Vector3f(0.0f, 0.0f, 0.0f);

    patch.reflectance = reflectance;
    patch.emissivity = emissivity;
    patch.radiosity = emissivity;
    return patch;
}

// Adapted from Editor/radiosity/original/hemiesfera.cpp:hemiEsfera::isVisible.
// fixed bug causing ceiling to not be visible and simplified the logic
bool isVisiblePorted(const SRadiosityPatch &receiver, const SRadiosityPatch &source) {
    auto dist = source.centroid - receiver.centroid;
    auto distProjection = dist.DotProduct(receiver.normal);
    bool isVisible = distProjection > kEpsilon;

    auto normalProjection = receiver.normal.DotProduct(source.normal);
    isVisible = isVisible || normalProjection < 0;

    return isVisible;
}

// Adapted from Editor/radiosity/original/hemiesfera.cpp:hemiEsfera::intersectionWithRay.
bool intersectHemisphereRayPorted(const Vector3f &center, float radius, const Vector3f &p1, const Vector3f &p2, Vector3f &intersection) {
    Vector3f ray = p2 - p1;
    if (ray.Length() <= kEpsilon) {
        return false;
    }

    Vector3f p = center - p1;
    float m = ray.DotProduct(ray);
    float n = -2.0f * ray.DotProduct(p);
    float k = p.DotProduct(p) - radius * radius;
    float delta = n * n - 4.0f * m * k;

    if (delta < 0.0f) {
        return false;
    }

    float sqrtDelta = std::sqrt(delta);
    float t1 = (-n + sqrtDelta) / (2.0f * m);
    float t2 = (-n - sqrtDelta) / (2.0f * m);
    float t = 0.0f;

    if (t1 > kEpsilon && t2 > kEpsilon) {
        t = std::min(t1, t2);
    } else if (t1 > kEpsilon) {
        t = t1;
    } else if (t2 > kEpsilon) {
        t = t2;
    } else {
        return false;
    }

    intersection = p1 + ray * t;
    return true;
}

void createLocalOrthoAxes(const Vector3f &ZRef, Vector3f &outputX, Vector3f &outputY) {
    // Choose an arbitrary, non co-linear direction, in relation to the choose Z
    Vector3f reference = std::fabs(ZRef.z) < 0.9f ? Vector3f(0.0f, 0.0f, 1.0f) : Vector3f(0.0f, 1.0f, 0.0f);
    outputX = reference.CrossProduct(ZRef).Normalized();
    outputY = ZRef.CrossProduct(outputX).Normalized();
}

// Adapted from Editor/radiosity/original/hemiesfera.cpp:hemiEsfera::calculateFormFactor.
float calculateProjectedFormFactorPorted(const SRadiosityPatch &receiver, const SRadiosityPatch &source) {
    if (receiver.id == source.id || receiver.area <= kEpsilon || source.area <= kEpsilon) {
        return 0.0f;
    }

    Vector3f axisX;
    Vector3f axisY;
    createLocalOrthoAxes(receiver.normal, axisX, axisY);

    std::array<float, 3> projectedX{};
    std::array<float, 3> projectedY{};
    for (std::size_t i = 0; i < source.vertex.size(); ++i) {
        Vector3f projected;
        if (!intersectHemisphereRayPorted(receiver.centroid, 1.0f, receiver.centroid, source.vertex[i], projected)) {
            return 0.0f;
        }

        Vector3f offset = projected - receiver.centroid;
        if (offset.DotProduct(receiver.normal) <= kEpsilon) {
            return 0.0f;
        }

        projectedX[i] = offset.DotProduct(axisX);
        projectedY[i] = offset.DotProduct(axisY);
    }

    float projectedArea = triangleArea2D(
        projectedX[0],
        projectedY[0],
        projectedX[1],
        projectedY[1],
        projectedX[2],
        projectedY[2]);

    // Represents the percentile of the triangle's view-cone? Hemisphere surface area = 2Pir^2, r=1, A=2Pi
    float formFactor = projectedArea / kPi; // Shouldn't divide by the original triangle area?

    if (!std::isfinite(formFactor) || formFactor <= kEpsilon) {
        return 0.0f;
    }
    return std::min(formFactor, 0.95f);
}

Vector3f meshVertex(const CMeshBuffer &mesh, unsigned int vertexIndex) {
    std::size_t offset = static_cast<std::size_t>(vertexIndex) * 3;
    return Vector3f(mesh.vertices[offset], mesh.vertices[offset + 1], mesh.vertices[offset + 2]);
}

// Adapted from Editor/radiosity/original/patch.cpp:patch::patchListFromObjectList.
std::vector<SRadiosityPatch> makePatchesFromMeshPorted(const std::vector<SCornellMeshElement> &meshElements) {
    std::vector<SRadiosityPatch> patches;

    for (const SCornellMeshElement &element : meshElements) {
        if (!element.mesh) {
            continue;
        }

        const CMeshBuffer &mesh = *element.mesh;
        for (std::size_t triangleOffset = 0; triangleOffset + 2 < mesh.triangles.size(); triangleOffset += 3) {
            int patchId = static_cast<int>(patches.size());
            Vector3f v0 = meshVertex(mesh, mesh.triangles[triangleOffset]);
            Vector3f v1 = meshVertex(mesh, mesh.triangles[triangleOffset + 1]);
            Vector3f v2 = meshVertex(mesh, mesh.triangles[triangleOffset + 2]);
            patches.push_back(makePatchPorted(patchId, element.name + " patch " + std::to_string(patchId), v0, v1, v2, element.reflectance, element.emissivity));
        }
    }

    return patches;
}

std::vector<SRadiosityPatch> makeCornellBoxScenePorted(const SCornellBoxConfig &config) {
    std::vector<SCornellMeshElement> meshElements = makeCornellBoxMeshElements(config);
    return makePatchesFromMeshPorted(meshElements);
}

void appendVisibilityTriangle(SVisibilityCollisionScene &scene, const SRadiosityPatch &patch) {
    const unsigned int firstVertexIndex = scene.mesh.nvertices;
    for (const Vector3f &vertex : patch.vertex) {
        scene.mesh.vertices.push_back(vertex.x);
        scene.mesh.vertices.push_back(vertex.y);
        scene.mesh.vertices.push_back(vertex.z);
        ++scene.mesh.nvertices;
    }

    scene.mesh.triangles.push_back(firstVertexIndex);
    scene.mesh.triangles.push_back(firstVertexIndex + 1);
    scene.mesh.triangles.push_back(firstVertexIndex + 2);
    scene.patchIdByTriangleIndex.push_back(patch.id);
    ++scene.mesh.nfaces;
}

void finalizeVisibilityMeshBounds(SVisibilityCollisionScene &scene) {
    if (scene.mesh.vertices.empty()) {
        scene.mesh.boundingBox = CBoundingBox3D(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f));
        return;
    }

    Vector3f minBounds(scene.mesh.vertices[0], scene.mesh.vertices[1], scene.mesh.vertices[2]);
    Vector3f maxBounds = minBounds;
    for (std::size_t i = 3; i + 2 < scene.mesh.vertices.size(); i += 3) {
        Vector3f vertex(scene.mesh.vertices[i], scene.mesh.vertices[i + 1], scene.mesh.vertices[i + 2]);
        minBounds = minVector(minBounds, vertex);
        maxBounds = maxVector(maxBounds, vertex);
    }

    scene.mesh.faces.resize(scene.mesh.nfaces);
    scene.mesh.boundingBox = CBoundingBox3D(minBounds, maxBounds);
}

SVisibilityCollisionScene makeVisibilityCollisionScenePorted(
    const std::vector<SRadiosityPatch> &patches,
    unsigned int maxFacesPerNode = 18,
    unsigned int maxDepth = 8) {

    SVisibilityCollisionScene scene;
    scene.mesh.nvertices = 0;
    scene.mesh.nnormals = 0;
    scene.mesh.ntexcoords = 0;
    scene.mesh.nmaterials = 0;
    scene.mesh.nfaces = 0;
    scene.mesh.vertices.reserve(patches.size() * 9);
    scene.mesh.triangles.reserve(patches.size() * 3);
    scene.patchIdByTriangleIndex.reserve(patches.size());

    // Navigate all the patches and add its triagles (vertex by vertex) to the SVisibilityCollisionScene
    for (const SRadiosityPatch &patch : patches) {
        appendVisibilityTriangle(scene, patch);
    }

    finalizeVisibilityMeshBounds(scene);
    scene.octree = buildMeshOctreePorted(scene.mesh, maxFacesPerNode, maxDepth);
    return scene;
}

struct SOctreeNodeRayEntry {
    const SMeshOctreeNode *node = nullptr;
    float entryDistance = 0.0f;
};

bool nodeIntersectsOcclusionSegment(
    const SMeshOctreeNode &node,
    const SRay3D &ray,
    float maxOcclusionDistance,
    float &entryDistance) {

    const auto entry = rayWithBoundingBoxPorted(node.box, ray);
    if (!entry || *entry > maxOcclusionDistance) {
        return false;
    }

    entryDistance = *entry;
    return true;
}

bool triangleBlocksPatchSegment(
    const SRadiosityPatch &receiver,
    const SRadiosityPatch &source,
    const SVisibilityCollisionScene &collisionScene,
    const SRay3D &ray,
    float pathLength,
    unsigned int triangleIndex,
    SRadiositySummary &summary) {

    ++summary.octreeCandidateTriangleTests;

    const std::size_t triangleOffset = static_cast<std::size_t>(triangleIndex) * 3;
    if (triangleOffset + 2 >= collisionScene.mesh.triangles.size()) {
        return false;
    }

    const auto v1 = meshVertexPorted(collisionScene.mesh, collisionScene.mesh.triangles[triangleOffset]);
    const auto v2 = meshVertexPorted(collisionScene.mesh, collisionScene.mesh.triangles[triangleOffset + 1]);
    const auto v3 = meshVertexPorted(collisionScene.mesh, collisionScene.mesh.triangles[triangleOffset + 2]);
    if (!v1 || !v2 || !v3) {
        return false;
    }

    auto hit = rayWithTrianglePorted(ray, *v1, *v2, *v3, triangleIndex);
    if (!hit || hit->distance <= kOcclusionEpsilon || hit->distance >= pathLength - kOcclusionEpsilon) {
        return false;
    }

    if (hit->triangleIndex >= collisionScene.patchIdByTriangleIndex.size()) {
        return false;
    }

    const int hitPatchId = collisionScene.patchIdByTriangleIndex[hit->triangleIndex];
    return hitPatchId != receiver.id && hitPatchId != source.id;
}

bool findOccluderInSegmentNode(
    const SRadiosityPatch &receiver,
    const SRadiosityPatch &source,
    const SVisibilityCollisionScene &collisionScene,
    const SMeshOctreeNode &node,
    const SRay3D &ray,
    float pathLength,
    float maxOcclusionDistance,
    SRadiositySummary &summary) {

    for (unsigned int triangleIndex : node.triangleIndices) {
        if (triangleBlocksPatchSegment(receiver, source, collisionScene, ray, pathLength, triangleIndex, summary)) {
            return true;
        }
    }

    std::array<SOctreeNodeRayEntry, 8> childEntries;
    std::size_t childCount = 0;
    for (const auto &child : node.children) {
        if (!child) {
            continue;
        }

        float entryDistance = 0.0f;
        if (nodeIntersectsOcclusionSegment(*child, ray, maxOcclusionDistance, entryDistance)) {
            childEntries[childCount++] = {child.get(), entryDistance};
        }
    }

    std::sort(childEntries.begin(), childEntries.begin() + static_cast<std::ptrdiff_t>(childCount), [](const SOctreeNodeRayEntry &a, const SOctreeNodeRayEntry &b) {
        return a.entryDistance < b.entryDistance;
    });

    for (std::size_t i = 0; i < childCount; ++i) {
        if (findOccluderInSegmentNode(receiver, source, collisionScene, *childEntries[i].node, ray, pathLength, maxOcclusionDistance, summary)) {
            return true;
        }
    }

    return false;
}

bool hasOccluderBetweenPatchesPorted(
    const SRadiosityPatch &receiver,
    const SRadiosityPatch &source,
    const SVisibilityCollisionScene &collisionScene,
    SRadiositySummary &summary) {

    Vector3f path = source.centroid - receiver.centroid;
    float pathLength = path.Length();
    if (pathLength <= kEpsilon) {
        return false;
    }

    ++summary.occlusionPairsTested;
    summary.bruteForceOcclusionTriangleTests += collisionScene.patchIdByTriangleIndex.size();

    const float maxOcclusionDistance = pathLength - kOcclusionEpsilon;
    if (maxOcclusionDistance <= kOcclusionEpsilon || !collisionScene.octree.root) {
        return false;
    }

    const SRay3D ray{receiver.centroid, path.Normalized()};
    float rootEntryDistance = 0.0f;
    if (!nodeIntersectsOcclusionSegment(*collisionScene.octree.root, ray, maxOcclusionDistance, rootEntryDistance)) {
        return false;
    }

    if (!findOccluderInSegmentNode(receiver, source, collisionScene, *collisionScene.octree.root, ray, pathLength, maxOcclusionDistance, summary)) {
        return false;
    }

    ++summary.directOccludedPairs;
    return true;
}

// Adapted from Editor/radiosity/original/formfactorscene.cpp:formFactorScene::formFactorScene.
//
// This version improves the old patch-pair form-factor loop, but uses the ported
// octree/ray/triangle collision algorithms to reject occluded direct-light
// pairs. Indirect patch-to-patch visibility stays with the projected form-factor
// adapter used by the original radiosity port, keeping the dense example
// practical to run.
SRadiositySummary calculateFormFactorSceneOctreePorted(
    std::vector<SRadiosityPatch> &patches,
    const SVisibilityCollisionScene &collisionScene) {
    SRadiositySummary summary;
    summary.patchCount = static_cast<int>(patches.size());

    for (int i = 0; i < patches.size(); ++i) {
        SRadiosityPatch &receiver = patches[i];

        for (int j = i+1; j < patches.size(); ++j) {
            SRadiosityPatch &source = patches[j];
            if (!isVisiblePorted(receiver, source)) {
                continue;
            }

            const float receiverFormFactor = calculateProjectedFormFactorPorted(receiver, source);
            const float sourceFormFactor = calculateProjectedFormFactorPorted(source, receiver);
            const bool receiverContributes = receiverFormFactor > kFormFactorContributionThreshold;
            const bool sourceContributes = sourceFormFactor > kFormFactorContributionThreshold;
            if (!receiverContributes && !sourceContributes) {
                ++summary.formFactorSkippedPairs;
                continue;
            }

            if (hasOccluderBetweenPatchesPorted(receiver, source, collisionScene, summary)) {
                continue;
            }

            ++summary.normalVisiblePairs;

            if (receiverContributes) {
                receiver.patches.push_back(SViewedPatch{source.id, receiverFormFactor});
                receiver.coverage += receiverFormFactor;
                summary.maxFormFactor = std::max(summary.maxFormFactor, receiverFormFactor);
            }

            if (sourceContributes) {
                source.patches.push_back(SViewedPatch{receiver.id, sourceFormFactor});
                source.coverage += sourceFormFactor;
                summary.maxFormFactor = std::max(summary.maxFormFactor, sourceFormFactor);
            }
        }


        // The visible patches projections may overlap, creating a coverage area greater than the actual view-cone surface area
        // Scale the view areas to a coherent value to try to account for the innacuracy
        // Best way to actually solve the issue is to implement klipping for the projected sources onto the receiver
        constexpr float maxCoverage = 0.90f;
        if (receiver.coverage > maxCoverage) {
            float scale = maxCoverage / receiver.coverage;
            for (SViewedPatch &viewedPatch : receiver.patches) {
                viewedPatch.formFactor *= scale;
            }
            receiver.coverage = maxCoverage;
        }


        summary.nonZeroFormFactors += static_cast<int>(receiver.patches.size());
        summary.maxRowSum = std::max(summary.maxRowSum, receiver.coverage);
    }
    return summary;
}

// Adapted from Editor/radiosity/original/gaussseidel.cpp:GaussSeidel::resolver.
void gaussSeidelRadiosityPorted(std::vector<SRadiosityPatch> &patches, SRadiositySummary &summary, int maxIterations = 80, float tolerance = 0.0001f) {
    for (SRadiosityPatch &patch : patches) {
        patch.radiosity = patch.emissivity;
    }

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        float maxDelta = 0.0f;

        for (SRadiosityPatch &patch : patches) {
            SRgb incoming;
            for (const SViewedPatch &viewed : patch.patches) {
                const SRgb &sourceRadiosity = patches[viewed.id].radiosity;
                incoming += sourceRadiosity * viewed.formFactor;
            }

            SRgb next = patch.emissivity + multiply(patch.reflectance, incoming);
            maxDelta = std::max(maxDelta, maxAbsDifference(next, patch.radiosity));
            patch.radiosity = next;
        }

        summary.iterations = iteration + 1;
        summary.maxDelta = maxDelta;
        if (maxDelta <= tolerance) {
            break;
        }
    }
}

std::unique_ptr<CMeshBuffer> makePatchMesh(const SRadiosityPatch &patch) {
    auto mesh = std::make_unique<CMeshBuffer>();
    mesh->nvertices = 3;
    mesh->nnormals = 0;
    mesh->ntexcoords = 0;
    mesh->nmaterials = 0;
    mesh->nfaces = 1;
    mesh->vertices = {
        patch.vertex[0].x,
        patch.vertex[0].y,
        patch.vertex[0].z,
        patch.vertex[1].x,
        patch.vertex[1].y,
        patch.vertex[1].z,
        patch.vertex[2].x,
        patch.vertex[2].y,
        patch.vertex[2].z,
    };
    mesh->triangles = {0, 1, 2};
    mesh->faces.resize(mesh->nfaces);

    Vector3f minBounds = minVector(patch.vertex[0], minVector(patch.vertex[1], patch.vertex[2]));
    Vector3f maxBounds = maxVector(patch.vertex[0], maxVector(patch.vertex[1], patch.vertex[2]));
    mesh->boundingBox = CBoundingBox3D(minBounds, maxBounds);
    return mesh;
}

class CMeshSceneObject : public CSceneComposite {
public:
    CMeshSceneObject(std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, EPolygonMode polygonMode = EPolygonMode::Fill)
        : mMesh(std::move(mesh)) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        render->SetMaterial(mat);
        render->SetPolygonMode(polygonMode);
        AddComponent<IRenderComponent>(std::move(render));
    }

private:
    std::unique_ptr<CMeshBuffer> mMesh;
};

class CRotatingSceneGroup
    : public CSceneComposite
    , public UListener<core_events::IUpdateListener> {
public:
    void OnUpdate() override {
        mAngleDegrees += 0.25f;
        if (mAngleDegrees >= 360.0f) {
            mAngleDegrees -= 360.0f;
        }
        SetRotation(mAngleDegrees, Vector3f(0.0f, 0.0f, 1.0f));
    }

private:
    float mAngleDegrees = -18.0f;
};

bool expect(bool condition, const std::string &message) {
    if (!condition) {
        std::cerr << "Self-test failed: " << message << "\n";
        return false;
    }
    return true;
}

bool nearlyEqual(float a, float b, float tolerance = 0.0001f) {
    return std::fabs(a - b) <= tolerance;
}

bool nonEmissivePatchReceivedLight(const std::vector<SRadiosityPatch> &scene) {
    for (const SRadiosityPatch &scenePatch : scene) {
        if (maxChannel(scenePatch.emissivity) <= kEpsilon && maxChannel(scenePatch.radiosity) > 0.05f) {
            return true;
        }
    }
    return false;
}

bool runSelfTest() {
    bool ok = true;

    SRadiosityPatch patch = makePatchPorted(0, "test patch", Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), rgb(0.5f, 0.5f, 0.5f));
    ok = expect(nearlyEqual(patch.area, 0.5f), "patch area should match a unit right triangle") && ok;
    ok = expect(nearlyEqual(patch.normal.z, 1.0f), "patch normal should point along +Z") && ok;
    ok = expect(nearlyEqual(patch.centroid.x, 1.0f / 3.0f) && nearlyEqual(patch.centroid.y, 1.0f / 3.0f), "patch centroid should be the triangle centroid") && ok;

    SRadiosityPatch receiver = makePatchPorted(0, "receiver", Vector3f(-1.0f, -1.0f, 0.0f), Vector3f(1.0f, -1.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), rgb(0.6f, 0.6f, 0.6f));
    SRadiosityPatch sourceVisible = makePatchPorted(1, "source visible", Vector3f(-0.5f, -0.5f, 1.5f), Vector3f(0.5f, -0.5f, 1.5f), Vector3f(-0.5f, 0.5f, 1.5f), rgb(0.0f, 0.0f, 0.0f), rgb(3.0f, 3.0f, 3.0f));
    SRadiosityPatch sourceOpposedNormal = makePatchPorted(2, "source opposed normal", Vector3f(-0.5f, 0.5f, 1.5f), Vector3f(0.5f, -0.5f, 1.5f), Vector3f(-0.5f, -0.5f, 1.5f), rgb(0.0f, 0.0f, 0.0f), rgb(3.0f, 3.0f, 3.0f));
    SRadiosityPatch sourceOutsideHemisphere = makePatchPorted(3, "source outside receiver hemisphere", Vector3f(-0.5f, -0.5f, -1.5f), Vector3f(0.5f, -0.5f, -1.5f), Vector3f(-0.5f, 0.5f, -1.5f), rgb(0.0f, 0.0f, 0.0f), rgb(3.0f, 3.0f, 3.0f));

    ok = expect(isVisiblePorted(receiver, sourceVisible), "source inside the receiver hemisphere should be visible") && ok;
    ok = expect(isVisiblePorted(receiver, sourceOpposedNormal), "opposed-normal source patch should be visible") && ok;
    ok = expect(!isVisiblePorted(receiver, sourceOutsideHemisphere), "source outside the receiver hemisphere with same normal should not be visible") && ok;

    float positiveFormFactor = calculateProjectedFormFactorPorted(receiver, sourceVisible);
    float outsideHemisphereFormFactor = calculateProjectedFormFactorPorted(receiver, sourceOutsideHemisphere);
    ok = expect(positiveFormFactor > 0.0f, "source inside the receiver hemisphere should produce a positive form factor") && ok;
    ok = expect(outsideHemisphereFormFactor == 0.0f, "source outside the receiver hemisphere should produce a zero form factor") && ok;

    std::vector<SRadiosityPatch> scene = makeCornellBoxScenePorted(makeSelfTestCornellConfig());
    SVisibilityCollisionScene collisionScene = makeVisibilityCollisionScenePorted(scene, 10, 8);
    SRadiositySummary summary = calculateFormFactorSceneOctreePorted(scene, collisionScene);
    gaussSeidelRadiosityPorted(scene, summary);

    ok = expect(summary.patchCount == static_cast<int>(scene.size()), "form-factor summary patch count mismatch") && ok;
    ok = expect(summary.nonZeroFormFactors > 0, "Cornell-box scene should create visible unoccluded patch relationships") && ok;
    ok = expect(summary.normalVisiblePairs > 0, "normal-facing patch pairs should be counted") && ok;
    ok = expect(summary.directOccludedPairs > 0, "torus occluder should block at least one direct patch relationship") && ok;
    ok = expect(summary.octreeCandidateTriangleTests < summary.bruteForceOcclusionTriangleTests, "octree visibility should reduce triangle tests against a brute-force occlusion baseline") && ok;
    ok = expect(summary.iterations > 0, "Gauss-Seidel solver should run at least one iteration") && ok;
    ok = expect(nonEmissivePatchReceivedLight(scene), "non-emissive patches should receive indirect/direct radiosity from the light") && ok;

    return ok;
}

void printSummary(const SRadiositySummary &summary) {
    float reduction = 0.0f;
    if (summary.bruteForceOcclusionTriangleTests > 0) {
        reduction = 100.0f * (1.0f - static_cast<float>(summary.octreeCandidateTriangleTests) / static_cast<float>(summary.bruteForceOcclusionTriangleTests));
    }

    std::cout << "Patches: " << summary.patchCount << "\n";
    std::cout << "Normal-facing patch pairs: " << summary.normalVisiblePairs << "\n";
    std::cout << "Form-factor culled patch pairs before occlusion: " << summary.formFactorSkippedPairs << "\n";
    std::cout << "Patch pairs reaching occlusion: " << summary.occlusionPairsTested << "\n";
    std::cout << "Direct-light occluded patch pairs: " << summary.directOccludedPairs << "\n";
    std::cout << "Visible form factors: " << summary.nonZeroFormFactors << "\n";
    std::cout << "Max form factor: " << summary.maxFormFactor << "\n";
    std::cout << "Max row coverage: " << summary.maxRowSum << "\n";
    std::cout << "Gauss-Seidel iterations: " << summary.iterations << "\n";
    std::cout << "Final max delta: " << summary.maxDelta << "\n";
    std::cout << "Brute-force occlusion triangle tests baseline: " << summary.bruteForceOcclusionTriangleTests << "\n";
    std::cout << "Octree occlusion candidate triangle tests: " << summary.octreeCandidateTriangleTests << "\n";
    std::cout << "Occlusion triangle-test reduction: " << std::fixed << std::setprecision(2) << reduction << "%\n";
}

void runVisualExample(bool enterMainLoop) {
    std::vector<SRadiosityPatch> solvedScene = makeCornellBoxScenePorted(enterMainLoop ? makeOctreeDemoCornellConfig() : makeRenderSmokeCornellConfig());
    SVisibilityCollisionScene collisionScene = makeVisibilityCollisionScenePorted(solvedScene, 18, 8);
    SRadiositySummary summary = calculateFormFactorSceneOctreePorted(solvedScene, collisionScene);
    gaussSeidelRadiosityPorted(solvedScene, summary);
    printSummary(summary);

    CCore engine(1200, 720, 32);
    Camera camera(1200, 720, 70.0f, 1.0f, 1.0f);
    camera.SetOrthographicIsometric(10.5f, 7.0f, 22.0f, Vector3f(0.0f, 0.0f, 1.4f), -45.0f, 33.0f);
    engine.SetCamera(camera);
    engine.Start();
    if (!engine.IsRunning()) {
        return;
    }

    CRotatingSceneGroup radiosityGroup;
    radiosityGroup.SetPosition(Vector3f(0.0f, 0.0f, 0.0f));
    radiosityGroup.SetRotation(-18.0f, Vector3f(0.0f, 0.0f, 1.0f));
    engine.RegisterEventListener(radiosityGroup);

    std::vector<std::unique_ptr<CMeshSceneObject>> sceneObjects;
    auto addSceneObject = [&engine, &sceneObjects](std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, CSceneNode *parent) {
        auto sceneObject = std::make_unique<CMeshSceneObject>(std::move(mesh), mat, EPolygonMode::Fill);
        sceneObject->SetParent(parent);
        sceneObjects.push_back(std::move(sceneObject));
        engine.RegisterSceneElement(*sceneObjects.back());
    };

    for (const SRadiosityPatch &patch : solvedScene) {
        addSceneObject(makePatchMesh(patch), materialFromColor(toneMap(patch.radiosity, 0.55f)), &radiosityGroup);
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
    engine.UnregisterEventListener(radiosityGroup);
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
