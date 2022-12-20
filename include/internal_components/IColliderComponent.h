#pragma once

#include "IComponent.h"
#include "systems/CollisionSystem.h"
#include "EventDispatcher.h" 

namespace unboxing_engine::internal_components {

class IColliderComponent : public IComponent
    , public UListener<systems::IIntersectsEventListener> {
public:
    ~IColliderComponent() override = default;

    [[nodiscard]] virtual bool HasCollided(const IColliderComponent &other) const = 0;
};

}// namespace unboxing_engine::internal_components