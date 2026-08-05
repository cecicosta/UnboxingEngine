#include "COctree.h"
#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "SceneCompositeLibrary/BasicSceneCompositeTypes.h"
#include "UnboxingEngine.h"
#include "algorithms/OctreeCollision.h"
#include "internal_components/IRenderComponent.h"
#include "internal_components/RenderComponent.h"
#include "internal_components/SegmentColliderComponent.h"

#include <cmath>
#include <memory>

using namespace unboxing_engine;

SMaterial torusMaterial() {
    SMaterial material{};
    material.materialDif[0] = 0.20f;
    material.materialDif[1] = 0.62f;
    material.materialDif[2] = 0.78f;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}

SMaterial octreeMaterial() {
    SMaterial material{};
    material.materialDif[0] = 0.78f;
    material.materialDif[1] = 0.62f;
    material.materialDif[2] = 0.20f;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}

class CTorusSceneObject : public CSceneComposite
    , public UListener<core_events::IMouseInputEvent>{
public:
    explicit CTorusSceneObject(CCore& engine)
    : mEngine(engine)
    , mMesh(primitive_utils::Torus(4.0f, 1.15f, 40, 14))
    , mOctree(std::make_unique<COctree>(*mMesh, 10, 6)) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        render->SetMaterial(torusMaterial());
        render->SetPolygonMode(EPolygonMode::Line);
        AddComponent<IRenderComponent>(std::move(render));

        assert(mOctree->root);

        mOctreeMesh = mOctree->Debug_CreateBoundingBoxesMesh(0, 4);
        mOctreeObject = std::make_unique<CSimpleMeshWireFrame>(*mOctreeMesh);
        mOctreeObject->SetMaterial(octreeMaterial());
        mRay = std::make_unique<CSegment>(mEngine, blueMaterial());
        mMark = std::make_unique<CSimpleMeshWireFrame>(*primitive_utils::Cylinder(Vector3f(), Vector3f(0,1,0), 1, 1, 6));
        mMark->SetParent(mRay.get());

        mEngine.RegisterSceneElement(*mMark);
        mEngine.RegisterSceneElement(*mRay);
        mEngine.RegisterSceneElement(*mOctreeObject);
    }

    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        if (!cursor.isButtonPressed) {
            mRay->SetMaterial(redMaterial());
            mMark->SetMaterial(redMaterial());
            return;
        }
        Vector3f outPoint, outDirection;
        mEngine.GetCamera().CastRayFromScreen(cursor.buttonPressedX, cursor.buttonPressedY, outPoint, outDirection);

        mRay->SetMaterial(yellowMaterial());
        mMark->SetMaterial(yellowMaterial());
        mRay->UpdateSegment((outPoint + outDirection*900) , outPoint );

        CMeshBuffer mesh;
        auto hit = RayWithOctree(outPoint,  outDirection, *mOctree, &mesh);
        if (hit.hit) {
            mEngine.UnregisterSceneElement(*mMark);
            mEngine.UnregisterSceneElement(*mRay);
            mEngine.UnregisterSceneElement(*mOctreeObject);

            mOctreeDebugMesh = std::make_unique<CMeshBuffer>(mesh);
            mRay->SetPosition(Vector3f(hit.intersection));
            mOctreeObject = std::make_unique<CSimpleMeshWireFrame>(*mOctreeDebugMesh);
            mOctreeObject->SetMaterial(octreeMaterial());

            mEngine.RegisterSceneElement(*mOctreeObject);
            mEngine.RegisterSceneElement(*mRay);
            mEngine.RegisterSceneElement(*mMark);
        }
    }

    ~CTorusSceneObject() override {
        mEngine.UnregisterSceneElement(*mOctreeObject);
        mEngine.UnregisterSceneElement(*mRay);
    }
private:
    CCore& mEngine;
    std::unique_ptr<CMeshBuffer> mMesh;
    std::unique_ptr<COctree> mOctree;
    std::unique_ptr<CMeshBuffer> mOctreeMesh;
    std::unique_ptr<CSegment> mRay;
    std::unique_ptr<CSimpleMeshWireFrame> mOctreeObject;
    std::unique_ptr<CSimpleMeshWireFrame> mMark;
    std::unique_ptr<CMeshBuffer> mOctreeDebugMesh;
};

int main() {
    CCore engine(1000, 700, 32);
    Camera camera(1000, 700, 10.f, 10.0f, 1000.0f);
    camera.SetPosition(Vector3f(0, -50, 50));
    camera.SetRotation(Vector3f(1, 0, 0), 45);
    engine.SetCamera(camera);

    engine.Start();

    CTorusSceneObject torusObject(engine);

    if (engine.IsRunning()) {
        engine.RegisterSceneElement(torusObject);
        engine.Run();
        engine.UnregisterSceneElement(torusObject);
    }

    engine.Release();
    return 0;
}
