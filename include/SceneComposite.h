#pragma once

#include "IComposite.h"
#include "SceneNode.h"
#include "internal_components/IComponent.h"

#include <memory>
#include <string>
#include <vector>

namespace unboxing_engine {

class CSceneComposite : public SceneNode, public IComposite {
public:
    int id{};

    CSceneComposite() = default;
    ~CSceneComposite() override = default;

    template<class T>
    void AddComponent(T &component);
    template<class T>
    T *GetComponent() const;
    template<class T>
    void RemoveComponent();

private:
    void AddComponent(IComponent &component) override;
    IComponent *GetComponent(const size_t &hash) const override;
    void RemoveComponent(const size_t &hash) override;

    //TODO: Try to improve memory allocation management. Potentially suboptimal with reallocation, copy and fragmentation of memory for new added components.
    std::vector<IComponent *> m_components;
};

template<class T>
void CSceneComposite::AddComponent(T &component) {
    m_components.push_back(&component);
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