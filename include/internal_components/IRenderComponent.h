#pragma once

#include "IComponent.h"
#include "material.h"

namespace unboxing_engine {
class CMeshBuffer;

namespace systems {
class IRenderSystem;
}

class IRenderComponent : public IComponent {
public:
    virtual ~IRenderComponent() = default;

    virtual const SMaterial &GetMaterial() const = 0;
    virtual void SetMaterial(const SMaterial &material) = 0;
    virtual const CMeshBuffer &GetMeshBuffer() const = 0;
    virtual void SetMeshBuffer(const CMeshBuffer &meshBuffer) = 0;
    virtual void ReleaseRenderContext() = 0;

    virtual void Render(systems::IRenderSystem &renderSystem) = 0;
};
}// namespace unboxing_engine
