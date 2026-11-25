#include "MeshPrimitivesUtils.h"
#include "algorithms/LegacyOctreeAlgorithms.h"

#include <emscripten/bind.h>

#include <algorithm>
#include <cstdint>
#include <unordered_map>

namespace {

using namespace unboxing_engine;
using namespace unboxing_engine::algorithms;

enum NodeState : std::uint8_t {
    Inactive, Visited, Candidate, FirstHit
};

struct ShotResult {
    int triangle = -1;
    unsigned visited = 0;
    unsigned candidates = 0;
    float distance = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Lab {
    std::unique_ptr<CMeshBuffer> mesh = primitive_utils::Torus(4.0f, 1.15f, 40, 14);
    SMeshOctree tree = buildMeshOctreePorted(*mesh, 10, 7);
    std::unordered_map<const SMeshOctreeNode *, std::size_t> ids;
    std::vector<float> bounds;
    std::vector<std::uint8_t> states;
    std::vector<const SMeshOctreeNode *> visited;

    Lab() {
        collect(*tree.root);
    }

    void collect(const SMeshOctreeNode &node) {
        ids.emplace(&node, states.size());
        states.push_back(Inactive);
        const auto min = node.box.getFirst();
        const auto max = node.box.getSecond();
        bounds.insert(bounds.end(), {min.x, min.y, min.z, max.x, max.y, max.z});
        for (const auto &child : node.children) {
            if (child) {
                collect(*child);
            }
        }
    }

    ShotResult shoot(const SRay3D &ray) {
        std::fill(states.begin(), states.end(), Inactive);
        std::size_t candidates = 0;
        const auto hits = rayWithMeshOctreePorted(*mesh, tree, ray, true, &candidates, &visited);
        ShotResult result;
        result.visited = static_cast<unsigned>(visited.size());
        result.candidates = static_cast<unsigned>(candidates);
        if (!hits.empty()) {
            const auto &hit = hits.front();
            result.triangle = static_cast<int>(hit.triangleIndex);
            result.distance = hit.distance;
            result.x = hit.intersection.x;
            result.y = hit.intersection.y;
            result.z = hit.intersection.z;
        }
        for (const auto *node : visited) {
            const auto &indices = node->triangleIndices;
            auto &state = states[ids.at(node)];
            state = indices.empty() ? Visited : Candidate;
            if (result.triangle >= 0 &&
                std::find(indices.begin(), indices.end(), static_cast<unsigned>(result.triangle)) != indices.end()) {
                state = FirstHit;
            }
        }
        return result;
    }
};

Lab &lab() {
    static Lab instance;
    return instance;
}

template <typename T>
emscripten::val view(const std::vector<T> &buffer) {
    return emscripten::val(emscripten::typed_memory_view(buffer.size(), buffer.data()));
}

} // namespace

EMSCRIPTEN_BINDINGS(octree_ray_lab) {
    using namespace emscripten;
    value_object<ShotResult>("ShotResult")
        .field("triangle", &ShotResult::triangle)
        .field("visited", &ShotResult::visited)
        .field("candidates", &ShotResult::candidates)
        .field("distance", &ShotResult::distance)
        .field("x", &ShotResult::x)
        .field("y", &ShotResult::y)
        .field("z", &ShotResult::z);

    function("vertices", +[] { return view(lab().mesh->vertices); });
    function("indices", +[] { return view(lab().mesh->triangles); });
    function("nodeBounds", +[] { return view(lab().bounds); });
    function("nodeStates", +[] { return view(lab().states); });
    function("shoot", +[](float x, float y, float z, float dx, float dy, float dz) {
        const Vector3f direction(dx, dy, dz);
        return lab().shoot({Vector3f(x, y, z), direction.Normalized()});
    });
}
