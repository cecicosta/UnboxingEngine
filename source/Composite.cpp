#include "Composite.h"

#include <algorithm>

namespace unboxing_engine {

    void Composite::AddComponent(IComponent &component) {
        m_components.push_back(&component);
    }

    IComponent *Composite::GetComponent(const size_t &hash) {
        auto it = std::find_if(m_components.begin(), m_components.end(), [&hash](IComponent* component) {
            auto compHash = typeid(*component).hash_code();
            return compHash == hash;
        });
        return it != m_components.end() ? *it : nullptr;
    }

    void Composite::RemoveComponent(const size_t &hash) {
        auto it = std::find_if(m_components.begin(), m_components.end(), [&hash](IComponent* component) {
            return typeid(component).hash_code() == hash;
        });
        if(it != m_components.end()) {
            m_components.erase(it);
        }
    }
}// namespace unboxing_engine