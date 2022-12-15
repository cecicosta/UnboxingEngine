#include "UnboxingEngine.h"

#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "internal_components/RenderComponent.h"

using namespace unboxing_engine;

int main(int argc, char *argv[]) {
    CCore engine(640, 480, 32);
    engine.Start();
    CSceneComposite composite;

    auto cube = primitive_utils::Cube();
    cube->material.materialDif[0] = 1;
    CDefaultMeshRenderComponent renderComponent(*cube);
    composite.AddComponent(renderComponent);

    engine.RegisterSceneElement(composite);


    engine.Run();
    engine.Release();
    return 0;
}
