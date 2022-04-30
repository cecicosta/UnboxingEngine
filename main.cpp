#include "UnboxingEngine.h"

#include <MeshPrimitivesUtils.h>

#include "CoreEvents.h"
#include "Composite.h"

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

class MockComponent : public unboxing_engine::IComponent {
public:
    ~MockComponent() override = default;
    bool IsComponent() { return true; }
};

int main(int argc, char *argv[]) {
    unboxing_engine::Composite composite;
    MockComponent mockComponent;

    std::cout << "Is component: " << mockComponent.IsComponent() << std::endl;
    composite.AddComponent(mockComponent);
//    auto component = composite.GetComponent<MockComponent>();


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
