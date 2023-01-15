#pragma once

#include "EventDispatcher.h"
#include "CoreEvents.h"

namespace unboxing_engine::systems {

class IInputSystem : public CEventDispatcher {
public:
    virtual ~IInputSystem() = default;

    virtual void OnInput() = 0;
};

}// namespace unboxing_engine