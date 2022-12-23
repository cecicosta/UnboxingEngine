#include "systems/CollisionSystem.h"
#include "internal_components/IColliderComponent.h"

namespace unboxing_engine::systems {

CollisionSystem::CollisionSystem() {
}
CollisionSystem::~CollisionSystem() = default;

void CollisionSystem::OnPreRender() {
    for (auto &&collider1: mColliders) {
        for (auto &&collider2: mColliders) {
            if (collider1->HasCollided(*collider2)) {
                collider1->OnIntersects();
            }
        }
    }
}

void CollisionSystem::RegisterCollider(IColliderComponent &colliderComponent) {
    mColliders.push_back(&colliderComponent);
    RegisterListener(colliderComponent);
}

void CollisionSystem::UnregisterCollider(IColliderComponent &colliderComponent) {
    auto it = std::find(mColliders.begin(), mColliders.end(), &colliderComponent);
    if (it != mColliders.end()) {
        UnregisterListener(colliderComponent);
        mColliders.erase(it); //TODO: Verify - Will it destroy the collider?
    }
}

}// namespace unboxing_engine::systems
