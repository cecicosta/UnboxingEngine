#include "systems/CollisionSystem.h"
#include "internal_components/IColliderComponent.h"
#include "algorithms/CollisionAlgorithms.h"

namespace unboxing_engine::systems {

CollisionSystem::CollisionSystem() {
}
CollisionSystem::~CollisionSystem() = default;

void CollisionSystem::OnPreRender() {
    auto it = mColliders.begin(); // Usen an extra iterator to avoid checking the collision twice with the same elements or against themselves
    for (auto &&collider1: mColliders) {
        it++;
        for (auto next = it; next != mColliders.end(); next++) {
            if (collider1->HasCollided(**next)) {
                for (auto&& listener : GetListeners<ICollisionEvent>()) {
                    algorithms::SCollisionResult<float, 3> result;
                    listener->OnCollisionEvent(*collider1, **next, result);
                }
            }
        }
    }
}

void CollisionSystem::RegisterCollider(IColliderComponent &colliderComponent) {
    mColliders.push_back(&colliderComponent);
}

void CollisionSystem::UnregisterCollider(IColliderComponent &colliderComponent) {
    auto it = std::find(mColliders.begin(), mColliders.end(), &colliderComponent);
    if (it != mColliders.end()) {
        mColliders.erase(it); //TODO: Verify - Will it destroy the collider?
    }
}

}// namespace unboxing_engine::systems
