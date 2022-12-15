#include "UnboxingEngine.h"

#include <MeshPrimitivesUtils.h>

using namespace unboxing_engine;

int main(int argc, char *argv[]) {
    CCore engine(640, 480, 32);
    engine.Start();

    auto cube = primitive_utils::Cube();
    cube->material.materialDif[0] = 1;
    engine.RegisterSceneElement(*cube);


    engine.Run();
    engine.Release();
    return 0;
}
