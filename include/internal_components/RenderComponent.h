#pragma once

#include "IComponent.h"
#include "MeshBuffer.h"
#include "MeshPrimitivesUtils.h"
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
    ~RenderToTextureComponent() override;

    void UpdateRenderContext() override;
    void OnInitialize(systems::IRenderSystem &renderSystem) override;
    void OnRender() override;
    void ReleaseRenderContext() override;

    void SetSrcTexture(systems::STextureHandle* texture);
    [[nodiscard]] systems::STextureHandle* GetDstTexture() const;

private:
    std::unique_ptr<CMeshBuffer> mQuadMesh;
    std::unique_ptr<systems::SRenderContextHandle> mQuadRenderContext;
    systems::STextureHandle *mTextureSrcHandle = nullptr;
    systems::STextureHandle *mTexturedstHandle = nullptr;
    systems::SRenderTarget * mRenderTarget = nullptr;
};

class RenderTextureComponent : public CustomShaderMeshRenderComponent {


public:
    RenderTextureComponent(std::unique_ptr<CMeshBuffer> meshBuffer  = primitive_utils::Quad()); // TODO: to create a Quad a default parameter was a workaround to create the mesh before the parent class initialization. We need to fix the storaging of the mesh on the base classes
    void OnInitialize(systems::IRenderSystem &renderSystem) override;
        void SetTexture(systems::STextureHandle* texture);

protected:
    void UpdateRenderContext() override;


private:
    std::unique_ptr<CMeshBuffer> mMeshBuffer;
    systems::STextureHandle *mTexture = nullptr;
};

}// namespace unboxing_engine
