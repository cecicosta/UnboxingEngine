#pragma once

#include "IComponent.h"
#include "internal_components/IRenderComponent.h"
#include "systems/IRenderSystem.h"

#include <memory>
#include <string>

namespace unboxing_engine {
class CMeshBuffer;

class CRenderComponentBase : public IRenderComponent {
public:
    CRenderComponentBase(const CMeshBuffer &meshBuffer)
        : mMeshBuffer(&meshBuffer) {}
    ~CRenderComponentBase() override = default;

    void SetMaterial(const SMaterial &material) override { mMaterial = material; }
    const SMaterial &GetMaterial() const override { return mMaterial; }
    EPolygonMode GetPolygonMode() const override { return mPolygonMode; }
    void SetPolygonMode(EPolygonMode polygonMode) override { mPolygonMode = polygonMode; }
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
    EPolygonMode mPolygonMode = EPolygonMode::Line;
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
protected:
    virtual void UpdateRenderContext();
    systems::IRenderSystem *mRenderSystem = nullptr;
};

namespace systems {
struct SShaderHandle;
}
class CustomShaderMeshRenderComponent : public CDefaultMeshRenderComponent {
public:
    CustomShaderMeshRenderComponent(const CMeshBuffer &meshBuffer);
    void SetVertexShader(const char* shader);
    void SetFragmentShader(const char* shader);

    void OnInitialize(systems::IRenderSystem &renderSystem) override;
protected:
    void UpdateRenderContext() override;
    std::string mVertexShader;
    std::string mFragmentShader;
    const systems::SShaderHandle *mShaderHandle = nullptr;
};

class RenderToTextureComponent : public CustomShaderMeshRenderComponent {
public:
    RenderToTextureComponent(const CMeshBuffer &meshBuffer);

    void UpdateRenderContext() override;
    void OnInitialize(systems::IRenderSystem &renderSystem) override;
    void OnRender() override;
private:
    std::unique_ptr<CMeshBuffer> mQuadMesh;
    std::unique_ptr<systems::SRenderContextHandle> mQuadRenderContext;
    systems::STextureHandle *mTextureHandle = nullptr;
};

}// namespace unboxing_engine
