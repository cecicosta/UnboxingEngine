#pragma once

#include "IComponent.h"
#include "internal_components/IRenderComponent.h"

namespace unboxing_engine {
class CMeshBuffer;

class CDefaultMeshRenderComponent : public IRenderComponent {
public:
    CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
        : mMeshBuffer(meshBuffer) {}
    ~CDefaultMeshRenderComponent() override = default;

    void SetMaterial(const SMaterial &material) { mMaterial = material; }
    const SMaterial &GetMaterial() const { return mMaterial; }
    const CMeshBuffer &GetMeshBuffer() const override { return mMeshBuffer; }
    void Render() const override;

    const CSceneComposite *GetSceneComposite() const override { return mSceneComposite; }
    void OnAttached(CSceneComposite &sceneComposite) override { mSceneComposite = &sceneComposite; }
    void OnDetached() override { mSceneComposite = nullptr; };

private:
    const CMeshBuffer &mMeshBuffer;
    SMaterial mMaterial;
    CSceneComposite *mSceneComposite = nullptr;
};
}// namespace unboxing_engine