#include "BresenhamVoxelization.h"
#include "MeshPrimitivesUtils.h"

#include <emscripten/bind.h>

#include <algorithm>
#include <cstdint>
#include <memory>

namespace {

using namespace unboxing_engine;
using namespace voxelization_demo;

struct Result {
    int rays = 0;
    int intersections = 0;
    unsigned cells = 0;
    float cellSize = 0.0f;
    int minZ = 0;
    int maxZ = 0;
};

std::unique_ptr<CMeshBuffer> makeMesh(int shape) {
    switch (shape) {
    case 1:
        return primitive_utils::Sphere(3.0f, 24, 12);
    case 2: {
        std::unique_ptr<CMeshBuffer> cube(primitive_utils::Cube());
        for (float &coordinate : cube->vertices) {
            coordinate *= 5.5f;
        }
        return cube;
    }
    default:
        return primitive_utils::Torus(2.6f, 0.9f, 32, 12);
    }
}

struct VoxelizerState {
    std::vector<float> originalVertices;
    CMeshBuffer scaledMesh;
    std::vector<std::int32_t> packedCells;
    std::vector<float> columnHits;
    int limit = 0;
    float cellSize = 0.0f;

    Result voxelize(int shape, int resolution) {
        resolution = std::clamp(resolution, 12, 44);
        resolution -= resolution % 2;
        limit = resolution / 2;
        cellSize = 8.0f / resolution;
        columnHits.clear();

        const auto mesh = makeMesh(shape);
        originalVertices = mesh->vertices;
        scaledMesh.vertices = originalVertices;
        scaledMesh.triangles = mesh->triangles;
        for (float &coordinate : scaledMesh.vertices) {
            coordinate /= cellSize;
        }

        SVoxelMap map;
        map.sizeI = map.sizeJ = resolution + 1;
        const auto summary = createVoxelizedScenePorted(scaledMesh, map);
        Result result{summary.raysCast, summary.intersections, static_cast<unsigned>(summary.filledVoxels), cellSize};
        packedCells.clear();
        packedCells.reserve(map.voxels.size() * 3);
        if (!map.voxels.empty()) {
            result.minZ = result.maxZ = map.voxels.begin()->z;
        }
        for (const auto &voxel : map.voxels) {
            packedCells.insert(packedCells.end(), {voxel.x, voxel.y, voxel.z});
            result.minZ = std::min(result.minZ, voxel.z);
            result.maxZ = std::max(result.maxZ, voxel.z);
        }
        return result;
    }

    const std::vector<float> &column(int x, int y) {
        columnHits.clear();
        if (x < -limit || x >= limit || y < -limit || y >= limit) {
            return columnHits;
        }
        const auto hits = rayCollisionsWithMeshBufferPorted(
            Vector3f(static_cast<float>(x), static_cast<float>(y), -1000.0f),
            Vector3f(0.0f, 0.0f, 1.0f), scaledMesh);
        for (const auto &hit : hits) {
            columnHits.insert(columnHits.end(), {hit.x * cellSize, hit.y * cellSize, hit.z * cellSize});
        }
        return columnHits;
    }
};

VoxelizerState &state() {
    static VoxelizerState instance;
    return instance;
}

template <typename T>
emscripten::val view(const std::vector<T> &buffer) {
    return emscripten::val(emscripten::typed_memory_view(buffer.size(), buffer.data()));
}

} // namespace

EMSCRIPTEN_BINDINGS(voxelization_lab) {
    using namespace emscripten;
    value_object<Result>("VoxelizationResult")
        .field("rays", &Result::rays)
        .field("intersections", &Result::intersections)
        .field("cells", &Result::cells)
        .field("cellSize", &Result::cellSize)
        .field("minZ", &Result::minZ)
        .field("maxZ", &Result::maxZ);

    function("voxelize", +[](int shape, int resolution) { return state().voxelize(shape, resolution); });
    function("vertices", +[] { return view(state().originalVertices); });
    function("indices", +[] { return view(state().scaledMesh.triangles); });
    function("cells", +[] { return view(state().packedCells); });
    function("column", +[](int x, int y) { return view(state().column(x, y)); });
}
