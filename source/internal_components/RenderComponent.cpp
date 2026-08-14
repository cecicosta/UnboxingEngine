#include "internal_components/RenderComponent.h"

#include "ShaderLibrary.h"
#include "Camera.h"
#include "MeshPrimitivesUtils.h"
#include "systems/IRenderSystem.h"

namespace unboxing_engine {
CDefaultMeshRenderComponent::CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
    : CRenderComponentBase(meshBuffer) {}

CDefaultMeshRenderComponent::~CDefaultMeshRenderComponent() {
    ReleaseRenderContext();
}

void CDefaultMeshRenderComponent::OnInitialize(systems::IRenderSystem & renderSystem) {
    mRenderSystem = &renderSystem;
    UpdateRenderContext();
}

void CDefaultMeshRenderComponent::ReleaseRenderContext() {
    if(mRenderSystem && mRenderContextHandle && mRenderContextHandle->renderBufferHandle) {
        mRenderSystem->EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
    }
    mRenderContextHandle.reset();
}

void CDefaultMeshRenderComponent::OnRender() {
    if (mIsDirty) {
        if(mRenderSystem && mRenderContextHandle && mRenderContextHandle->renderBufferHandle) {
            mRenderSystem->EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
        }
        UpdateRenderContext();
    }

    if(mRenderSystem && mRenderContextHandle) {
        mRenderSystem->Render(*mRenderContextHandle);
    }
}

void CDefaultMeshRenderComponent::UpdateRenderContext() {
    if(mRenderSystem) {
        const CMeshBuffer &meshBuffer = GetMeshBuffer();
        auto renderBufferHandle = mRenderSystem->WriteRenderBufferData(meshBuffer);
        auto shaderHandle = mRenderSystem->GetDefaultShader();
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(renderBufferHandle, shaderHandle, *mSceneComposite);
        mIsDirty = false;
    }
}

CustomShaderMeshRenderComponent::CustomShaderMeshRenderComponent(const CMeshBuffer &meshBuffer)
    : CDefaultMeshRenderComponent(meshBuffer) {
}

void CustomShaderMeshRenderComponent::SetVertexShader(const char *shader) {
    mVertexShader = shader;
}

void CustomShaderMeshRenderComponent::SetFragmentShader(const char *shader) {
    mFragmentShader = shader;
}
void CustomShaderMeshRenderComponent::OnInitialize(systems::IRenderSystem &renderSystem) {
    // TODO: See how to delete the shader later
    if (!mVertexShader.empty() && !mFragmentShader.empty()) {
        mShaderHandle = renderSystem.CompileShader(mVertexShader.c_str(), mFragmentShader.c_str());
    }
    if (!mShaderHandle) {
        mShaderHandle = renderSystem.GetDefaultShader();
    }
    CDefaultMeshRenderComponent::OnInitialize(renderSystem);
}

void CustomShaderMeshRenderComponent::UpdateRenderContext() {
    CDefaultMeshRenderComponent::UpdateRenderContext();
    if (mRenderContextHandle && mShaderHandle) {
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mShaderHandle,
                *mSceneComposite,
                nullptr,
                nullptr);
    }
}

RenderToTextureComponent::RenderToTextureComponent(const CMeshBuffer &meshBuffer)
: CustomShaderMeshRenderComponent(meshBuffer)
, mQuadMesh(primitive_utils::Quad()){
}

RenderToTextureComponent::~RenderToTextureComponent() {
    ReleaseRenderContext();
}

void RenderToTextureComponent::OnInitialize(systems::IRenderSystem &renderSystem) {
    mRenderSystem = &renderSystem;

    mTexturedstHandle = renderSystem.CreateTexture(
        renderSystem.GetCamera().mWidth,
        renderSystem.GetCamera().mHeight,
        systems::ETextureFormat::RGBA32F);
    if (!mTexturedstHandle) {
        return;
    }

    mRenderTarget = renderSystem.CreateTextureRenderTarget(
        mTexturedstHandle,
        systems::ERenderTargetKind::FloatingPointAccumulation);
    if (!mRenderTarget) {
        return;
    }

    CustomShaderMeshRenderComponent::OnInitialize(renderSystem);

    auto renderBufferHandle = mRenderSystem->WriteRenderBufferData(*mQuadMesh);
    auto shaderHandle = mRenderSystem->CompileShader(quad_render_vertex_shader, potential_region_fragment_shader);

    mQuadRenderContext = std::make_unique<systems::SRenderContextHandle>(
            renderBufferHandle,
            shaderHandle,
            *mSceneComposite,
            mTexturedstHandle);
}

void RenderToTextureComponent::UpdateRenderContext() {
    CustomShaderMeshRenderComponent::UpdateRenderContext();
    if (mRenderTarget && mRenderContextHandle) {
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mRenderContextHandle->shaderHandle,
                *mSceneComposite,
                mTextureSrcHandle,
                mRenderTarget);
    }
}

void RenderToTextureComponent::OnRender() {
    if (mIsDirty) {
        if(mRenderSystem && mRenderContextHandle && mRenderContextHandle->renderBufferHandle) {
            mRenderSystem->EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
        }
        UpdateRenderContext();
    }

    if(mRenderSystem && mRenderContextHandle && mQuadRenderContext) {
        mRenderSystem->Render(*mRenderContextHandle);
        //mRenderSystem->Render(*mQuadRenderContext);
    }
}

void RenderToTextureComponent::ReleaseRenderContext() {
    if (mRenderSystem && mQuadRenderContext && mQuadRenderContext->renderBufferHandle) {
        mRenderSystem->EraseRenderBufferData(*mQuadRenderContext->renderBufferHandle);
    }
    mQuadRenderContext.reset();
    CDefaultMeshRenderComponent::ReleaseRenderContext();
}

void RenderToTextureComponent::SetSrcTexture(systems::STextureHandle *texture) {
    mTextureSrcHandle = texture;
}

systems::STextureHandle *RenderToTextureComponent::GetDstTexture() const {
    return mTexturedstHandle;
}

RenderTextureComponent::RenderTextureComponent(std::unique_ptr<CMeshBuffer> meshBuffer)
: CustomShaderMeshRenderComponent(*meshBuffer)
, mMeshBuffer(std::move(meshBuffer)) {}

void RenderTextureComponent::OnInitialize(systems::IRenderSystem &renderSystem) {
    mShaderHandle = renderSystem.GetTexturePresentationShader();
    CustomShaderMeshRenderComponent::OnInitialize(renderSystem);
}

void RenderTextureComponent::SetTexture(systems::STextureHandle *texture) {
    mTexture = texture;
    // TODO: Review the "isDirty" mechanism. Maybe we can make it more robust and not neet updating manually
    UpdateRenderContext();
}

void RenderTextureComponent::UpdateRenderContext() {
    CustomShaderMeshRenderComponent::UpdateRenderContext();
    if (mTexture && mRenderContextHandle) {
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mRenderContextHandle->shaderHandle,
                *mSceneComposite,
                mTexture);
    }
}


}// namespace unboxing_engine
