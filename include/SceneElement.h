#pragma once

#include <memory>
#include <string>

namespace unboxing_engine {

class TransformComponent;
class Composite;

class SceneElement {
public:
    int id{};

public:
    SceneElement();
    ~SceneElement() = default;

    int getID();
    void setID(int id);
    int getType();
    int getTexID();

    virtual void Update();

private:
    std::unique_ptr<TransformComponent> m_transform;
    std::unique_ptr<Composite> m_components;
};
}// namespace unboxing_engine