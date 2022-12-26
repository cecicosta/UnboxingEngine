#include "UnboxingEngine.h"

#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "CoreEvents.h"
#include "internal_components/RenderComponent.h"
#include "internal_components/SegmentColliderComponent.h"
#include "internal_components/BoundingBox2DColliderComponent.h"

using namespace unboxing_engine;

std::unique_ptr<CSceneComposite> CreateLineObject(CCore& engine, const CMeshBuffer& mesh) {
    std::unique_ptr<CSceneComposite> line = std::make_unique<CSceneComposite>();
    std::unique_ptr<IRenderComponent> lineRenderComponent = std::make_unique<CDefaultMeshRenderComponent>(mesh);
    std::unique_ptr<IColliderComponent> segmentCollider = std::make_unique<CSegmentColliderComponent>();
    line->AddComponent(std::move(lineRenderComponent));
    line->AddComponent(std::move(segmentCollider));
    engine.RegisterSceneElement(*line);
    return std::move(line);
}

class CSegment : public CSceneComposite, public unboxing_engine::UListener<systems::IIntersectsEventListener, core_events::IUpdateListener> {
public:
    CSegment() {
        mMesh = primitive_utils::Lines(Vector3f(0.5f, 0, 0), Vector3f(0.4f, 0, 0));
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

private:
    std::unique_ptr<CMeshBuffer> mMesh;
};

std::unique_ptr<CSceneComposite> CreateBoxObject(CCore& engine, const CMeshBuffer& mesh) {
    std::unique_ptr<CSceneComposite> box = std::make_unique<CSceneComposite>();
    std::unique_ptr<IRenderComponent> boxRenderComponent = std::make_unique<CDefaultMeshRenderComponent>(mesh);
    std::unique_ptr<IColliderComponent> boxCollider = std::make_unique<CBoxColliderComponent2D>();
    box->AddComponent(std::move(boxRenderComponent));
    box->AddComponent(std::move(boxCollider));
    engine.RegisterSceneElement(*box);
    return std::move(box);
}

int main(int argc, char *argv[]) {
    CCore engine(640, 480, 32);
    engine.Start();

    std::unique_ptr<CSceneComposite> segment = std::make_unique<CSegment>();
    engine.RegisterSceneElement(*segment.get());

    auto box_mesh = primitive_utils::Quad();
    auto box = CreateBoxObject(engine, *box_mesh);
    box->SetScale({0.5f, 0.5f, 0.5f});

    engine.Run();
    engine.Release();
    return 0;
}
