#pragma once

#include "IComponent.h"
#include "material.h"

namespace unboxing_engine {
class CMeshBuffer;

class IRenderComponent : public IComponent {
public:
    virtual ~IRenderComponent() = default;

    virtual void SetMaterial(const SMaterial &material) = 0;
    virtual const SMaterial &GetMaterial() const = 0;
    virtual const CMeshBuffer &GetMeshBuffer() const = 0;
    virtual void Render() const = 0;
};
}// namespace unboxing_engine