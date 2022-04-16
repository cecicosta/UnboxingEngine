#include "UnboxingEngine.h"

#include <MeshPrimitivesUtils.h>


int main(int argc, char *argv[]) {

    CUnboxingEngine engine(640, 480, 32);
    engine.CreateWindow();

    auto cube = primitive_utils::Cube();
    engine.RegisterSceneElement(*cube);

    engine.Run();
    engine.Release();
    return 0;
}
