#include "MeshBuffer.h"
#include "RadiosityCornellBox.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "CoreEvents.h"
#include "internal_components/RenderComponent.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace unboxing_engine;

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kEpsilon = 0.00001f;

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
    SRgb emissivity;
    SRgb radiosity;
    SRgb reflectance;
    std::vector<SViewedPatch> patches;
};

struct SRadiositySummary {
    int patchCount = 0;
    int nonZeroFormFactors = 0;
    int iterations = 0;
    float maxFormFactor = 0.0f;
    float maxRowSum = 0.0f;
    float maxDelta = 0.0f;
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

// Adapted from Editor/radiosity/original/patch.cpp:patch::patch.
//
// Original role:
// - Store triangular patch vertices.
// - Compute patch normal, centroid, and area.
// - Store emissivity, reflectance, radiosity, and viewed patch data.
//
// UnboxingEngine adapter:
// - Uses Vector3f and value-owned STL containers instead of List/SceneElement.
// - Uses the standard triangle centroid formula to keep the standalone demo stable.
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
//
// Original role:
// - Reject patch pairs whose centroids/normals do not face each other.
bool isVisiblePorted(const SRadiosityPatch &receiver, const SRadiosityPatch &source) {
    Vector3f ray = receiver.centroid - source.centroid;
    float receiverProjection = ray.DotProduct(receiver.normal);
    float sourceProjection = ray.DotProduct(source.normal);
    return receiverProjection < -kEpsilon && sourceProjection > kEpsilon;
}

// Adapted from Editor/radiosity/original/hemiesfera.cpp:hemiEsfera::intersectionWithRay.
//
// Original role:
// - Intersect a ray with a unit hemisphere/sphere around the receiving patch.
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

void makeTangentBasis(const Vector3f &normal, Vector3f &axisX, Vector3f &axisY) {
    Vector3f reference = std::fabs(normal.z) < 0.9f ? Vector3f(0.0f, 0.0f, 1.0f) : Vector3f(0.0f, 1.0f, 0.0f);
    axisX = reference.CrossProduct(normal).Normalized();
    axisY = normal.CrossProduct(axisX).Normalized();
}

// Adapted from Editor/radiosity/original/hemiesfera.cpp:hemiEsfera::calculateFormFactor.
//
// Original role:
// - Project a visible patch onto a hemisphere centered at the receiver patch.
// - Project the hemisphere points onto the receiver plane and estimate area / pi.
//
// UnboxingEngine adapter:
// - Projects rays from the receiver centroid to the source patch vertices.
// - Stores one form factor per directed receiver/source patch pair.
float calculateProjectedFormFactorPorted(const SRadiosityPatch &receiver, const SRadiosityPatch &source) {
    if (receiver.id == source.id || receiver.area <= kEpsilon || source.area <= kEpsilon || !isVisiblePorted(receiver, source)) {
        return 0.0f;
    }

    Vector3f axisX;
    Vector3f axisY;
    makeTangentBasis(receiver.normal, axisX, axisY);

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
    float formFactor = projectedArea / kPi;

    if (!std::isfinite(formFactor) || formFactor <= kEpsilon) {
        return 0.0f;
    }
    return std::min(formFactor, 0.95f);
}

// Adapted from Editor/radiosity/original/formfactorscene.cpp:formFactorScene::formFactorScene.
//
// Original role:
// - Build one hemisphere per patch.
// - Paint/project other visible patches.
// - Store visible patch ids and their form factors.
SRadiositySummary calculateFormFactorScenePorted(std::vector<SRadiosityPatch> &patches) {
    SRadiositySummary summary;
    summary.patchCount = static_cast<int>(patches.size());

    for (SRadiosityPatch &receiver : patches) {
        receiver.patches.clear();
        float rowSum = 0.0f;

        for (const SRadiosityPatch &source : patches) {
            float formFactor = calculateProjectedFormFactorPorted(receiver, source);
            if (formFactor > kEpsilon) {
                receiver.patches.push_back({source.id, formFactor});
                rowSum += formFactor;
                summary.maxFormFactor = std::max(summary.maxFormFactor, formFactor);
            }
        }

        // The old rasterized hemisphere naturally bounded visible pixel coverage.
        // This compact projected-area adapter keeps the same stability expectation.
        constexpr float maxRowCoverage = 0.90f;
        if (rowSum > maxRowCoverage) {
            float scale = maxRowCoverage / rowSum;
            for (SViewedPatch &viewedPatch : receiver.patches) {
                viewedPatch.formFactor *= scale;
            }
            rowSum = maxRowCoverage;
        }

        summary.nonZeroFormFactors += static_cast<int>(receiver.patches.size());
        summary.maxRowSum = std::max(summary.maxRowSum, rowSum);
    }

    return summary;
}

// Adapted from Editor/radiosity/original/gaussseidel.cpp:GaussSeidel::resolver.
//
// Original role:
// - Perform one Gauss-Seidel style linear-system update.
//
// UnboxingEngine adapter:
// - Solves the radiosity recurrence directly:
//   B_i = E_i + rho_i * sum(F_ij * B_j)
// - Adds iteration and tolerance control so the standalone demo converges.
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

Vector3f meshVertex(const CMeshBuffer &mesh, unsigned int vertexIndex) {
    std::size_t offset = static_cast<std::size_t>(vertexIndex) * 3;
    return Vector3f(mesh.vertices[offset], mesh.vertices[offset + 1], mesh.vertices[offset + 2]);
}

// Adapted from Editor/radiosity/original/patch.cpp:patch::patchListFromObjectList.
//
// Original role:
// - Iterate SceneElement OBJ meshes.
// - Read each mesh triangle's transformed vertices.
// - Create one patch per triangle and copy the SceneElement material values.
//
// UnboxingEngine adapter:
// - Iterates local Cornell mesh elements instead of SceneElement/Resources.
// - Uses CMeshBuffer triangle indices as the source mesh data.
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

std::vector<SRadiosityPatch> makeCornellBoxScenePorted() {
    std::vector<SCornellMeshElement> meshElements = makeCornellBoxMeshElements();
    return makePatchesFromMeshPorted(meshElements);
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

    float positiveFormFactor = calculateProjectedFormFactorPorted(receiver, sourceFacing);
    float backFacingFormFactor = calculateProjectedFormFactorPorted(receiver, sourceBackFacing);
    ok = expect(positiveFormFactor > 0.0f, "facing source patch should produce a positive form factor") && ok;
    ok = expect(backFacingFormFactor == 0.0f, "back-facing source patch should produce a zero form factor") && ok;

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

void runVisualExample(bool enterMainLoop) {
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
    Camera camera(1200, 720, 70.0f, 1.0f, 1.0f);
    camera.SetOrthographicIsometric(10.5f, 7.0f, 22.0f, Vector3f(0.0f, 0.0f, 1.4f), -45.0f, 33.0f);
    engine.SetCamera(camera);
    engine.Start();

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
