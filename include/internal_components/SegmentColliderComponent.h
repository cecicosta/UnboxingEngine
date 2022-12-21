#pragma once

#include "internal_components/IColliderComponent.h"

namespace unboxing_engine {

class CSegmentColliderComponent : public internal_components::IColliderComponent {
public:
    CSegmentColliderComponent();
    ~CSegmentColliderComponent() override;

    [[nodiscard]] bool HasCollided(const IColliderComponent &other) const override;
};
}// namespace unboxing_engine