#pragma once

#include "BoundingBox.h"
#include "MeshBuffer.h"
#include "internal_components/IColliderComponent.h"

namespace unboxing_engine {
class CSceneComposite;
}

namespace unboxing_engine {

class CBoxColliderComponent2D : public IColliderComponent {
public:
    CBoxColliderComponent2D() = default;
    ~CBoxColliderComponent2D() override = default;

    //IIntersectsListener
    void OnIntersects() override;
    //IColliderComponent
    [[nodiscard]] bool HasCollided(const IColliderComponent &other) const override;

    //IComponent
    const CSceneComposite *GetSceneComposite() const override { return mComposite; }
    void OnAttached(CSceneComposite& composite) override;
    void OnDetached() override;

private:
    [[nodiscard]] bool HasCollided(const CBoxColliderComponent2D &other);
    [[nodiscard]] bool HasCollided(const CMeshBuffer &mesh);
    [[nodiscard]] bool HasCollided(const Vector2f& p1, const Vector2f p2);

    CBoundingBox3D mBoundingBox;
    CSceneComposite *mComposite = nullptr;
};
}// namespace unboxing_engine
