#include "RadiosityCornellBox.h"

#include "UVector.h"

#include <algorithm>
#include <utility>

using namespace unboxing_engine;

namespace radiosity_demo {
namespace {

SRgb rgb(float r, float g, float b) {
    return {r, g, b};
}

SRgb operator*(const SRgb &color, float value) {
    return {color.r * value, color.g * value, color.b * value};
}

Vector3f minVector(const Vector3f &a, const Vector3f &b) {
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

Vector3f maxVector(const Vector3f &a, const Vector3f &b) {
    return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

Vector3f interpolateQuad(const Vector3f &v0, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3, float u, float v) {
    return v0 * ((1.0f - u) * (1.0f - v)) + v1 * (u * (1.0f - v)) + v2 * (u * v) + v3 * ((1.0f - u) * v);
}

std::unique_ptr<CMeshBuffer> makeEmptyMeshBuffer() {
    auto mesh = std::make_unique<CMeshBuffer>();
    mesh->nvertices = 0;
    mesh->nnormals = 0;
    mesh->ntexcoords = 0;
    mesh->nmaterials = 0;
    mesh->nfaces = 0;
    return mesh;
}

unsigned int appendMeshVertex(CMeshBuffer &mesh, const Vector3f &vertex) {
    unsigned int vertexIndex = mesh.nvertices++;
    mesh.vertices.push_back(vertex.x);
    mesh.vertices.push_back(vertex.y);
    mesh.vertices.push_back(vertex.z);
    return vertexIndex;
}

void appendMeshTriangle(CMeshBuffer &mesh, const Vector3f &v0, const Vector3f &v1, const Vector3f &v2) {
    unsigned int index0 = appendMeshVertex(mesh, v0);
    unsigned int index1 = appendMeshVertex(mesh, v1);
    unsigned int index2 = appendMeshVertex(mesh, v2);
    mesh.triangles.push_back(index0);
    mesh.triangles.push_back(index1);
    mesh.triangles.push_back(index2);
    ++mesh.nfaces;
}

void appendMeshQuad(CMeshBuffer &mesh, const Vector3f &v0, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3) {
    appendMeshTriangle(mesh, v0, v1, v2);
    appendMeshTriangle(mesh, v0, v2, v3);
}

void finalizeMeshBounds(CMeshBuffer &mesh) {
    if (mesh.vertices.empty()) {
        mesh.boundingBox = CBoundingBox3D(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 0.0f));
        return;
    }

    Vector3f minBounds(mesh.vertices[0], mesh.vertices[1], mesh.vertices[2]);
    Vector3f maxBounds = minBounds;
    for (std::size_t i = 3; i + 2 < mesh.vertices.size(); i += 3) {
        Vector3f vertex(mesh.vertices[i], mesh.vertices[i + 1], mesh.vertices[i + 2]);
        minBounds = minVector(minBounds, vertex);
        maxBounds = maxVector(maxBounds, vertex);
    }

    mesh.faces.resize(mesh.nfaces);
    mesh.boundingBox = CBoundingBox3D(minBounds, maxBounds);
}

std::unique_ptr<CMeshBuffer> makeSubdividedQuadMesh(
    const Vector3f &v0,
    const Vector3f &v1,
    const Vector3f &v2,
    const Vector3f &v3,
    unsigned int columns,
    unsigned int rows) {
    columns = std::max(1u, columns);
    rows = std::max(1u, rows);

    std::unique_ptr<CMeshBuffer> mesh = makeEmptyMeshBuffer();
    for (unsigned int row = 0; row < rows; ++row) {
        float vStart = static_cast<float>(row) / static_cast<float>(rows);
        float vEnd = static_cast<float>(row + 1) / static_cast<float>(rows);

        for (unsigned int column = 0; column < columns; ++column) {
            float uStart = static_cast<float>(column) / static_cast<float>(columns);
            float uEnd = static_cast<float>(column + 1) / static_cast<float>(columns);

            Vector3f p00 = interpolateQuad(v0, v1, v2, v3, uStart, vStart);
            Vector3f p10 = interpolateQuad(v0, v1, v2, v3, uEnd, vStart);
            Vector3f p11 = interpolateQuad(v0, v1, v2, v3, uEnd, vEnd);
            Vector3f p01 = interpolateQuad(v0, v1, v2, v3, uStart, vEnd);
            appendMeshQuad(*mesh, p00, p10, p11, p01);
        }
    }

    finalizeMeshBounds(*mesh);
    return mesh;
}

SCornellMeshElement makeCornellMeshElement(std::string name, std::unique_ptr<CMeshBuffer> mesh, const SRgb &reflectance, const SRgb &emissivity = {}) {
    return {std::move(name), std::move(mesh), reflectance, emissivity};
}

}  // namespace

std::vector<SCornellMeshElement> makeCornellBoxMeshElements() {
    std::vector<SCornellMeshElement> elements;
    elements.reserve(6);

    const SRgb grey = rgb(0.68f, 0.68f, 0.66f);
    const SRgb red = rgb(0.78f, 0.16f, 0.12f);
    const SRgb green = rgb(0.15f, 0.68f, 0.20f);
    const SRgb lightReflectance = rgb(0.05f, 0.05f, 0.05f);
    const SRgb lightEmission = rgb(4.8f, 4.3f, 3.4f) * 2.0f;

    elements.push_back(makeCornellMeshElement("floor", makeSubdividedQuadMesh(Vector3f(-2.0f, -2.0f, 0.0f), Vector3f(2.0f, -2.0f, 0.0f), Vector3f(2.0f, 2.0f, 0.0f), Vector3f(-2.0f, 2.0f, 0.0f), 8, 8), grey));
    elements.push_back(makeCornellMeshElement("back wall", makeSubdividedQuadMesh(Vector3f(-2.0f, 2.0f, 0.0f), Vector3f(2.0f, 2.0f, 0.0f), Vector3f(2.0f, 2.0f, 3.0f), Vector3f(-2.0f, 2.0f, 3.0f), 8, 6), grey));
    elements.push_back(makeCornellMeshElement("left wall", makeSubdividedQuadMesh(Vector3f(-2.0f, -2.0f, 0.0f), Vector3f(-2.0f, 2.0f, 0.0f), Vector3f(-2.0f, 2.0f, 3.0f), Vector3f(-2.0f, -2.0f, 3.0f), 8, 6), red));
    elements.push_back(makeCornellMeshElement("right wall", makeSubdividedQuadMesh(Vector3f(2.0f, 2.0f, 0.0f), Vector3f(2.0f, -2.0f, 0.0f), Vector3f(2.0f, -2.0f, 3.0f), Vector3f(2.0f, 2.0f, 3.0f), 8, 6), green));
    elements.push_back(makeCornellMeshElement("ceiling", makeSubdividedQuadMesh(Vector3f(-2.0f, 2.0f, 3.0f), Vector3f(2.0f, 2.0f, 3.0f), Vector3f(2.0f, -2.0f, 3.0f), Vector3f(-2.0f, -2.0f, 3.0f), 8, 8), grey * 0.85f));
    elements.push_back(makeCornellMeshElement("emissive ceiling patch", makeSubdividedQuadMesh(Vector3f(-0.55f, 0.55f, 2.92f), Vector3f(0.55f, 0.55f, 2.92f), Vector3f(0.55f, -0.55f, 2.92f), Vector3f(-0.55f, -0.55f, 2.92f), 3, 3), lightReflectance, lightEmission));

    return elements;
}

}  // namespace radiosity_demo
