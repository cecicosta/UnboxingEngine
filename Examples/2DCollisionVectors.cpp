#include "UnboxingEngine.h"

#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "internal_components/RenderComponent.h"

using namespace unboxing_engine;

int main(int argc, char *argv[]) {
    CCore engine(640, 480, 32);
    engine.Start();
    CSceneComposite box;
    CSceneComposite line;

    auto line_mesh = primitive_utils::DrawLines(Vector3f(1,1,0), Vector3f(-1,-1,0));
    std::unique_ptr<IRenderComponent> lineRenderComponent = std::make_unique<CDefaultMeshRenderComponent>(*line_mesh);
    line.AddComponent(*lineRenderComponent);
    engine.RegisterSceneElement(line);

    auto box_mesh = primitive_utils::Cube();
    box_mesh->material.materialDif[0] = 1;
    std::unique_ptr<IRenderComponent> boxRenderComponent = std::make_unique<CDefaultMeshRenderComponent>(*box_mesh);
    box.AddComponent(*boxRenderComponent);
    engine.RegisterSceneElement(box);


    engine.Run();
    engine.Release();
    return 0;
}
