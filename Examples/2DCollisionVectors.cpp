#include "UnboxingEngine.h"

#include "CoreEvents.h"
#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "internal_components/BoundingBox2DColliderComponent.h"
#include "internal_components/RenderComponent.h"
#include "internal_components/SegmentColliderComponent.h"

using namespace unboxing_engine;

class CSegment
    : public CSceneComposite
    , public unboxing_engine::UListener<systems::IIntersectsEvent, core_events::IUpdateListener, core_events::IMouseInputEvent> {
public:
    CSegment(CCore &engine)
        : mEngine(engine) {
        mMesh = primitive_utils::Lines(Vector3f(0.5f, 0, 0), Vector3f(-0.4f, 0, 0));
        AddComponent<IRenderComponent>(std::make_unique<CDefaultMeshRenderComponent>(*mMesh));
        AddComponent<IColliderComponent>(std::make_unique<CSegmentColliderComponent>());
    }

    ~CSegment() override = default;
    void OnIntersects() override {
        auto render = GetComponent<IRenderComponent>();
        SMaterial material = render->GetMaterial();
        material.materialDif[0] = 1;
        material.materialDif[1] = 0;
        material.materialDif[2] = 0;
        material.materialDif[3] = 1;
        render->SetMaterial(material);
    }

    void OnUpdate() override {
        auto render = GetComponent<IRenderComponent>();
        SMaterial material = render->GetMaterial();
        material.materialDif[0] = 0;
        material.materialDif[1] = 0;
        material.materialDif[2] = 0;
        material.materialDif[3] = 1;
        render->SetMaterial(material);
    }

    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        mEngine.UnregisterSceneElement(*this);
        auto point = Vector3f(static_cast<float>(2 * cursor.x) / 640.0f - 1, -static_cast<float>(2 * cursor.y) / 480.0f + 1, 0);
        mMesh->vertices[3] = point.x;
        mMesh->vertices[4] = point.y;
        mMesh->vertices[5] = point.z;
        mMesh->vertices[6] = point.x;
        mMesh->vertices[7] = point.y;
        mMesh->vertices[8] = point.z;
        mEngine.RegisterSceneElement(*this);
    }

private:
    std::unique_ptr<CMeshBuffer> mMesh;
    CCore &mEngine;
};


class CBox : public CSceneComposite
    , public unboxing_engine::UListener<systems::IIntersectsEvent, core_events::IUpdateListener> {
public:
    CBox() {
        mMesh = primitive_utils::Quad();
        AddComponent<IRenderComponent>(std::make_unique<CDefaultMeshRenderComponent>(*mMesh));
        AddComponent<IColliderComponent>(std::make_unique<CBoxColliderComponent2D>());
    }

    ~CBox() override = default;
    void OnIntersects() override {
        auto render = GetComponent<IRenderComponent>();
        SMaterial material = render->GetMaterial();
        material.materialDif[0] = 1;
        material.materialDif[1] = 0;
        material.materialDif[2] = 0;
        material.materialDif[3] = 1;
        render->SetMaterial(material);
    }

    void OnUpdate() override {
        auto render = GetComponent<IRenderComponent>();
        SMaterial material = render->GetMaterial();
        material.materialDif[0] = 0;
        material.materialDif[1] = 0;
        material.materialDif[2] = 0;
        material.materialDif[3] = 1;
        render->SetMaterial(material);
    }

private:
    std::unique_ptr<CMeshBuffer> mMesh;
};


int main(int argc, char *argv[]) {
    CCore engine(640, 480, 32);
    engine.Start();

    std::unique_ptr<CSegment> segment = std::make_unique<CSegment>(engine);
    engine.RegisterSceneElement(*segment.get());
    engine.RegisterEventListener(*segment.get());

    std::unique_ptr<CBox> box = std::make_unique<CBox>();
    engine.RegisterSceneElement(*box.get());
    engine.RegisterEventListener(*box.get());
    box->SetScale({0.5f, 0.5f, 0.5f});

    engine.Run();
    engine.Release();
    return 0;
}
