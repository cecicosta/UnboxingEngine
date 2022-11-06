#pragma once

#include "MeshBuffer.h"
#include "SceneComposite.h"
#include "UVector.h"

#include <unordered_map>

namespace unboxing_engine {

class CCollisionManager {
public:
    std::unordered_map<int, const CMeshBuffer&> mMeshes;
    std::unordered_map<int, const SceneComposite&> mSceneComposite;


    void RegisterMesh(const CMeshBuffer &sceneNode, const SceneComposite &sceneComposite);
    static bool rayWithObject(Vector3f point, Vector3f ray, int ObjectID, Vector3f &intersection, face &poly);
    static void rayWithAllObjects(Vector3f point, Vector3f ray, List<int> &ObjectsID, List<Vector3f> &intersections, List<face> &faces);
    static bool betweenObjects(int id1, int id2, int parameter, List<Vector3f> &intersections, List<Vector3f> &triangles);
    static bool rayWithTriangle(Vector3f point, Vector3f ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f &intersection);

    static bool isInsideTriangle(Vector3f v1, Vector3f v2, Vector3f v3, Vector3f intersection);
    static bool sphereWithTriangle(sphere spr, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f &intersection);
};
}// namespace unboxing_engine