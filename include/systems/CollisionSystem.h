#pragma once 

#include "EventDispatcher.h"
#include "CoreEvents.h"
#include "algorithms/CollisionAlgorithms.h"

namespace unboxing_engine {
class IColliderComponent;
}

namespace unboxing_engine::systems {

class IIntersectsEvent {
public:
    virtual ~IIntersectsEvent() = default;
    virtual void OnIntersects() = 0;
};

class ICollisionEvent {
public:
    virtual ~ICollisionEvent() = default;
    virtual void OnCollisionEvent(const IColliderComponent &c1, const IColliderComponent &c2, const algorithms::SCollisionResult<float, 3> &result) = 0;
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
