#include "internal_components/BoundingBox2DColliderComponent.h"

#include "SceneComposite.h"
#include "algorithms/CollisionAlgorithms.h"
#include "internal_components/IRenderComponent.h"

#include <assert.h>

namespace unboxing_engine {

void CBoxColliderComponent2D::OnIntersects() {
}

bool CBoxColliderComponent2D::HasCollided(const IColliderComponent& other) const {

    return false;
}

void CBoxColliderComponent2D::OnAttached(CSceneComposite& composite) {
    mComposite = &composite;
}

void CBoxColliderComponent2D::OnDetached() {
}

bool CBoxColliderComponent2D::HasCollided(const Vector2f &p1, const Vector2f p2) {
    assert(mComposite && "CSceneComposite has not been attached.");
    if (auto render = mComposite->GetComponent<IRenderComponent>()) {
        //algorithms::checkSegmentCollisionAgainstRect(render->GetMeshBuffer().vertices, p1, p2);
    }
    return false;
}

bool CBoxColliderComponent2D::HasCollided(const CBoxColliderComponent2D &other) {
    return false;
}

bool CBoxColliderComponent2D::HasCollided(const CMeshBuffer &mesh) {
    return false;
}

}// namespace unboxing_engine
