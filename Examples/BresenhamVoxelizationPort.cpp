#include "BresenhamVoxelization.h"
#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "internal_components/RenderComponent.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace unboxing_engine;

namespace {

using namespace voxelization_demo;

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
    std::vector<Vector3f> hits = rayCollisionsWithMeshBufferPorted(Vector3f(0.0f, 0.0f, -1000.0f), Vector3f(0.0f, 0.0f, 1.0f), *cube);
    ok = expect(hits.size() == 2, "central ray should find two unique cube intersections") && ok;

    SVoxelMap map;
    SVoxelizationSummary summary = createVoxelizedScenePorted(*cube, map);
    ok = expect(summary.raysCast == 16, "default map sweep should cast 16 rays") && ok;
    ok = expect(summary.intersections == 32, "default cube sweep should find 32 unique intersections") && ok;
    ok = expect(summary.filledVoxels == 48, "ported fill should produce 48 voxel cells for the default cube") && ok;
    ok = expect(map.voxels.count({0, 0, -2}) == 1, "expected central lower voxel missing") && ok;
    ok = expect(map.voxels.count({0, 0, -1}) == 1, "expected central middle voxel missing") && ok;
    ok = expect(map.voxels.count({0, 0, 0}) == 1, "expected central upper sampled voxel missing") && ok;
    ok = expect(map.voxels.count({0, 0, 1}) == 0, "ported fill should stop one voxel before the exit point") && ok;
    ok = expect(map.voxels.count({3, 0, 0}) == 0, "outside voxel should not be filled") && ok;

    return ok;
}

void runVisualExample(bool enterMainLoop) {
    std::unique_ptr<CMeshBuffer> sourceMesh = primitive_utils::Torus(9.0f, 5.0f);

    SVoxelMap map;
    map.dim = 1;
    map.sizeI = 30;
    map.sizeJ = 30;
    SVoxelizationSummary summary = createVoxelizedScenePorted(*sourceMesh, map);

    std::cout << "Rays cast: " << summary.raysCast << "\n";
    std::cout << "Mesh intersections: " << summary.intersections << "\n";
    std::cout << "Filled voxels: " << summary.filledVoxels << "\n";

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

    addSceneObject(std::move(sourceMesh), material(0.35f, 0.24f, 0.23f, 1.0f), EPolygonMode::Fill, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f)/3);
    addSceneObject(primitive_utils::Torus(9.0f, 5.0f), material(0.85f, 0.72f, 0.64f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.012f, 1.012f, 1.012f)/3);

    addSceneObject(makeRayMesh(Vector3f(0.0f, 0.0f, -2.2f), Vector3f(0.0f, 0.0f, 2.2f)), material(0.95f, 0.78f, 0.18f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f)/3);
    addSceneObject(makeRayMesh(Vector3f(3.0f, 0.0f, -2.2f), Vector3f(3.0f, 0.0f, 2.2f)), material(0.95f, 0.78f, 0.18f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f)/3);
    addSceneObject(makeRayMesh(Vector3f(2.0f, 2.0f, -2.2f), Vector3f(2.0f, 2.0f, 2.2f)), material(0.95f, 0.78f, 0.18f, 1.0f), EPolygonMode::Line, &sourceGroup, Vector3f(1.0f, 1.0f, 1.0f)/3);

    for (const SVoxel &voxel : map.voxels) {
        addSceneObject(makeVoxelMesh(voxel, map.dim, 0.82f), material(0.10f, 0.43f, 0.98f, 1.0f), EPolygonMode::Fill, &voxelGroup, Vector3f(1.0f, 1.0f, 1.0f)/3);
        addSceneObject(makeVoxelMesh(voxel, map.dim, 0.90f), material(0.02f, 0.08f, 0.18f, 1.0f), EPolygonMode::Line, &voxelGroup, Vector3f(1.0f, 1.0f, 1.0f)/3);
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

}  // namespace

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
