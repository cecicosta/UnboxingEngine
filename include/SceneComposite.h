#pragma once

#include "IComposite.h"
#include "SceneNode.h"
#include "internal_components/IComponent.h"

#include <memory>
#include <string>
#include <map>

namespace unboxing_engine {

class CSceneComposite : public SceneNode, public IComposite {
public:
    int id{};

    CSceneComposite() = default;
    CSceneComposite(const CSceneComposite &) = delete;
    ~CSceneComposite() override = default;

    template<class T>
    void AddComponent(std::unique_ptr<T> component);
    template<class T>
    T *GetComponent() const;
    template<class T>
    void RemoveComponent();

private:
    void AddComponent(std::size_t hash, std::unique_ptr<IComponent> component) override;
    IComponent *GetComponent(std::size_t hash) const override;
    void RemoveComponent(std::size_t hash) override;

    //TODO: Try to improve memory allocation management. Potentially suboptimal with reallocation, copy and fragmentation of memory for new added components.
    std::map<std::size_t, std::unique_ptr<IComponent>> m_components;
};

template<class T>
void CSceneComposite::AddComponent(std::unique_ptr<T> component) {
    static_assert(std::is_base_of_v<IComponent, T>);
    auto key = typeid(T).hash_code();
    AddComponent(key, std::move(component));
}

template<class T>
T *CSceneComposite::GetComponent() const {
    return dynamic_cast<T *>(GetComponent(typeid(T).hash_code()));
}

template<class T>
void CSceneComposite::RemoveComponent() {
    RemoveComponent(typeid(T).hash_code());
}

}// namespace unboxing_engine