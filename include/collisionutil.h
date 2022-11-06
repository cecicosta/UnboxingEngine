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
    static bool rayWithObject(vector3D point, vector3D ray, int ObjectID, vector3D &intersection, face &poly);
    static void rayWithAllObjects(vector3D point, vector3D ray, List<int> &ObjectsID, List<vector3D> &intersections, List<face> &faces);
    static bool betweenObjects(int id1, int id2, int parameter, List<vector3D> &intersections, List<vector3D> &triangles);
    static bool rayWithTriangle(vector3D point, vector3D ray, vector3D v1, vector3D v2, vector3D v3, vector3D &intersection);

    static bool isInsideTriangle(vector3D v1, vector3D v2, vector3D v3, vector3D intersection);
    static bool sphereWithTriangle(sphere spr, vector3D v1, vector3D v2, vector3D v3, vector3D &intersection);
};
}// namespace unboxing_engine