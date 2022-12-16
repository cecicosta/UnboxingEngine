#include "SceneComposite.h"
#include "internal_components/IComponent.h"

namespace unboxing_engine {

void CSceneComposite::AddComponent(std::size_t hash, IComponent &component) {
    m_components.try_emplace(hash, &component);
}

IComponent *CSceneComposite::GetComponent(std::size_t hash) const {
    auto it = m_components.find(hash);
    return it != m_components.end() ? it->second : nullptr;
}

void CSceneComposite::RemoveComponent(std::size_t hash) {
    auto it = m_components.find(hash);
    if(it != m_components.end()) {
        m_components.erase(it);
    }
}

}// namespace unboxing_engine

