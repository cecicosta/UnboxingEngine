#pragma once 

#include "EventDispatcher.h"
#include "CoreEvents.h"

namespace unboxing_engine::systems {

class IIntersectsEventListener {
public:
    virtual ~IIntersectsEventListener() = default;
    virtual void OnIntersects() = 0;
};

class CollisionSystem: public core_events::IPreRenderListener, public CEventDispatcher {
public:
    CollisionSystem();
    ~CollisionSystem() override;

    void OnPreRender() override;
};

}// namespace unboxing_engine::systems
