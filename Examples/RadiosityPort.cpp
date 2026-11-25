#include "MeshBuffer.h"
#include "RadiositySolver.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "CoreEvents.h"
#include "internal_components/RenderComponent.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace unboxing_engine;

namespace {

using namespace radiosity_demo;

SMaterial materialFromColor(const SRgb &color) {
    SMaterial mat{};
    mat.materialDif[0] = clamp01(color.r);
    mat.materialDif[1] = clamp01(color.g);
    mat.materialDif[2] = clamp01(color.b);
    mat.materialDif[3] = 1.0f;
    mat.enable = true;
    return mat;
}

Vector3f minVector(const Vector3f &a, const Vector3f &b) {
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

Vector3f maxVector(const Vector3f &a, const Vector3f &b) {
    return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
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

float findFormFactor(const SRadiosityPatch &patch, int viewedPatchId) {
    for (const SViewedPatch &viewedPatch : patch.patches) {
        if (viewedPatch.id == viewedPatchId) {
            return viewedPatch.formFactor;
        }
    }
    return 0.0f;
}

bool runSelfTest() {
    bool ok = true;

    SRadiosityPatch patch = makePatchPorted(0, "test patch", Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), rgb(0.5f, 0.5f, 0.5f));
    ok = expect(nearlyEqual(patch.area, 0.5f), "patch area should match a unit right triangle") && ok;
    ok = expect(nearlyEqual(patch.normal.z, 1.0f), "patch normal should point along +Z") && ok;
    ok = expect(nearlyEqual(patch.centroid.x, 1.0f / 3.0f) && nearlyEqual(patch.centroid.y, 1.0f / 3.0f), "patch centroid should be the triangle centroid") && ok;

    SRadiosityPatch receiver = makePatchPorted(0, "receiver", Vector3f(-1.0f, -1.0f, 0.0f), Vector3f(1.0f, -1.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), rgb(0.6f, 0.6f, 0.6f));
    SRadiosityPatch sourceFacing = makePatchPorted(1, "source facing", Vector3f(-0.5f, 0.5f, 1.5f), Vector3f(0.5f, -0.5f, 1.5f), Vector3f(-0.5f, -0.5f, 1.5f), rgb(0.0f, 0.0f, 0.0f), rgb(3.0f, 3.0f, 3.0f));
    SRadiosityPatch sourceBackFacing = makePatchPorted(2, "source backfacing", Vector3f(-0.5f, -0.5f, 1.5f), Vector3f(0.5f, -0.5f, 1.5f), Vector3f(-0.5f, 0.5f, 1.5f), rgb(0.0f, 0.0f, 0.0f), rgb(3.0f, 3.0f, 3.0f));

    ok = expect(isVisiblePorted(receiver, sourceFacing), "facing source patch should be visible") && ok;
    ok = expect(!isVisiblePorted(receiver, sourceBackFacing), "back-facing source patch should not be visible") && ok;

    std::vector<SRadiosityPatch> visibilityScene = {receiver, sourceFacing, sourceBackFacing};
    calculateFormFactorScenePorted(visibilityScene);
    ok = expect(findFormFactor(visibilityScene[0], sourceFacing.id) > 0.0f, "facing source patch should produce a positive rasterized form factor") && ok;
    ok = expect(findFormFactor(visibilityScene[0], sourceBackFacing.id) == 0.0f, "back-facing source patch should produce a zero rasterized form factor") && ok;

    SRadiosityPatch depthReceiver = makePatchPorted(0, "depth receiver", Vector3f(-1.0f, -1.0f, 0.0f), Vector3f(1.0f, -1.0f, 0.0f), Vector3f(0.0f, 2.0f, 0.0f), rgb(0.6f, 0.6f, 0.6f));
    SRadiosityPatch farSource = makePatchPorted(1, "far overlapping source", Vector3f(-0.666667f, 0.666667f, 2.5f), Vector3f(0.666667f, -0.666667f, 2.5f), Vector3f(-0.666667f, -0.666667f, 2.5f), rgb(0.0f, 0.0f, 0.0f), rgb(2.0f, 2.0f, 2.0f));
    SRadiosityPatch nearSource = makePatchPorted(2, "near overlapping source", Vector3f(-0.4f, 0.4f, 1.5f), Vector3f(0.4f, -0.4f, 1.5f), Vector3f(-0.4f, -0.4f, 1.5f), rgb(0.0f, 0.0f, 0.0f), rgb(2.0f, 2.0f, 2.0f));
    std::vector<SRadiosityPatch> depthScene = {depthReceiver, farSource, nearSource};
    calculateFormFactorScenePorted(depthScene);
    ok = expect(findFormFactor(depthScene[0], nearSource.id) > 0.0f, "near overlapping source should be visible in the raster depth test") && ok;
    ok = expect(findFormFactor(depthScene[0], farSource.id) == 0.0f, "nearer overlapping source should replace the farther source in the raster depth test") && ok;

    std::vector<SRadiosityPatch> scene = makeCornellBoxScenePorted();
    SRadiositySummary summary = calculateFormFactorScenePorted(scene);
    gaussSeidelRadiosityPorted(scene, summary);

    bool nonEmissivePatchReceivedLight = false;
    for (const SRadiosityPatch &scenePatch : scene) {
        if (maxChannel(scenePatch.emissivity) <= kEpsilon && maxChannel(scenePatch.radiosity) > 0.05f) {
            nonEmissivePatchReceivedLight = true;
            break;
        }
    }

    ok = expect(summary.patchCount == static_cast<int>(scene.size()), "form-factor summary patch count mismatch") && ok;
    ok = expect(summary.nonZeroFormFactors > 0, "Cornell-box scene should create visible patch relationships") && ok;
    ok = expect(summary.iterations > 0, "Gauss-Seidel solver should run at least one iteration") && ok;
    ok = expect(nonEmissivePatchReceivedLight, "non-emissive patches should receive indirect/direct radiosity from the light") && ok;

    return ok;
}

bool runVisualExample(bool enterMainLoop) {
    std::vector<SRadiosityPatch> solvedScene = makeCornellBoxScenePorted();
    SRadiositySummary summary = calculateFormFactorScenePorted(solvedScene);
    gaussSeidelRadiosityPorted(solvedScene, summary);

    std::cout << "Patches: " << summary.patchCount << "\n";
    std::cout << "Visible form factors: " << summary.nonZeroFormFactors << "\n";
    std::cout << "Max form factor: " << summary.maxFormFactor << "\n";
    std::cout << "Max row coverage: " << summary.maxRowSum << "\n";
    std::cout << "Gauss-Seidel iterations: " << summary.iterations << "\n";
    std::cout << "Final max delta: " << summary.maxDelta << "\n";

    CCore engine(1200, 720, 32);
    Camera camera(1200, 720, 70.0f, 1.0f, 100.0f);
    camera.SetOrthographicIsometric(10.5f, 7.0f, 22.0f, Vector3f(0.0f, 0.0f, 1.4f), -45.0f, 33.0f);
    engine.SetCamera(camera);
    engine.Start();
    if (!engine.IsRunning()) {
        engine.Release();
        return false;
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
        engine.StepRender();
    }

    for (auto it = sceneObjects.rbegin(); it != sceneObjects.rend(); ++it) {
        engine.UnregisterSceneElement(**it);
    }
    sceneObjects.clear();
    engine.UnregisterEventListener(radiosityGroup);
    engine.Release();
    return true;
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
            return runVisualExample(false) ? 0 : 1;
        }
        if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--self-test] [--render-smoke-test]\n";
            return 0;
        }

        std::cerr << "Unknown argument: " << arg << "\n";
        std::cerr << "Usage: " << argv[0] << " [--self-test] [--render-smoke-test]\n";
        return 2;
    }

    return runVisualExample(true) ? 0 : 1;
}
