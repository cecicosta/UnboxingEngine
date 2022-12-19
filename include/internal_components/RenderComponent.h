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

class CDefaultMeshRenderComponent : public IRenderComponent {
public:
    CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
        : mMeshBuffer(meshBuffer) {}
    ~CDefaultMeshRenderComponent() override = default;

    const CMeshBuffer &GetMeshBuffer() const override { return mMeshBuffer; }
    void Render() const override;

    void OnAttached(IComposite &) override {}
    void OnDetached() override{};
private:
    const CMeshBuffer &mMeshBuffer;
};
}// namespace unboxing_engine