#include "UnboxingEngine.h"

#include <MeshPrimitivesUtils.h>

using namespace unboxing_engine;

int main(int argc, char *argv[]) {
    CCore engine(640, 480, 32);
    engine.Start();



    engine.Run();
    engine.Release();
    return 0;
}
