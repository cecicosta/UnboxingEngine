#include "SceneComposite.h"
#include "internal_components/IComponent.h"

namespace unboxing_engine {

CSceneComposite::CSceneComposite() {
    //TODO: Should use another mean to achieve this?
    static int ids = 0;
    id = ids++;
}

void CSceneComposite::AddComponent(std::size_t hash, std::unique_ptr<IComponent> component) {
    auto &ref = *component;
    m_components.try_emplace(hash, std::move(component));
    ref.OnAttached(*this);
}

IComponent *CSceneComposite::GetComponent(std::size_t hash) const {
    auto it = m_components.find(hash);
    return it != m_components.end() ? it->second.get() : nullptr;
}

void CSceneComposite::RemoveComponent(std::size_t hash) {
    auto it = m_components.find(hash);
    if (it != m_components.end()) {
        it->second->OnDetached();
        m_components.erase(it);
    }
}

}// namespace unboxing_engine
