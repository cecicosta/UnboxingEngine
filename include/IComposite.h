#pragma once

#include "internal_components/IComponent.h"
#include <cstddef>
#include <cstdint>

namespace unboxing_engine {

class IComposite {
    public:
        virtual ~IComposite() = default;
        virtual void AddComponent(IComponent &component) = 0;
        virtual IComponent *GetComponent(const size_t &hash) = 0;
        virtual void RemoveComponent(const size_t &hash) = 0;
    };

}// namespace unboxing_engine