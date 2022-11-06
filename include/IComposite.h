#pragma once

#include <cstddef>
#include <cstdint>

namespace unboxing_engine {

    class IComponent {
    public:
        virtual ~IComponent() = default;
    };

    class IComposite {
    public:
        virtual ~IComposite() = default;
        virtual void AddComponent(IComponent &component) = 0;
        virtual IComponent *GetComponent(const size_t &hash) = 0;
        virtual void RemoveComponent(const size_t &hash) = 0;
    };

}// namespace unboxing_engine