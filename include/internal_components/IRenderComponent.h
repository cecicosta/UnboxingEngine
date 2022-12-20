#pragma once

#include "IComponent.h"

namespace unboxing_engine {
class CMeshBuffer;

class IRenderComponent : public IComponent {
public:
    virtual ~IRenderComponent() = default;

    virtual const CMeshBuffer &GetMeshBuffer() const = 0;
    virtual void Render() const = 0;
};
}// namespace unboxing_engine