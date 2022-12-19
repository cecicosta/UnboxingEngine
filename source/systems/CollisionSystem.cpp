#include "systems/CollisionSystem.h"

namespace unboxing_engine::systems {

CollisionSystem::CollisionSystem() {
}
CollisionSystem::~CollisionSystem() = default;

void CollisionSystem::OnPreRender() {
    auto listeners = GetListeners<IIntersectsEventListener>();
    for (auto&& listener : listeners) {
        listener->OnIntersects();
    }
}

}// namespace unboxing_engine::systems
