#include "MeshBuffer.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "algorithms/CollisionAlgorithms.h"
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
using unboxing_engine::algorithms::findPathIntersectionWithSegment;

namespace {

constexpr float kDuplicateTolerance = 0.0001f;

struct STriangleCase {
    std::string name;
    std::array<Vector3f, 3> base;
    std::array<Vector3f, 3> candidate;
    Vector3f offset;
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

std::unique_ptr<CMeshBuffer> makeMesh(const std::vector<Vector3f> &vertices, const std::vector<unsigned int> &triangles) {
    auto mesh = std::make_unique<CMeshBuffer>();
    mesh->nvertices = static_cast<uint>(vertices.size());
    mesh->nnormals = 0;
    mesh->ntexcoords = 0;
    mesh->nmaterials = 0;
    mesh->nfaces = static_cast<uint>(triangles.size() / 3);
    mesh->triangles = triangles;
    mesh->faces.resize(mesh->nfaces);
    mesh->vertices.reserve(vertices.size() * 3);

    if (vertices.empty()) {
        mesh->boundingBox = CBoundingBox3D(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f));
        return mesh;
    }

    Vector3f minBounds = vertices[0];
    Vector3f maxBounds = vertices[0];
    for (const Vector3f &vertex : vertices) {
        mesh->vertices.push_back(vertex.x);
        mesh->vertices.push_back(vertex.y);
        mesh->vertices.push_back(vertex.z);
        minBounds = Vector3f(std::min(minBounds.x, vertex.x), std::min(minBounds.y, vertex.y), std::min(minBounds.z, vertex.z));
        maxBounds = Vector3f(std::max(maxBounds.x, vertex.x), std::max(maxBounds.y, vertex.y), std::max(maxBounds.z, vertex.z));
    }

    mesh->boundingBox = CBoundingBox3D(minBounds, maxBounds);
    return mesh;
}

std::unique_ptr<CMeshBuffer> makeTriangleMesh(const std::array<Vector3f, 3> &triangle, const Vector3f &offset, float zOffset) {
    return makeMesh(
        {
            triangle[0] + offset + Vector3f(0.0f, 0.0f, zOffset),
            triangle[1] + offset + Vector3f(0.0f, 0.0f, zOffset),
            triangle[2] + offset + Vector3f(0.0f, 0.0f, zOffset),
        },
        {0, 1, 2});
}

std::unique_ptr<CMeshBuffer> makePolygonMesh(const std::vector<Vector3f> &polygon, const Vector3f &offset, float zOffset) {
    if (polygon.size() < 3) {
        return nullptr;
    }

    std::vector<Vector3f> vertices;
    vertices.reserve(polygon.size());
    for (const Vector3f &vertex : polygon) {
        vertices.push_back(vertex + offset + Vector3f(0.0f, 0.0f, zOffset));
    }

    std::vector<unsigned int> triangles;
    for (unsigned int i = 1; i + 1 < vertices.size(); ++i) {
        triangles.push_back(0);
        triangles.push_back(i);
        triangles.push_back(i + 1);
    }

    return makeMesh(vertices, triangles);
}

std::unique_ptr<CMeshBuffer> makeMarkerMesh(const Vector3f &center, const Vector3f &offset, float size, float zOffset) {
    const Vector3f c = center + offset + Vector3f(0.0f, 0.0f, zOffset);
    return makeMesh(
        {
            Vector3f(c.x - size, c.y - size, c.z),
            Vector3f(c.x + size, c.y - size, c.z),
            Vector3f(c.x + size, c.y + size, c.z),
            Vector3f(c.x - size, c.y + size, c.z),
        },
        {0, 1, 2, 0, 2, 3});
}

std::vector<STriangleCase> makeCases() {
    const std::array<Vector3f, 3> base = {
        Vector3f(-1.0f, -1.0f, 0.0f),
        Vector3f(1.0f, -1.0f, 0.0f),
        Vector3f(0.0f, 1.0f, 0.0f),
    };

    return {
        {
            "contained candidate",
            base,
            {
                Vector3f(-0.25f, -0.45f, 0.0f),
                Vector3f(0.35f, -0.45f, 0.0f),
                Vector3f(0.0f, 0.15f, 0.0f),
            },
            Vector3f(-2.8f, 1.7f, 0.0f),
        },
        {
            "edge crossing, no candidate vertex inside",
            base,
            {
                Vector3f(-1.45f, 0.0f, 0.0f),
                Vector3f(1.45f, 0.0f, 0.0f),
                Vector3f(0.0f, 1.45f, 0.0f),
            },
            Vector3f(2.8f, 1.7f, 0.0f),
        },
        {
            "one candidate vertex inside",
            base,
            {
                Vector3f(0.0f, -0.10f, 0.0f),
                Vector3f(1.45f, 0.35f, 0.0f),
                Vector3f(1.20f, -1.35f, 0.0f),
            },
            Vector3f(-2.8f, -1.7f, 0.0f),
        },
        {
            "two candidate vertices inside",
            base,
            {
                Vector3f(-0.35f, -0.35f, 0.0f),
                Vector3f(0.30f, -0.25f, 0.0f),
                Vector3f(1.55f, 0.10f, 0.0f),
            },
            Vector3f(2.8f, -1.7f, 0.0f),
        },
    };
}

void addUniquePoint(std::vector<Vector3f> &points, const Vector3f &point) {
    auto alreadyPresent = std::find_if(points.begin(), points.end(), [&point](const Vector3f &candidate) {
        return (candidate - point).Length() <= kDuplicateTolerance;
    });
    if (alreadyPresent == points.end()) {
        points.push_back(point);
    }
}

std::vector<Vector3f> sortByAngle(std::vector<Vector3f> points) {
    if (points.size() < 3) {
        return points;
    }

    Vector3f center(0.0f, 0.0f, 0.0f);
    for (const Vector3f &point : points) {
        center = center + point;
    }
    center = center / static_cast<float>(points.size());

    std::sort(points.begin(), points.end(), [&center](const Vector3f &a, const Vector3f &b) {
        return std::atan2(a.y - center.y, a.x - center.x) < std::atan2(b.y - center.y, b.x - center.x);
    });
    return points;
}

void printPoint(const Vector3f &point) {
    std::cout << "(" << point.x << ", " << point.y << ", " << point.z << ")";
}

std::vector<Vector3f> collectRawResultVertices(const STriangleCase &triangleCase, std::vector<Vector3f> &intersectionPoints) {
    std::vector<Vector3f> rawVertices;

    std::cout << triangleCase.name << "\n";
    for (std::size_t i = 0; i < triangleCase.candidate.size(); ++i) {
        const Vector3f &start = triangleCase.candidate[i];
        const Vector3f &end = triangleCase.candidate[(i + 1) % triangleCase.candidate.size()];
        const Vector3f &dir = end - start;


        auto result = algorithms::findIntersectionBetweenLines()<float, 3>(
            {triangleCase.base[0], triangleCase.base[1], triangleCase.base[2]},
            start,
            end);

        std::cout << "  edge " << i << ": returned vertices=" << result.vertices.size();
        if (!result.vertices.empty()) {
            std::cout << ", returned={";
            for (std::size_t vertexIndex = 0; vertexIndex < result.vertices.size(); ++vertexIndex) {
                if (vertexIndex > 0) {
                    std::cout << ", ";
                }
                printPoint(result.vertices[vertexIndex]);
            }
            std::cout << "}, intersection=";
            printPoint(result.intersection);
            addUniquePoint(intersectionPoints, result.intersection);
        }
        std::cout << "\n";

        for (const Vector3f &vertex : result.vertices) {
            addUniquePoint(rawVertices, vertex);
        }
    }

    std::vector<Vector3f> sorted = sortByAngle(rawVertices);
    std::cout << "  hypothetical polygon vertices=" << sorted.size() << "\n";
    return sorted;
}

class CMeshSceneObject : public CSceneComposite {
public:
    CMeshSceneObject(std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, EPolygonMode polygonMode)
        : mMesh(std::move(mesh)) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        render->SetMaterial(mat);
        render->SetPolygonMode(polygonMode);
        AddComponent<IRenderComponent>(std::move(render));
    }

private:
    std::unique_ptr<CMeshBuffer> mMesh;
};

void runExample(bool summaryOnly) {
    std::vector<STriangleCase> cases = makeCases();
    std::vector<std::vector<Vector3f>> hypotheticalPolygons;
    std::vector<std::vector<Vector3f>> intersectionMarkers;
    hypotheticalPolygons.reserve(cases.size());
    intersectionMarkers.reserve(cases.size());

    for (const STriangleCase &triangleCase : cases) {
        std::vector<Vector3f> intersections;
        hypotheticalPolygons.push_back(collectRawResultVertices(triangleCase, intersections));
        intersectionMarkers.push_back(std::move(intersections));
    }

    if (summaryOnly) {
        return;
    }

    CCore engine(1280, 760, 32);
    Camera camera(1280, 760, 70.0f, 1.0f, 1.0f);
    camera.SetOrthographicIsometric(8.4f, 5.6f, 8.0f, Vector3f(0.0f, 0.0f, 0.0f), 0.0f, 90.0f);
    engine.SetCamera(camera);
    engine.Start();
    if (!engine.IsRunning()) {
        return;
    }

    std::vector<std::unique_ptr<CMeshSceneObject>> sceneObjects;
    auto addObject = [&engine, &sceneObjects](std::unique_ptr<CMeshBuffer> mesh, const SMaterial &mat, EPolygonMode polygonMode) {
        if (!mesh) {
            return;
        }
        auto sceneObject = std::make_unique<CMeshSceneObject>(std::move(mesh), mat, polygonMode);
        sceneObjects.push_back(std::move(sceneObject));
        engine.RegisterSceneElement(*sceneObjects.back());
    };

    for (std::size_t i = 0; i < cases.size(); ++i) {
        const STriangleCase &triangleCase = cases[i];
        addObject(makePolygonMesh(hypotheticalPolygons[i], triangleCase.offset, -0.03f), material(0.90f, 0.78f, 0.20f), EPolygonMode::Fill);
        addObject(makeTriangleMesh(triangleCase.base, triangleCase.offset, 0.00f), material(0.18f, 0.18f, 0.20f), EPolygonMode::Line);
        addObject(makeTriangleMesh(triangleCase.candidate, triangleCase.offset, 0.02f), material(0.95f, 0.20f, 0.18f), EPolygonMode::Line);

        for (const Vector3f &intersection : intersectionMarkers[i]) {
            addObject(makeMarkerMesh(intersection, triangleCase.offset, 0.045f, 0.05f), material(0.05f, 0.45f, 1.0f), EPolygonMode::Fill);
        }
    }

    engine.Run();

    for (auto it = sceneObjects.rbegin(); it != sceneObjects.rend(); ++it) {
        engine.UnregisterSceneElement(**it);
    }
    engine.Release();
}

} // namespace

int main(int argc, char *argv[]) {
    bool summaryOnly = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--summary-only") {
            summaryOnly = true;
            continue;
        }
        if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--summary-only]\n";
            return 0;
        }
        std::cerr << "Unknown argument: " << arg << "\n";
        std::cerr << "Usage: " << argv[0] << " [--summary-only]\n";
        return 2;
    }

    runExample(summaryOnly);
    return 0;
}
