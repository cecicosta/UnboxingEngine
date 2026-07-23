#include "algorithms/LegacyOctreeAlgorithms.h"

#include <algorithm>

namespace unboxing_engine::algorithms {

namespace {

Vector3f minCorner(const CBoundingBox3D &box) {
    const Vector3f first = box.getFirst();
    const Vector3f second = box.getSecond();
    return {
        std::min(first.x, second.x),
        std::min(first.y, second.y),
        std::min(first.z, second.z)};
}

Vector3f maxCorner(const CBoundingBox3D &box) {
    const Vector3f first = box.getFirst();
    const Vector3f second = box.getSecond();
    return {
        std::max(first.x, second.x),
        std::max(first.y, second.y),
        std::max(first.z, second.z)};
}

bool containsTriangle(const std::vector<unsigned int> &triangles, unsigned int triangleIndex) {
    return std::find(triangles.begin(), triangles.end(), triangleIndex) != triangles.end();
}

void addUnique(std::vector<unsigned int> &triangles, unsigned int triangleIndex) {
    if (!containsTriangle(triangles, triangleIndex)) {
        triangles.push_back(triangleIndex);
    }
}

std::vector<unsigned int> allTriangleIndices(const CMeshBuffer &mesh) {
    std::vector<unsigned int> triangleIndices;
    triangleIndices.reserve(mesh.triangles.size() / 3);
    for (std::size_t triangleOffset = 0; triangleOffset + 2 < mesh.triangles.size(); triangleOffset += 3) {
        triangleIndices.push_back(static_cast<unsigned int>(triangleOffset / 3));
    }
    return triangleIndices;
}

std::array<std::unique_ptr<SMeshOctreeNode>, 8> subdivideOctPorted(const CBoundingBox3D &box) {
    std::array<std::unique_ptr<SMeshOctreeNode>, 8> children;

    const Vector3f min = minCorner(box);
    const Vector3f max = maxCorner(box);
    const Vector3f center = (min + max) / 2.0f;

    int childIndex = 0;
    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                const Vector3f childMin{
                    x == 0 ? min.x : center.x,
                    y == 0 ? min.y : center.y,
                    z == 0 ? min.z : center.z};
                const Vector3f childMax{
                    x == 0 ? center.x : max.x,
                    y == 0 ? center.y : max.y,
                    z == 0 ? center.z : max.z};

                auto child = std::make_unique<SMeshOctreeNode>();
                child->box = CBoundingBox3D(childMin, childMax);
                children[childIndex++] = std::move(child);
            }
        }
    }

    return children;
}

bool triangleHasVertexInsideOctPorted(
    const CMeshBuffer &mesh,
    unsigned int triangleIndex,
    const CBoundingBox3D &box) {

    const std::size_t triangleOffset = static_cast<std::size_t>(triangleIndex) * 3;
    if (triangleOffset + 2 >= mesh.triangles.size()) {
        return false;
    }

    for (std::size_t vertex = 0; vertex < 3; ++vertex) {
        const auto point = meshVertexPorted(mesh, mesh.triangles[triangleOffset + vertex]);
        if (point && isInsideBoundingBoxPorted(box, *point)) {
            return true;
        }
    }

    return false;
}

void createOctreePorted(
    const CMeshBuffer &mesh,
    const std::vector<unsigned int> &triangleIndices,
    SMeshOctreeNode &root,
    unsigned int maxFacesPerNode,
    unsigned int maxDepth,
    unsigned int depth) {

    // Ported from octree::createOctree in Legacy/octree.cpp.
    //
    // The legacy version stops when faces->size < MAX_FACES and otherwise
    // assigns a face to a child if any of its vertices is inside that octant.
    if (triangleIndices.size() <= maxFacesPerNode || depth >= maxDepth) {
        root.triangleIndices = triangleIndices;
        return;
    }

    root.children = subdivideOctPorted(root.box);

    std::array<std::vector<unsigned int>, 8> oct;
    std::vector<unsigned int> retained;
    for (unsigned int triangleIndex : triangleIndices) {
        std::vector<std::size_t> assignedChildren;
        for (std::size_t childIndex = 0; childIndex < root.children.size(); ++childIndex) {
            if (triangleHasVertexInsideOctPorted(mesh, triangleIndex, root.children[childIndex]->box)) {
                assignedChildren.push_back(childIndex);
            }
        }

        // The old implementation pushed a face to every child containing one of
        // its vertices. Large triangles then migrate toward corner leaves, so a
        // ray through the triangle interior can miss all candidate leaves. Keep
        // spanning triangles on the parent and only push fully localized
        // triangles down.
        if (assignedChildren.size() == 1) {
            oct[assignedChildren[0]].push_back(triangleIndex);
        } else {
            retained.push_back(triangleIndex);
        }
    }

    root.triangleIndices = retained;
    for (std::size_t childIndex = 0; childIndex < root.children.size(); ++childIndex) {
        if (oct[childIndex].empty()) {
            root.children[childIndex].reset();
            continue;
        }

        createOctreePorted(
            mesh,
            oct[childIndex],
            *root.children[childIndex],
            maxFacesPerNode,
            maxDepth,
            depth + 1);
    }
}

void collectRayCandidates(const SMeshOctreeNode &node, const SRay3D &ray, std::vector<unsigned int> &candidateTriangles) {
    if (!rayWithBoundingBoxPorted(node.box, ray)) {
        return;
    }

    for (unsigned int triangleIndex : node.triangleIndices) {
        addUnique(candidateTriangles, triangleIndex);
    }

    for (const auto &child : node.children) {
        if (child) {
            collectRayCandidates(*child, ray, candidateTriangles);
        }
    }
}

void collectSphereCandidates(const SMeshOctreeNode &node, const SSphere3D &sphere, std::vector<unsigned int> &candidateTriangles) {
    if (!sphereWithBoundingBoxPorted(node.box, sphere)) {
        return;
    }

    for (unsigned int triangleIndex : node.triangleIndices) {
        addUnique(candidateTriangles, triangleIndex);
    }

    for (const auto &child : node.children) {
        if (child) {
            collectSphereCandidates(*child, sphere, candidateTriangles);
        }
    }
}

void addHitSorted(std::vector<STriangleHit3D> &hits, const STriangleHit3D &hit) {
    auto it = std::lower_bound(hits.begin(), hits.end(), hit.distance, [](const STriangleHit3D &candidate, float distance) {
        return candidate.distance < distance;
    });
    hits.insert(it, hit);
}

} // namespace

bool SMeshOctreeNode::IsLeaf() const {
    return std::none_of(children.begin(), children.end(), [](const auto &child) {
        return child != nullptr;
    });
}

SMeshOctree buildMeshOctreePorted(
    const CMeshBuffer &mesh,
    unsigned int maxFacesPerNode,
    unsigned int maxDepth) {

    SMeshOctree octree;
    octree.maxFacesPerNode = maxFacesPerNode;
    octree.maxDepth = maxDepth;
    octree.root = std::make_unique<SMeshOctreeNode>();
    octree.root->box = mesh.boundingBox;

    createOctreePorted(mesh, allTriangleIndices(mesh), *octree.root, maxFacesPerNode, maxDepth, 0);
    return octree;
}

std::vector<unsigned int> queryMeshOctreeRayPorted(
    const SMeshOctree &octree,
    const SRay3D &ray) {

    std::vector<unsigned int> candidateTriangles;
    if (octree.root) {
        collectRayCandidates(*octree.root, ray, candidateTriangles);
    }
    return candidateTriangles;
}

std::vector<unsigned int> queryMeshOctreeSpherePorted(
    const SMeshOctree &octree,
    const SSphere3D &sphere) {

    std::vector<unsigned int> candidateTriangles;
    if (octree.root) {
        collectSphereCandidates(*octree.root, sphere, candidateTriangles);
    }
    return candidateTriangles;
}

std::vector<STriangleHit3D> rayWithMeshOctreePorted(
    const CMeshBuffer &mesh,
    const SMeshOctree &octree,
    const SRay3D &ray,
    bool firstIntersection,
    std::size_t *candidateTriangleCount) {

    std::vector<STriangleHit3D> hits;
    const auto candidateTriangles = queryMeshOctreeRayPorted(octree, ray);
    if (candidateTriangleCount != nullptr) {
        *candidateTriangleCount += candidateTriangles.size();
    }

    for (unsigned int triangleIndex : candidateTriangles) {
        const std::size_t triangleOffset = static_cast<std::size_t>(triangleIndex) * 3;
        if (triangleOffset + 2 >= mesh.triangles.size()) {
            continue;
        }

        const auto v1 = meshVertexPorted(mesh, mesh.triangles[triangleOffset]);
        const auto v2 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 1]);
        const auto v3 = meshVertexPorted(mesh, mesh.triangles[triangleOffset + 2]);
        if (!v1 || !v2 || !v3) {
            continue;
        }

        auto hit = rayWithTrianglePorted(ray, *v1, *v2, *v3, triangleIndex);
        if (hit) {
            addHitSorted(hits, *hit);
        }
    }

    if (firstIntersection && hits.size() > 1) {
        hits.resize(1);
    }

    return hits;
}

} // namespace unboxing_engine::algorithms
