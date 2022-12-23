#pragma once

#include "internal_components/IColliderComponent.h"

namespace unboxing_engine {

class CRayColliderComponent : public IColliderComponent {
public:
    CRayColliderComponent();
    ~CRayColliderComponent() override;

    const CSceneComposite *GetSceneComposite() const override { return nullptr; }
    [[nodiscard]] bool HasCollided(const IColliderComponent &) const override;
    void OnAttached(CSceneComposite &) override {}
    void OnDetached() override {}
};
}// namespace unboxing_engine