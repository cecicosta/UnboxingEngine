#include "internal_components/RenderComponent.h"

#include "ShaderLibrary.h"
#include "Camera.h"
#include "MeshPrimitivesUtils.h"
#include "systems/IRenderSystem.h"

#include <algorithm>

namespace unboxing_engine {
namespace {

void SetTextureBinding(
    std::vector<systems::STextureBinding>& bindings,
    const std::string& uniformName,
    systems::STextureHandle* texture) {
    const auto existing = std::find_if(
        bindings.begin(),
        bindings.end(),
        [&uniformName](const systems::STextureBinding& binding) {
            return binding.uniformName == uniformName;
        });

    if (!texture) {
        if (existing != bindings.end()) {
            bindings.erase(existing);
        }
        return;
    }

    if (existing != bindings.end()) {
        existing->texture = texture;
    } else {
        bindings.push_back({uniformName, texture});
    }
}

}

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
    mRenderSystem = nullptr;
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

CustomShaderMeshRenderComponent::~CustomShaderMeshRenderComponent() {
    ReleaseRenderContext();
}

void CustomShaderMeshRenderComponent::SetVertexShader(const char *shader) {
    mVertexShader = shader;
    mIsDirty = true;
}

void CustomShaderMeshRenderComponent::SetFragmentShader(const char *shader) {
    mFragmentShader = shader;
    mIsDirty = true;
}
void CustomShaderMeshRenderComponent::OnInitialize(systems::IRenderSystem &renderSystem) {
    CDefaultMeshRenderComponent::OnInitialize(renderSystem);
}

void CustomShaderMeshRenderComponent::ReleaseRenderContext() {
    if (mRenderSystem && mOwnsShaderHandle && mShaderHandle) {
        mRenderSystem->EraseShaderData(*mShaderHandle);
    }
    mShaderHandle = nullptr;
    mOwnsShaderHandle = false;
    CDefaultMeshRenderComponent::ReleaseRenderContext();
}

void CustomShaderMeshRenderComponent::UpdateRenderContext() {
    CDefaultMeshRenderComponent::UpdateRenderContext();
    if (mRenderSystem) {
        if (mOwnsShaderHandle && mShaderHandle) {
            mRenderSystem->EraseShaderData(*mShaderHandle);
            mShaderHandle = nullptr;
            mOwnsShaderHandle = false;
        }
        if (!mVertexShader.empty() && !mFragmentShader.empty()) {
            mShaderHandle = mRenderSystem->CompileShader(mVertexShader.c_str(), mFragmentShader.c_str());
            mOwnsShaderHandle = mShaderHandle != nullptr;
        }
        if (!mShaderHandle) {
            mShaderHandle = mRenderSystem->GetDefaultShader();
        }
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mShaderHandle,
                *mSceneComposite,
                std::vector<systems::STextureBinding>{},
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
    renderSystem.SetRenderTargetClearEnabled(
        *mRenderTarget,
        mRenderTargetClearEnabled);

    CustomShaderMeshRenderComponent::OnInitialize(renderSystem);

    auto renderBufferHandle = mRenderSystem->WriteRenderBufferData(*mQuadMesh);
    mQuadShaderHandle = mRenderSystem->CompileShader(quad_render_vertex_shader, potential_region_fragment_shader);

    mQuadRenderContext = std::make_unique<systems::SRenderContextHandle>(
            renderBufferHandle,
            mQuadShaderHandle,
            *mSceneComposite,
            std::vector<systems::STextureBinding>{{"u_texture", mTexturedstHandle}});
}

void RenderToTextureComponent::UpdateRenderContext() {
    CustomShaderMeshRenderComponent::UpdateRenderContext();
    if (mRenderSystem && mRenderTarget) {
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mRenderContextHandle->shaderHandle,
                *mSceneComposite,
                mTextureSrcBindings,
                mRenderTarget);
    }
}

void RenderToTextureComponent::OnRender() {
    if (mIsDirty) {
        // TODO: Just calling EraseRenderBufferData no longer means a clear state for the object.
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
    auto* renderSystem = mRenderSystem;
    if (mRenderSystem && mQuadRenderContext && mQuadRenderContext->renderBufferHandle) {
        mRenderSystem->EraseRenderBufferData(*mQuadRenderContext->renderBufferHandle);
    }
    mQuadRenderContext.reset();
    if (mRenderSystem && mQuadShaderHandle) {
        mRenderSystem->EraseShaderData(*mQuadShaderHandle);
    }
    mQuadShaderHandle = nullptr;

    CustomShaderMeshRenderComponent::ReleaseRenderContext();

    if (renderSystem && mRenderTarget) {
        renderSystem->EraseRenderTargetData(*mRenderTarget);
    }
    mRenderTarget = nullptr;
    if (renderSystem && mTexturedstHandle) {
        renderSystem->EraseTextureData(*mTexturedstHandle);
    }
    mTexturedstHandle = nullptr;
}

void RenderToTextureComponent::SetSrcTexture(systems::STextureHandle *texture) {
    SetSrcTexture("u_texture", texture);
}

void RenderToTextureComponent::SetSrcTexture(
    const std::string& uniformName,
    systems::STextureHandle* texture) {
    SetTextureBinding(mTextureSrcBindings, uniformName, texture);
    mIsDirty = true;
}

void RenderToTextureComponent::SetRenderTargetClearEnabled(const bool enabled) {
    mRenderTargetClearEnabled = enabled;
    if (mRenderSystem && mRenderTarget) {
        mRenderSystem->SetRenderTargetClearEnabled(*mRenderTarget, enabled);
    }
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
    SetTexture("u_texture", texture);
}

void RenderTextureComponent::SetTexture(
    const std::string& uniformName,
    systems::STextureHandle* texture) {
    SetTextureBinding(mTextureBindings, uniformName, texture);
    mIsDirty = true;
}

void RenderTextureComponent::UpdateRenderContext() {
    CustomShaderMeshRenderComponent::UpdateRenderContext();
    if (mRenderSystem && !mTextureBindings.empty()) {
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mRenderContextHandle->shaderHandle,
                *mSceneComposite,
                mTextureBindings);
    }
}


}// namespace unboxing_engine
