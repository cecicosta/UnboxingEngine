#include "UnboxingEngine.h"

#include "CoreEvents.h"
#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "internal_components/BoundingBox2DColliderComponent.h"
#include "internal_components/RenderComponent.h"
#include "internal_components/SegmentColliderComponent.h"
#include "sdl_gl_render_system_lib.h"
using namespace unboxing_engine;


class CBox : public CSceneComposite
    , public unboxing_engine::UListener<systems::IIntersectsEvent, core_events::IUpdateListener> {
public:
    CBox() {
        mMesh = primitive_utils::Quad();
        AddComponent<IRenderComponent>(std::make_unique<CDefaultMeshRenderComponent>(*mMesh));
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

#include <iostream>
int main(int argc, char *argv[]) {

    CCore engine(640, 480, 32);
    engine.Start();
    
    std::unique_ptr<CBox> box = std::make_unique<CBox>();
    engine.RegisterSceneElement(*box.get());
    engine.RegisterEventListener(*box.get());
    box->SetScale({0.5f, 0.5f, 0.5f});

    engine.Run();
    engine.Release(); 
    return 0;
}
