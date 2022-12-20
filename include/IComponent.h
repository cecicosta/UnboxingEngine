#pragma once

namespace unboxing_engine {
class CSceneComposite;

class IComponent {
public:
    virtual ~IComponent() = default;

    virtual void OnAttached(CSceneComposite &) = 0;
    virtual void OnDetached() = 0;
};
}// namespace unboxing_engine