#include "UnboxingEngine.h"

#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "internal_components/RenderComponent.h"

using namespace unboxing_engine;

std::unique_ptr<CSceneComposite> CreateObject(CCore& engine, const CMeshBuffer& mesh) {
    std::unique_ptr<CSceneComposite> line = std::make_unique<CSceneComposite>();
    std::unique_ptr<IRenderComponent> lineRenderComponent = std::make_unique<CDefaultMeshRenderComponent>(mesh);
    line->AddComponent(std::move(lineRenderComponent));
    engine.RegisterSceneElement(*line);
    return std::move(line);
}

int main(int argc, char *argv[]) {
    CCore engine(640, 480, 32);
    engine.Start();
    auto line_mesh = primitive_utils::Lines(Vector3f(0.5f, 0, 0), Vector3f(-0.5f, 0, 0));
    auto line = CreateObject(engine, *line_mesh);



    auto box_mesh = primitive_utils::Cube();
    box_mesh->material.materialDif[0] = 1;
    auto box = CreateObject(engine, *box_mesh);
    box->SetScale({0.5f, 0.5f, 0.5f});

    engine.Run();
    engine.Release();
    return 0;
}
