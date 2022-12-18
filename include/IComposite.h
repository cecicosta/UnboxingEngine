#pragma once

#include "internal_components/IComponent.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace unboxing_engine {

class IComposite {
    public:
        virtual ~IComposite() = default;
        virtual void AddComponent(std::size_t hash, std::unique_ptr<IComponent> component) = 0;
        virtual IComponent *GetComponent(size_t hash) const = 0;
        virtual void RemoveComponent(size_t hash) = 0;
    };

}// namespace unboxing_engine