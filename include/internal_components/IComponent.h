#pragma once

namespace unboxing_engine {
class IComposite;

class IComponent {
public:
    virtual ~IComponent() = default;

    virtual void OnAttached(IComposite &) = 0;
    virtual void OnDetached() = 0;
};
}// namespace unboxing_engine