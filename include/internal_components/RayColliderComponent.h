#pragma once

#include "internal_components/IColliderComponent.h"

namespace unboxing_engine {

class CRayColliderComponent : public internal_components::IColliderComponent {
public:
    CRayColliderComponent();
    ~CRayColliderComponent() override;

    [[nodiscard]] bool HasCollided(const IColliderComponent &other) const override;
};
}// namespace unboxing_engine