#include "SceneComposite.h"

namespace unboxing_engine {

void SceneComposite::AddComponent(IComponent &component) {
    m_components.push_back(&component);
}

IComponent *SceneComposite::GetComponent(const size_t &hash) {
    auto it = std::find_if(m_components.begin(), m_components.end(), [&hash](IComponent* component) {
        return typeid(*component).hash_code() == hash;
    });
    return it != m_components.end() ? *it : nullptr;
}

void SceneComposite::RemoveComponent(const size_t &hash) {
    auto it = std::find_if(m_components.begin(), m_components.end(), [&hash](IComponent* component) {
        return typeid(*component).hash_code() == hash;
    });
    if(it != m_components.end()) {
        m_components.erase(it);
    }
}

}// namespace unboxing_engine

