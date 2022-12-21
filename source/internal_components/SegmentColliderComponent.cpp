#include "internal_components/SegmentColliderComponent.h"

#include "algorithms/CollisionAlgorithms.h"

namespace unboxing_engine {

unboxing_engine::CSegmentColliderComponent::CSegmentColliderComponent() {
}
CSegmentColliderComponent::~CSegmentColliderComponent() = default;

bool CSegmentColliderComponent::HasCollided(const IColliderComponent &other) const {
    return false;
}

}// namespace unboxing_engine