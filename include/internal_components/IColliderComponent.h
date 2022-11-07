#pragma once

#include "IComponent.h"

class IColliderComponent : public IComponent {
public:
    ~IColliderComponent() override = default;

    [[nodiscard]] virtual bool HasCollided() const = 0;
};
