#pragma once

#include <systems/IInputSystem.h>
#include <CoreEvents.h> 

namespace unboxing_engine::systems {
class CSDLInputSystem : public IInputSystem {
public:
    ~CSDLInputSystem() override;

    void OnInput() override;

private:
    core_events::SCursor mCursor;
    core_events::SKeyboard mKeyboard;

    ///Keyboard input attributes
    std::uint8_t const *keyState = nullptr;
    bool quit = false;
};
}// namespace unboxing_engine::systems