#include "UnboxingEngine.h"

#include <MeshPrimitivesUtils.h>

#include "CoreEvents.h"

using namespace unboxing_engine;


class CEventListener: public core_events::ICoreEventsListener {
public:
    void OnStart() override {
        std::cout << "OnStart called" << std::endl;
    }
    void OnUpdate() override {
        std::cout << "OnUpdate called" << std::endl;
    }
    void OnInput() override {
        std::cout << "OnInput called" << std::endl;
    }
    void OnPreRender() override {
        std::cout << "OnPreRender called" << std::endl;
    }
    void OnPostRender() override {
        std::cout << "OnPostRender called" << std::endl;
    }
    void OnRelease() override {
        std::cout << "OnRelease called" << std::endl;
    }
};

int main(int argc, char *argv[]) {

    CCore engine(640, 480, 32);


    CEventListener eventListener;
    engine.RegisterEventListener(eventListener);
    engine.Start();

    auto cube = primitive_utils::Cube();
    engine.RegisterSceneElement(*cube);

    engine.Run();
    engine.Release();
    return 0;
}
