#pragma once

#include "internal_components/IRenderComponent.h"
#include "IComponent.h"

namespace unboxing_engine {
class CMeshBuffer;

class CDefaultMeshRenderComponent : public IRenderComponent {
public:
    CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
        : mMeshBuffer(meshBuffer) {}
    ~CDefaultMeshRenderComponent() override = default;

    const CMeshBuffer &GetMeshBuffer() const override { return mMeshBuffer; }
    void Render() const override;

    void OnAttached(CSceneComposite &) override {}
    void OnDetached() override{};
private:
    const CMeshBuffer &mMeshBuffer;
};
}// namespace unboxing_engine