#pragma once

#include "MeshBuffer.h"

#include <cstddef>
#include <set>
#include <vector>

namespace voxelization_demo {

struct SVoxel {
    int x = 0;
    int y = 0;
    int z = 0;

    bool operator<(const SVoxel &other) const {
        if (x != other.x) {
            return x < other.x;
        }
        if (y != other.y) {
            return y < other.y;
        }
        return z < other.z;
    }

    bool operator==(const SVoxel &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct SVoxelMap {
    int dim = 1;
    int sizeI = 6;
    int sizeJ = 6;
    std::set<SVoxel> voxels;
};

struct SVoxelizationSummary {
    int raysCast = 0;
    int intersections = 0;
    std::size_t filledVoxels = 0;
};

std::vector<Vector3f> rayCollisionsWithMeshBufferPorted(
    const Vector3f &point,
    const Vector3f &ray,
    const unboxing_engine::CMeshBuffer &mesh,
    bool firstIntersection = false);
std::vector<SVoxel> drawLine3DPorted(Vector3f p, Vector3f ray, int dim, int safetyLimit = 1024);
SVoxelizationSummary createVoxelizedScenePorted(
    const unboxing_engine::CMeshBuffer &mesh, SVoxelMap &map);

} // namespace voxelization_demo
