#pragma once

#include "internal_components/IColliderComponent.h"

namespace unboxing_engine {
class CBoxColliderComponent2D;
class CMeshBuffer;

class CSegmentColliderComponent : public IColliderComponent {
public:
    CSegmentColliderComponent();
    ~CSegmentColliderComponent() override;

    [[nodiscard]] bool HasCollided(const IColliderComponent &other) const override;

    void OnIntersects() override;
    const CSceneComposite *GetSceneComposite() const override;
    void OnAttached(CSceneComposite &) override;
    void OnDetached() override;

private:
    [[nodiscard]] bool HasCollided(const CBoxColliderComponent2D &other) const;
    [[nodiscard]] bool HasCollided(const CSegmentColliderComponent &other) const;

    CSceneComposite *mSceneComposite = nullptr;
    const CMeshBuffer *mMeshBuffer = nullptr;
};
}// namespace unboxing_engine