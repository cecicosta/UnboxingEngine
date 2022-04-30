#include "Composite.h"

#include <algorithm>
#include <typeinfo>

namespace unboxing_engine {

    void Composite::AddComponent(IComponent &component) {
        m_components.push_back(&component);
    }

    IComponent *Composite::GetComponent(const size_t &hash) {
        auto it = std::find_if(m_components.begin(), m_components.end(), [&hash](IComponent* component) {
            return typeid(component).hash_code() == hash;
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

    template<class T>
    void Composite::AddComponent(T &component) {
        IComposite::AddComponent(component);
    }

    template<class T>
    void Composite::RemoveComponent(T &component) {
        IComposite::RemoveComponent(typeid(T).hash_code());
    }

    template<class T>
    T *Composite::GetComponent() {
        return dynamic_cast<T*>(IComposite::GetComponent(typeid(T).hash_code()));
    }
}// namespace unboxing_engine