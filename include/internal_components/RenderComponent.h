#pragma once

#include "IComponent.h"
#include "internal_components/IRenderComponent.h"
#include "systems/IRenderSystem.h"

#include <memory>

namespace unboxing_engine {
class CMeshBuffer;

class CRenderComponentBase : public IRenderComponent {
public:
    CRenderComponentBase(const CMeshBuffer &meshBuffer)
        : mMeshBuffer(&meshBuffer) {}
    ~CRenderComponentBase() override = default;

    void SetMaterial(const SMaterial &material) { mMaterial = material; }
    const SMaterial &GetMaterial() const { return mMaterial; }
    const CMeshBuffer &GetMeshBuffer() const override { return *mMeshBuffer; }
    void SetMeshBuffer(const CMeshBuffer &meshBuffer) override {
        mMeshBuffer = &meshBuffer;
        mIsDirty = true;
    }

    const CSceneComposite *GetSceneComposite() const override { return mSceneComposite; }
    void OnAttached(CSceneComposite &sceneComposite) override { mSceneComposite = &sceneComposite; }
    void OnDetached() override { mSceneComposite = nullptr; };
    
protected:
    bool mIsDirty = true;
    const CMeshBuffer *mMeshBuffer;
    SMaterial mMaterial;
    CSceneComposite *mSceneComposite = nullptr;
    std::unique_ptr<systems::SRenderContextHandle> mRenderContextHandle;
};


class CDefaultMeshRenderComponent : public CRenderComponentBase {
public:
    CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer);
    ~CDefaultMeshRenderComponent() override;

    void OnInitialize(systems::IRenderSystem &renderSystem) override;
    void ReleaseRenderContext() override;
    void OnRender() override;
private:
    void UpdateRenderContext();
    systems::IRenderSystem *mRenderSystem;
};
}// namespace unboxing_engine
