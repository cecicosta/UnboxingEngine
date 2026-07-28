#include "COctree.h"
#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "algorithms/OctreeCollision.h"
#include "internal_components/IRenderComponent.h"
#include "internal_components/RenderComponent.h"
#include "internal_components/SegmentColliderComponent.h"

#include <memory>
#include <cmath>

using namespace unboxing_engine;

namespace {

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

SMaterial markMaterial() {
    SMaterial material{};
    material.materialDif[0] = 1.78f;
    material.materialDif[1] = 0.22f;
    material.materialDif[2] = 0.20f;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}


class CSegment
    : public CSceneComposite
    , public UListener<core_events::IUpdateListener, core_events::IMouseInputEvent> {
public:
    CSegment(CCore &engine, const std::vector<float>& color)
    : mEnd(Vector3f(0, 1, 0))
    , mStart(0,0,0)
    , mMesh(primitive_utils::Cylinder(mStart, mEnd, 1, 1, 6))
    , mColor(color)
    , mEngine(engine) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        SMaterial material = render->GetMaterial();
        material.materialDif[0] = mColor[0];
        material.materialDif[1] = mColor[1];
        material.materialDif[2] = mColor[2];
        material.materialDif[3] = mColor[3];
        render->SetMaterial(material);
        AddComponent<IRenderComponent>(std::move(render));
    }

    ~CSegment() override = default;

    void UpdateSegment(const Vector3f& start, const Vector3f& end) {
        const Vector3f direction = (end - start).Normalized();
        float length = (end - start).Length();

        const Vector3f initialDirection = (mEnd - mStart).Normalized();

        const float cos = initialDirection.DotProduct(direction);
        float angle = acosf(cos);
        const Vector3f axis = initialDirection.CrossProduct(direction);

        SetRotation(Quaternion(angle*(180.0f/pi), axis.Normalized()));
        SetPosition(start);
        SetScale(Vector3f(length, length, length));
    }

    void OnUpdate() override {
    }

    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        if (!cursor.isButtonPressed) {
            SetColor(1, 0, 0, 1);
            return;
        }
        SetColor(mColor[0], mColor[1], mColor[2], mColor[3]);
        Vector3f outPoint, outDirection;
        mEngine.GetCamera().CastRayFromScreen(cursor.buttonPressedX, cursor.buttonPressedY, outPoint, outDirection);
        UpdateSegment((outPoint + outDirection*900) , outPoint );
    }

    void SetStart(const Vector3f& start) {
        SetPosition(start);
    }

    void SetEnd(const Vector3f& end) {
        // If line surfer transformations, end might not reflect global coordinates
        mEnd = end;
    }

    void SetColor(float r, float g, float b, float a = 1) {
        auto render = GetComponent<IRenderComponent>();
        SMaterial material = render->GetMaterial();
        material.materialDif[0] = r;
        material.materialDif[1] = g;
        material.materialDif[2] = b;
        material.materialDif[3] = a;
        render->SetMaterial(material);
    }

private:
    Vector3f mEnd;
    Vector3f mStart;
    std::unique_ptr<CMeshBuffer> mMesh;
    std::vector<float> mColor;
    CCore &mEngine;
};


class CMarkSceneObject : public CSceneComposite {
public:
    explicit CMarkSceneObject(std::unique_ptr<CMeshBuffer> mesh) : mMesh(std::move(mesh)) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        render->SetMaterial(markMaterial());
        render->SetPolygonMode(EPolygonMode::Fill);
        AddComponent<IRenderComponent>(std::move(render));
    }
private:
    std::unique_ptr<CMeshBuffer> mMesh;
};

class COctreeSceneObject : public CSceneComposite {
public:
    COctreeSceneObject(const CMeshBuffer &mesh, const COctree &octree, const Camera &camera, CMarkSceneObject &mark)
        : mOctree(octree)
        , mCamera(camera)
        , mMark(mark) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(mesh);
        render->SetMaterial(octreeMaterial());
        render->SetPolygonMode(EPolygonMode::Line);
        AddComponent<IRenderComponent>(std::move(render));
    }

private:
    const COctree &mOctree;
    const Camera &mCamera;
    CMarkSceneObject &mMark;
};

}// namespace

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
        mMark = std::make_unique<CMarkSceneObject>(primitive_utils::Cylinder(Vector3f(), Vector3f(0, 1 , 0), 0.01, 0.01, 6));
        mOctreeObject = std::make_unique<COctreeSceneObject>(*mOctreeMesh, *mOctree, mEngine.GetCamera(), *mMark);
        mEngine.RegisterSceneElement(*mOctreeObject);
        mEngine.RegisterSceneElement(*mMark);
    }


    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        if (!cursor.isButtonPressed) {
            return;
        }
        Vector3f outPoint, outDirection;
        mEngine.GetCamera().CastRayFromScreen(cursor.buttonPressedX, cursor.buttonPressedY, outPoint, outDirection);
        CMeshBuffer mesh;
        auto hit = RayWithOctree(outPoint,  outDirection, *mOctree, &mesh);

        if (hit.hit) {
            mEngine.UnregisterSceneElement(*mOctreeObject);
            mEngine.UnregisterSceneElement(*mMark);

            mOctreeDebugMesh = std::make_unique<CMeshBuffer>(mesh);
            mMark = std::make_unique<CMarkSceneObject>(primitive_utils::Cylinder(Vector3f(), Vector3f(0, 1 , 0), 0.01, 0.01, 6));
            mMark->SetPosition(Vector3f(hit.intersection));
            mOctreeObject = std::make_unique<COctreeSceneObject>(*mOctreeDebugMesh, *mOctree, mEngine.GetCamera(), *mMark);

            mEngine.RegisterSceneElement(*mOctreeObject);
            mEngine.RegisterSceneElement(*mMark);
        }
    }

    ~CTorusSceneObject() override {
        mEngine.UnregisterSceneElement(*mOctreeObject);
        mEngine.UnregisterSceneElement(*mMark);
    }
private:
    CCore& mEngine;
    std::unique_ptr<CMeshBuffer> mMesh;
    std::unique_ptr<COctree> mOctree;

    std::unique_ptr<CMeshBuffer> mOctreeMesh;
    std::unique_ptr<CMarkSceneObject> mMark;
    std::unique_ptr<COctreeSceneObject> mOctreeObject;

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
    CSegment segment(engine, {0, 1, 0, 1});

    if (engine.IsRunning()) {
        engine.RegisterSceneElement(segment);
        engine.RegisterSceneElement(torusObject);
        engine.Run();
        engine.UnregisterSceneElement(torusObject);
        engine.UnregisterSceneElement(segment);
    }

    engine.Release();
    return 0;
}
