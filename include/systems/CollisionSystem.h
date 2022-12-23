#pragma once 

#include "EventDispatcher.h"
#include "CoreEvents.h"

namespace unboxing_engine {
class IColliderComponent;
}

namespace unboxing_engine::systems {

class IIntersectsEventListener {
public:
    virtual ~IIntersectsEventListener() = default;
    virtual void OnIntersects() = 0;
};

class CollisionSystem : public UListener <core_events::IPreRenderListener>, public CEventDispatcher {
public:
    CollisionSystem();
    ~CollisionSystem() override;

    void RegisterCollider(IColliderComponent &colliderComponent);
    void UnregisterCollider(IColliderComponent &colliderComponent);

    void OnPreRender() override;

private:
    std::vector<IColliderComponent *> mColliders;
};

}// namespace unboxing_engine::systems
