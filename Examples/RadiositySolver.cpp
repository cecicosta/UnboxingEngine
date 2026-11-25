#include "RadiositySolver.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace radiosity_demo {

using unboxing_engine::CMeshBuffer;

constexpr float kPi = 3.14159265358979323846f;

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

SRgb toneMap(const SRgb &color, float exposure) {
    return {
        clamp01(1.0f - std::exp(-std::max(0.0f, color.r) * exposure)),
        clamp01(1.0f - std::exp(-std::max(0.0f, color.g) * exposure)),
        clamp01(1.0f - std::exp(-std::max(0.0f, color.b) * exposure)),
    };
}

float triangleArea2D(float x0, float y0, float x1, float y1, float x2, float y2) {
    return std::fabs((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) * 0.5f;
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
    const SRgb &emissivity) {
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
    auto dist = source.centroid - receiver.centroid;
    auto distProjection = dist.DotProduct(receiver.normal);
    bool isVisible = distProjection > kEpsilon;

    auto normalProjection = receiver.normal.DotProduct(source.normal);
    isVisible = isVisible || normalProjection < 0;

    return isVisible;
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

struct SHemispherePixel {
    int patch_id = -1;
    float patch_z = std::numeric_limits<float>::max();
};

// Adapted from Editor/radiosity/original/hemiesfera.cpp:hemiEsfera.
//
// Original role:
// - Keep a receiver-owned 100x100 hemisphere grid.
// - Paint projected source patches into the grid.
// - Convert visible pixel ownership into form factors.
class CHemiEsferaPorted {
public:
    static constexpr int gridSize = 100;
    static constexpr float radius = 1.0f;

    explicit CHemiEsferaPorted(const SRadiosityPatch &myPatch)
        : my(&myPatch)
        , position(myPatch.centroid) {
        for (SHemispherePixel &pixel : grid) {
            pixel.patch_id = -1;
            pixel.patch_z = std::numeric_limits<float>::max();
        }
    }

    bool isVisible(const SRadiosityPatch &other) const {
        return isVisiblePorted(*my, other);
    }

    void paintGrid(const SRadiosityPatch &other) {
        if (my->id == other.id || my->area <= kEpsilon || other.area <= kEpsilon || !isVisible(other)) {
            return;
        }

        std::array<Vector3f, 3> projected{};
        for (std::size_t index = 0; index < other.vertex.size(); ++index) {
            if (!intersectHemisphereRayPorted(position, radius, position, other.vertex[index], projected[index])) {
                return;
            }

            if ((projected[index] - position).DotProduct(my->normal) <= kEpsilon) {
                return;
            }
        }

        std::array<Vector3f, 3> projectedOnPlane{};
        for (std::size_t index = 0; index < projected.size(); ++index) {
            Vector3f dist = projected[index] - my->vertex[0];
            Vector3f proj = my->normal * dist.DotProduct(my->normal);
            Vector3f distproj = projected[index] - (proj + my->vertex[0]);
            projectedOnPlane[index] = my->vertex[0] + distproj;
        }

        Vector3f x;
        Vector3f y;
        // hemiesfera.cpp referenced vertex[3], but patches have three vertices; use a receiver-normal basis instead.
        makeTangentBasis(my->normal, x, y);

        const float halfGrid = static_cast<float>(gridSize / 2);
        std::array<Vector3f, 3> gridPoints{};
        for (std::size_t index = 0; index < projectedOnPlane.size(); ++index) {
            float proj2D_x = (projectedOnPlane[index] - position).DotProduct(x);
            float proj2D_y = (projectedOnPlane[index] - position).DotProduct(y);
            gridPoints[index] = Vector3f(proj2D_y * halfGrid + halfGrid, proj2D_x * halfGrid + halfGrid, 0.0f);
        }

        int min_i = static_cast<int>(std::floor(std::min({gridPoints[0].x, gridPoints[1].x, gridPoints[2].x})));
        int max_i = static_cast<int>(std::ceil(std::max({gridPoints[0].x, gridPoints[1].x, gridPoints[2].x})));
        int min_j = static_cast<int>(std::floor(std::min({gridPoints[0].y, gridPoints[1].y, gridPoints[2].y})));
        int max_j = static_cast<int>(std::ceil(std::max({gridPoints[0].y, gridPoints[1].y, gridPoints[2].y})));

        // The original trusted these bounds; clamp them before indexing the fixed STL grid.
        min_i = clampGridIndex(min_i);
        max_i = clampGridIndex(max_i);
        min_j = clampGridIndex(min_j);
        max_j = clampGridIndex(max_j);

        float area = triangleArea2D(
            gridPoints[0].x,
            gridPoints[0].y,
            gridPoints[1].x,
            gridPoints[1].y,
            gridPoints[2].x,
            gridPoints[2].y);
        if (area <= kEpsilon) {
            return;
        }

        const float sourceDistance = (other.centroid - my->centroid).Length();
        for (int i = min_i; i <= max_i; ++i) {
            for (int j = min_j; j <= max_j; ++j) {
                Vector3f p(static_cast<float>(i), static_cast<float>(j), 0.0f);

                float area1 = triangleArea2D(p.x, p.y, gridPoints[0].x, gridPoints[0].y, gridPoints[1].x, gridPoints[1].y) / area;
                float area2 = triangleArea2D(p.x, p.y, gridPoints[0].x, gridPoints[0].y, gridPoints[2].x, gridPoints[2].y) / area;
                float area3 = triangleArea2D(p.x, p.y, gridPoints[2].x, gridPoints[2].y, gridPoints[1].x, gridPoints[1].y) / area;

                if (area1 + area2 + area3 <= 1.00001f) {
                    SHemispherePixel &pixel = grid[pixelIndex(i, j)];
                    if (sourceDistance < pixel.patch_z) {
                        // hemiesfera.cpp used objID and only filled empty pixels; this keeps patch ids and lets nearer patches replace farther ones.
                        pixel.patch_id = other.id;
                        pixel.patch_z = sourceDistance;
                    }
                }
            }
        }
    }

    float calculateFormFactor(std::vector<SViewedPatch> &viewedPatches) const {
        std::map<int, float> formFactorsByPatch;
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                const SHemispherePixel &pixel = grid[pixelIndex(i, j)];
                if (pixel.patch_id != -1) {
                    formFactorsByPatch[pixel.patch_id] += 1.0f;
                }
            }
        }

        viewedPatches.clear();
        float rowSum = 0.0f;
        const float halfGrid = static_cast<float>(gridSize / 2);
        const float normalizer = halfGrid * halfGrid * kPi;
        for (const auto &entry : formFactorsByPatch) {
            float formFactor = entry.second / normalizer;
            if (formFactor > kEpsilon) {
                viewedPatches.push_back({entry.first, formFactor});
                rowSum += formFactor;
            }
        }

        return rowSum;
    }

private:
    static int clampGridIndex(int value) {
        return std::max(0, std::min(gridSize - 1, value));
    }

    static std::size_t pixelIndex(int i, int j) {
        return static_cast<std::size_t>(i) * gridSize + static_cast<std::size_t>(j);
    }

    const SRadiosityPatch *my = nullptr;
    Vector3f position;
    std::array<SHemispherePixel, gridSize * gridSize> grid;
};

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
        CHemiEsferaPorted hemiEsfera(receiver);

        for (const SRadiosityPatch &source : patches) {
            hemiEsfera.paintGrid(source);
        }

        float rowSum = hemiEsfera.calculateFormFactor(receiver.patches);
        for (const SViewedPatch &viewedPatch : receiver.patches) {
            summary.maxFormFactor = std::max(summary.maxFormFactor, viewedPatch.formFactor);
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
void gaussSeidelRadiosityPorted(std::vector<SRadiosityPatch> &patches, SRadiositySummary &summary, int maxIterations, float tolerance) {
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

    for (std::size_t elementIndex = 0; elementIndex < meshElements.size(); ++elementIndex) {
        const auto &element = meshElements[elementIndex];
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
            patches.back().elementIndex = elementIndex;
        }
    }

    return patches;
}

std::vector<SRadiosityPatch> makeCornellBoxScenePorted() {
    std::vector<SCornellMeshElement> meshElements = makeCornellBoxMeshElements();
    return makePatchesFromMeshPorted(meshElements);
}

} // namespace radiosity_demo
