#pragma once

#include "IComponent.h"
#include "internal_components/IRenderComponent.h"

namespace unboxing_engine {
class CMeshBuffer;

class CRenderComponentBase : public IRenderComponent {
public:
    CRenderComponentBase(const CMeshBuffer &meshBuffer)
        : mMeshBuffer(meshBuffer) {}
    ~CRenderComponentBase() override = default;

    void SetMaterial(const SMaterial &material) { mMaterial = material; }
    const SMaterial &GetMaterial() const { return mMaterial; }
    const CMeshBuffer &GetMeshBuffer() const override { return mMeshBuffer; }

    const CSceneComposite *GetSceneComposite() const override { return mSceneComposite; }
    void OnAttached(CSceneComposite &sceneComposite) override { mSceneComposite = &sceneComposite; }
    void OnDetached() override { mSceneComposite = nullptr; };

protected:
    const CMeshBuffer &mMeshBuffer;
    SMaterial mMaterial;
    CSceneComposite *mSceneComposite = nullptr;
};


class CDefaultMeshRenderComponent : public CRenderComponentBase {
public:
    CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
        : CRenderComponentBase(meshBuffer) {}
    ~CDefaultMeshRenderComponent() override = default;

    void Render() const override;
};
}// namespace unboxing_engine