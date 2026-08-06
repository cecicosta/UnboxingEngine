#include "CoreEvents.h"
#include "MeshBuffer.h"
#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "algorithms/LegacyCollisionAlgorithms.h"
#include "algorithms/LegacyOctreeAlgorithms.h"
#include "internal_components/IRenderComponent.h"
#include "internal_components/RenderComponent.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

using namespace unboxing_engine;
using namespace unboxing_engine::algorithms;

namespace {

constexpr float kEpsilon = 0.00001f;

struct SCandidateTriangle {
    const SMeshOctreeNode *node = nullptr;
    unsigned int triangleIndex = 0;
};

struct SRayTraceResult {
    std::vector<const SMeshOctreeNode *> traversedNodes;
    std::vector<SCandidateTriangle> candidateTriangles;
    std::vector<STriangleHit3D> hits;
};

struct SOctreeNodeRecord {
    const SMeshOctreeNode *node = nullptr;
    unsigned int depth = 0;
};

class CMeshSceneObject : public CSceneComposite {
public:
    CMeshSceneObject(std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, EPolygonMode polygonMode)
        : mMesh(std::move(mesh)) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        render->SetMaterial(mat);
        render->SetPolygonMode(polygonMode);
        AddComponent<IRenderComponent>(std::move(render));
    }

    void SetMesh(std::unique_ptr<CMeshBuffer> mesh) {
        mMesh = std::move(mesh);
        if (auto render = GetComponent<IRenderComponent>()) {
            render->SetMeshBuffer(*mMesh);
        }
    }

    void SetMaterial(const SMaterial &mat) {
        if (auto render = GetComponent<IRenderComponent>()) {
            render->SetMaterial(mat);
        }
    }

private:
    std::unique_ptr<CMeshBuffer> mMesh;
};

struct SNodeVisual {
    const SMeshOctreeNode *node = nullptr;
    unsigned int depth = 0;
    CMeshSceneObject *sceneObject = nullptr;
};

SMaterial material(float r, float g, float b, float a = 1.0f) {
    SMaterial mat{};
    mat.materialDif[0] = r;
    mat.materialDif[1] = g;
    mat.materialDif[2] = b;
    mat.materialDif[3] = a;
    mat.enable = true;
    return mat;
}

SMaterial inactiveNodeMaterial(unsigned int depth) {
    const float shade = std::min(0.16f + 0.025f * static_cast<float>(depth), 0.34f);
    return material(shade, shade + 0.015f, shade + 0.045f, 1.0f);
}

const SMaterial &rayNodeMaterial() {
    static const SMaterial mat = material(0.95f, 0.72f, 0.18f, 1.0f);
    return mat;
}

const SMaterial &candidateNodeMaterial() {
    static const SMaterial mat = material(1.00f, 0.42f, 0.12f, 1.0f);
    return mat;
}

const SMaterial &firstHitNodeMaterial() {
    static const SMaterial mat = material(1.00f, 0.08f, 0.04f, 1.0f);
    return mat;
}

Vector3f minVector(const Vector3f &a, const Vector3f &b) {
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

Vector3f maxVector(const Vector3f &a, const Vector3f &b) {
    return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

std::unique_ptr<CMeshBuffer> makeEmptyMesh() {
    auto mesh = std::make_unique<CMeshBuffer>();
    mesh->nvertices = 0;
    mesh->nnormals = 0;
    mesh->ntexcoords = 0;
    mesh->nmaterials = 0;
    mesh->nfaces = 0;
    mesh->boundingBox = CBoundingBox3D(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f));
    return mesh;
}

std::unique_ptr<CMeshBuffer> makeMesh(const std::vector<Vector3f> &vertices, const std::vector<unsigned int> &triangles) {
    if (vertices.empty() || triangles.empty()) {
        return makeEmptyMesh();
    }

    auto mesh = std::make_unique<CMeshBuffer>();
    mesh->nvertices = static_cast<uint>(vertices.size());
    mesh->nnormals = 0;
    mesh->ntexcoords = 0;
    mesh->nmaterials = 0;
    mesh->nfaces = static_cast<uint>(triangles.size() / 3);
    mesh->triangles = triangles;
    mesh->faces.resize(mesh->nfaces);
    mesh->vertices.reserve(vertices.size() * 3);

    Vector3f minBounds = vertices[0];
    Vector3f maxBounds = vertices[0];
    for (const Vector3f &vertex : vertices) {
        mesh->vertices.push_back(vertex.x);
        mesh->vertices.push_back(vertex.y);
        mesh->vertices.push_back(vertex.z);
        minBounds = minVector(minBounds, vertex);
        maxBounds = maxVector(maxBounds, vertex);
    }

    mesh->boundingBox = CBoundingBox3D(minBounds, maxBounds);
    return mesh;
}

std::unique_ptr<CMeshBuffer> makeBoxMesh(const CBoundingBox3D &box) {
    auto mesh = std::make_unique<CMeshBuffer>();
    mesh->vertices = box.GetVertices();
    mesh->triangles = box.GetTriangles();
    mesh->nvertices = 8;
    mesh->nnormals = 0;
    mesh->ntexcoords = 0;
    mesh->nmaterials = 0;
    mesh->nfaces = static_cast<uint>(mesh->triangles.size() / 3);
    mesh->faces.resize(mesh->nfaces);
    mesh->boundingBox = box;
    return mesh;
}

std::unique_ptr<CMeshBuffer> makeRayMesh(const Vector3f &origin, const Vector3f &end) {
    auto mesh = primitive_utils::Lines(origin, end);
    mesh->boundingBox = CBoundingBox3D(origin, end);
    return mesh;
}

std::unique_ptr<CMeshBuffer> makeMarkerMesh(const Vector3f &center, float radius) {
    const Vector3f offset(radius, radius, radius);
    return makeBoxMesh(CBoundingBox3D(center - offset, center + offset));
}

std::unique_ptr<CMeshBuffer> makeTriangleHighlightMesh(
    const CMeshBuffer &mesh,
    const STriangleHit3D &hit,
    const Vector3f &lift) {

    const std::size_t triangleOffset = static_cast<std::size_t>(hit.triangleIndex) * 3;
    if (triangleOffset + 2 >= mesh.triangles.size()) {
        return makeEmptyMesh();
    }

    const auto v0 = meshVertexPorted(mesh, mesh.triangles[triangleOffset]);
    const auto v1 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 1]);
    const auto v2 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 2]);
    if (!v0 || !v1 || !v2) {
        return makeEmptyMesh();
    }

    return makeMesh({*v0 + lift, *v1 + lift, *v2 + lift}, {0, 1, 2});
}

void addHitSorted(std::vector<STriangleHit3D> &hits, const STriangleHit3D &hit) {
    auto it = std::lower_bound(hits.begin(), hits.end(), hit.distance, [](const STriangleHit3D &candidate, float distance) {
        return candidate.distance < distance;
    });
    hits.insert(it, hit);
}

void collectNodeRecords(const SMeshOctreeNode &node, unsigned int depth, std::vector<SOctreeNodeRecord> &records) {
    records.push_back({&node, depth});
    for (const auto &child : node.children) {
        if (child) {
            collectNodeRecords(*child, depth + 1, records);
        }
    }
}

std::vector<SOctreeNodeRecord> collectNodeRecords(const SMeshOctree &octree) {
    std::vector<SOctreeNodeRecord> records;
    if (octree.root) {
        collectNodeRecords(*octree.root, 0, records);
    }
    return records;
}

struct SNodeRayEntry {
    const SMeshOctreeNode *node = nullptr;
    float entryDistance = 0.0f;
};

void collectRayTrace(const SMeshOctreeNode &node, const SRay3D &ray, SRayTraceResult &trace) {
    if (!rayWithBoundingBoxPorted(node.box, ray)) {
        return;
    }

    trace.traversedNodes.push_back(&node);
    for (unsigned int triangleIndex : node.triangleIndices) {
        trace.candidateTriangles.push_back({&node, triangleIndex});
    }

    std::vector<SNodeRayEntry> children;
    children.reserve(node.children.size());
    for (const auto &child : node.children) {
        if (!child) {
            continue;
        }

        const auto entry = rayWithBoundingBoxPorted(child->box, ray);
        if (entry) {
            children.push_back({child.get(), *entry});
        }
    }

    std::sort(children.begin(), children.end(), [](const SNodeRayEntry &a, const SNodeRayEntry &b) {
        return a.entryDistance < b.entryDistance;
    });

    for (const SNodeRayEntry &child : children) {
        collectRayTrace(*child.node, ray, trace);
    }
}

SRayTraceResult traceRayThroughOctree(const CMeshBuffer &mesh, const SMeshOctree &octree, const SRay3D &ray) {
    SRayTraceResult trace;
    if (!octree.root) {
        return trace;
    }

    collectRayTrace(*octree.root, ray, trace);
    for (const SCandidateTriangle &candidate : trace.candidateTriangles) {
        const std::size_t triangleOffset = static_cast<std::size_t>(candidate.triangleIndex) * 3;
        if (triangleOffset + 2 >= mesh.triangles.size()) {
            continue;
        }

        const auto v0 = meshVertexPorted(mesh, mesh.triangles[triangleOffset]);
        const auto v1 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 1]);
        const auto v2 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 2]);
        if (!v0 || !v1 || !v2) {
            continue;
        }

        auto hit = rayWithTrianglePorted(ray, *v0, *v1, *v2, candidate.triangleIndex);
        if (hit) {
            addHitSorted(trace.hits, *hit);
        }
    }

    return trace;
}

Vector3f transformPoint(const Matrix4f &transform, float x, float y, float z) {
    const float transformedX = transform.at(0, 0) * x + transform.at(0, 1) * y + transform.at(0, 2) * z + transform.at(0, 3);
    const float transformedY = transform.at(1, 0) * x + transform.at(1, 1) * y + transform.at(1, 2) * z + transform.at(1, 3);
    const float transformedZ = transform.at(2, 0) * x + transform.at(2, 1) * y + transform.at(2, 2) * z + transform.at(2, 3);
    const float transformedW = transform.at(3, 0) * x + transform.at(3, 1) * y + transform.at(3, 2) * z + transform.at(3, 3);

    if (std::fabs(transformedW) <= kEpsilon) {
        return {transformedX, transformedY, transformedZ};
    }
    return {transformedX / transformedW, transformedY / transformedW, transformedZ / transformedW};
}

SRay3D makeRayFromScreen(const Camera &camera, int screenX, int screenY) {
    const int maxX = static_cast<int>(std::max(1u, camera.mWidth)) - 1;
    const int maxY = static_cast<int>(std::max(1u, camera.mHeight)) - 1;
    const float pixelX = static_cast<float>(std::clamp(screenX, 0, maxX)) + 0.5f;
    const float pixelY = static_cast<float>(std::clamp(screenY, 0, maxY)) + 0.5f;
    const float ndcX = 2.0f * pixelX / static_cast<float>(std::max(1u, camera.mWidth)) - 1.0f;
    const float ndcY = 1.0f - 2.0f * pixelY / static_cast<float>(std::max(1u, camera.mHeight));

    const Matrix4f inverseProjection = camera.mTransformation.Inverse();
    const Vector3f nearPoint = transformPoint(inverseProjection, ndcX, ndcY, -1.0f);
    const Vector3f farPoint = transformPoint(inverseProjection, ndcX, ndcY, 1.0f);
    const Vector3f direction = farPoint - nearPoint;

    if (direction.Length() <= kEpsilon) {
        return {nearPoint, Vector3f(0.0f, 0.0f, -1.0f)};
    }
    return {nearPoint, direction.Normalized()};
}

void screenPointForWorld(const Camera &camera, const Vector3f &worldPoint, int &screenX, int &screenY) {
    const Vector3f clipPoint = transformPoint(camera.mTransformation, worldPoint.x, worldPoint.y, worldPoint.z);
    const int maxX = static_cast<int>(std::max(1u, camera.mWidth)) - 1;
    const int maxY = static_cast<int>(std::max(1u, camera.mHeight)) - 1;
    screenX = static_cast<int>(std::lround((clipPoint.x + 1.0f) * 0.5f * static_cast<float>(camera.mWidth)));
    screenY = static_cast<int>(std::lround((1.0f - clipPoint.y) * 0.5f * static_cast<float>(camera.mHeight)));
    screenX = std::clamp(screenX, 0, maxX);
    screenY = std::clamp(screenY, 0, maxY);
}

const SMeshOctreeNode *nodeHoldingTriangle(const SRayTraceResult &trace, unsigned int triangleIndex) {
    for (const SCandidateTriangle &candidate : trace.candidateTriangles) {
        if (candidate.triangleIndex == triangleIndex) {
            return candidate.node;
        }
    }
    return nullptr;
}

class COctreeRayPickingController : public CSceneComposite, public UListener<core_events::IMouseInputEvent> {
public:
    COctreeRayPickingController(
        const CMeshBuffer &mesh,
        const SMeshOctree &octree,
        const Camera &camera,
        std::vector<SNodeVisual> &nodeVisuals,
        CMeshSceneObject &rayObject,
        CMeshSceneObject &hitTriangleObject,
        CMeshSceneObject &hitMarkerObject)
        : mMesh(mesh)
        , mOctree(octree)
        , mCamera(camera)
        , mNodeVisuals(nodeVisuals)
        , mRayObject(rayObject)
        , mHitTriangleObject(hitTriangleObject)
        , mHitMarkerObject(hitMarkerObject) {}

    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        const bool leftPressed = cursor.cursorState[L_BUTTON] == 1 && cursor.isButtonPressed;
        if (leftPressed && !mWasLeftPressed) {
            FireFromScreen(cursor.buttonPressedX, cursor.buttonPressedY);
        }
        mWasLeftPressed = leftPressed;
    }

    void FireFromScreen(int screenX, int screenY) {
        ApplyRay(makeRayFromScreen(mCamera, screenX, screenY));
    }

private:
    void ApplyRay(const SRay3D &ray) {
        const SRayTraceResult trace = traceRayThroughOctree(mMesh, mOctree, ray);

        Vector3f rayEnd = ray.origin + ray.direction * 13.0f;
        const STriangleHit3D *firstHit = trace.hits.empty() ? nullptr : &trace.hits.front();
        const SMeshOctreeNode *firstHitNode = nullptr;

        if (firstHit) {
            firstHitNode = nodeHoldingTriangle(trace, firstHit->triangleIndex);
            rayEnd = firstHit->intersection + ray.direction * 3.5f;
            const Vector3f liftDirection = firstHit->distance > kEpsilon
                                                ? (ray.origin - firstHit->intersection).Normalized()
                                                : firstHit->normal;
            mHitTriangleObject.SetMesh(makeTriangleHighlightMesh(mMesh, *firstHit, liftDirection * 0.055f));
            mHitMarkerObject.SetMesh(makeMarkerMesh(firstHit->intersection + liftDirection * 0.08f, 0.10f));
        } else {
            mHitTriangleObject.SetMesh(makeEmptyMesh());
            mHitMarkerObject.SetMesh(makeEmptyMesh());
        }

        mRayObject.SetMesh(makeRayMesh(ray.origin, rayEnd));
        UpdateNodeMaterials(trace, firstHitNode);
    }

    void UpdateNodeMaterials(const SRayTraceResult &trace, const SMeshOctreeNode *firstHitNode) {
        std::unordered_set<const SMeshOctreeNode *> traversedNodes(trace.traversedNodes.begin(), trace.traversedNodes.end());
        std::unordered_set<const SMeshOctreeNode *> candidateNodes;
        for (const SCandidateTriangle &candidate : trace.candidateTriangles) {
            candidateNodes.insert(candidate.node);
        }

        for (SNodeVisual &visual : mNodeVisuals) {
            if (visual.node == firstHitNode) {
                visual.sceneObject->SetMaterial(firstHitNodeMaterial());
            } else if (candidateNodes.find(visual.node) != candidateNodes.end()) {
                visual.sceneObject->SetMaterial(candidateNodeMaterial());
            } else if (traversedNodes.find(visual.node) != traversedNodes.end()) {
                visual.sceneObject->SetMaterial(rayNodeMaterial());
            } else {
                visual.sceneObject->SetMaterial(inactiveNodeMaterial(visual.depth));
            }
        }
    }

    const CMeshBuffer &mMesh;
    const SMeshOctree &mOctree;
    const Camera &mCamera;
    std::vector<SNodeVisual> &mNodeVisuals;
    CMeshSceneObject &mRayObject;
    CMeshSceneObject &mHitTriangleObject;
    CMeshSceneObject &mHitMarkerObject;
    bool mWasLeftPressed = false;
};

bool expect(bool condition, const std::string &message) {
    if (!condition) {
        std::cerr << "Self-test failed: " << message << "\n";
        return false;
    }
    return true;
}

bool nearlyEqual(float a, float b, float tolerance = 0.0005f) {
    return std::fabs(a - b) <= tolerance;
}

bool runSelfTest() {
    bool ok = true;
    std::unique_ptr<CMeshBuffer> mesh = primitive_utils::Torus(4.0f, 1.15f, 32, 12);
    const SMeshOctree octree = buildMeshOctreePorted(*mesh, 8, 6);
    const std::vector<SOctreeNodeRecord> nodeRecords = collectNodeRecords(octree);
    Camera camera(1200, 720, 70.0f, 1.0f, 1.0f);
    camera.SetOrthographicIsometric(13.5f, 8.4f, 24.0f, Vector3f(0.0f, 0.0f, 0.4f), -42.0f, 31.0f);

    ok = expect(octree.root != nullptr, "octree root should be built") && ok;
    ok = expect(!nodeRecords.empty(), "octree should expose at least one node") && ok;

    int clickX = 0;
    int clickY = 0;
    screenPointForWorld(camera, Vector3f(4.05f, 0.0f, 0.0f), clickX, clickY);
    const SRay3D ray = makeRayFromScreen(camera, clickX, clickY);
    const SRayTraceResult trace = traceRayThroughOctree(*mesh, octree, ray);
    const std::vector<STriangleHit3D> bruteHits = rayWithMeshBufferPorted(*mesh, ray, true);

    ok = expect(!trace.traversedNodes.empty(), "ray should traverse octree nodes") && ok;
    ok = expect(!trace.candidateTriangles.empty(), "ray should collect candidate triangles") && ok;
    ok = expect(!trace.hits.empty(), "ray should hit the torus mesh") && ok;
    ok = expect(!bruteHits.empty(), "brute-force ray should hit the torus mesh") && ok;

    if (!trace.hits.empty() && !bruteHits.empty()) {
        ok = expect(nearlyEqual(trace.hits.front().distance, bruteHits.front().distance), "octree and brute-force first-hit distances should match") && ok;
        ok = expect(nodeHoldingTriangle(trace, trace.hits.front().triangleIndex) != nullptr, "first hit triangle should map back to a candidate node") && ok;
    }

    const std::size_t meshTriangles = mesh->triangles.size() / 3;
    ok = expect(trace.candidateTriangles.size() < meshTriangles, "octree should reduce candidate triangle tests for the sample ray") && ok;
    ok = expect(trace.traversedNodes.size() < nodeRecords.size(), "ray should highlight a subset of octree nodes") && ok;

    return ok;
}

void printSummary(const CMeshBuffer &mesh, const std::vector<SOctreeNodeRecord> &nodeRecords, const SRayTraceResult &trace) {
    const std::size_t meshTriangles = mesh.triangles.size() / 3;
    float reduction = 0.0f;
    if (meshTriangles > 0) {
        reduction = 100.0f * (1.0f - static_cast<float>(trace.candidateTriangles.size()) / static_cast<float>(meshTriangles));
    }

    std::cout << "Octree ray picking lab\n";
    std::cout << "Mesh triangles: " << meshTriangles << "\n";
    std::cout << "Octree nodes: " << nodeRecords.size() << "\n";
    std::cout << "Initial traversed nodes: " << trace.traversedNodes.size() << "\n";
    std::cout << "Initial candidate triangle tests: " << trace.candidateTriangles.size() << "\n";
    std::cout << "Initial triangle-test reduction: " << std::fixed << std::setprecision(2) << reduction << "%\n";
    if (!trace.hits.empty()) {
        std::cout << "Initial first hit triangle: " << trace.hits.front().triangleIndex << "\n";
        std::cout << "Initial first hit distance: " << trace.hits.front().distance << "\n";
    }
    std::cout << "Left-click the mesh to fire a new ray.\n";
    std::cout << "Press Esc to quit.\n";
}

void runVisualExample(bool enterMainLoop) {
    std::unique_ptr<CMeshBuffer> sourceMesh = primitive_utils::Torus(4.0f, 1.15f, 40, 14);
    SMeshOctree octree = buildMeshOctreePorted(*sourceMesh, 10, 7);
    std::vector<SOctreeNodeRecord> nodeRecords = collectNodeRecords(octree);

    Camera camera(1200, 720, 70.0f, 1.0f, 1.0f);
    camera.SetOrthographicIsometric(13.5f, 8.4f, 24.0f, Vector3f(0.0f, 0.0f, 0.4f), -42.0f, 31.0f);

    int initialClickX = 0;
    int initialClickY = 0;
    screenPointForWorld(camera, Vector3f(4.05f, 0.0f, 0.0f), initialClickX, initialClickY);
    const SRayTraceResult initialTrace = traceRayThroughOctree(*sourceMesh, octree, makeRayFromScreen(camera, initialClickX, initialClickY));
    printSummary(*sourceMesh, nodeRecords, initialTrace);

    CCore engine(1200, 720, 32);
    engine.SetCamera(camera);
    engine.Start();
    if (!engine.IsRunning()) {
        return;
    }

    std::vector<std::unique_ptr<CMeshSceneObject>> sceneObjects;
    auto addObject = [&engine, &sceneObjects](std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, EPolygonMode polygonMode) {
        auto sceneObject = std::make_unique<CMeshSceneObject>(std::move(mesh), mat, polygonMode);
        CMeshSceneObject *raw = sceneObject.get();
        sceneObjects.push_back(std::move(sceneObject));
        engine.RegisterSceneElement(*raw);
        return raw;
    };

    addObject(std::make_unique<CMeshBuffer>(*sourceMesh), material(0.17f, 0.55f, 0.70f, 1.0f), EPolygonMode::Line);

    std::vector<SNodeVisual> nodeVisuals;
    nodeVisuals.reserve(nodeRecords.size());
    for (const SOctreeNodeRecord &record : nodeRecords) {
        CMeshSceneObject *nodeObject = addObject(makeBoxMesh(record.node->box), inactiveNodeMaterial(record.depth), EPolygonMode::Line);
        nodeVisuals.push_back({record.node, record.depth, nodeObject});
    }

    CMeshSceneObject *rayObject = addObject(makeEmptyMesh(), material(1.00f, 0.10f, 0.04f, 1.0f), EPolygonMode::Line);
    CMeshSceneObject *hitTriangleObject = addObject(makeEmptyMesh(), material(1.00f, 0.86f, 0.18f, 1.0f), EPolygonMode::Fill);
    CMeshSceneObject *hitMarkerObject = addObject(makeEmptyMesh(), material(0.08f, 0.92f, 1.00f, 1.0f), EPolygonMode::Fill);

    COctreeRayPickingController controller(*sourceMesh, octree, engine.GetCamera(), nodeVisuals, *rayObject, *hitTriangleObject, *hitMarkerObject);
    engine.RegisterSceneElement(controller);
    controller.FireFromScreen(initialClickX, initialClickY);

    if (enterMainLoop) {
        engine.Run();
    } else {
        controller.FireFromScreen(initialClickX, initialClickY);
        for (const auto &sceneObject : sceneObjects) {
            if (auto render = sceneObject->GetComponent<IRenderComponent>()) {
                render->OnRender();
            }
        }
    }

    engine.UnregisterSceneElement(controller);
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
