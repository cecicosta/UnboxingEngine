#include "internal_components/RayColliderComponent.h"

namespace unboxing_engine {

CRayColliderComponent::CRayColliderComponent() {}
CRayColliderComponent::~CRayColliderComponent() = default;

bool CRayColliderComponent::HasCollided(const IColliderComponent&) const {
    return false;
}

}// namespace unboxing_engine