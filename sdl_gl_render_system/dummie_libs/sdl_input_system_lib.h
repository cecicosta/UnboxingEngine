#pragma once

#include <systems/IInputSystem.h>

namespace unboxing_engine::systems {
class CSDLInputSystem : public IInputSystem {
public:
    ~CSDLInputSystem() override;
    void OnInput() override;
};
}// namespace unboxing_engine::systems