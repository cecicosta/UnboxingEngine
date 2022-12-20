#include "internal_components/BoundingBoxColliderComponent.h"

#include "algorithms/CollisionAlgorithms.h"
#include "internal_components/IRenderComponent.h"

namespace unboxing_engine::internal_components {

bool CBoxColliderComponent2D::HasCollided(const IColliderComponent& other) const {

    return false;
}

void CBoxColliderComponent2D::OnAttached(CSceneComposite& composite) {
    mComposite = &composite;
}

bool CBoxColliderComponent2D::HasCollided(const Vector2f &p1, const Vector2f p2) {
    /* if (!mComposite || mComposite->GetComponent<IRenderComponent>()) {
        return nullptr;
    }

    algorithms::checkSegmentCollisionAgainstRect(mComposite->)*/
    return false;
}

}// namespace unboxing_engine
