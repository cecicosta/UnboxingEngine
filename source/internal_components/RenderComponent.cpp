#include "internal_components/RenderComponent.h"

unboxing_engine::CDefaultMeshRenderComponent::CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
    : CRenderComponentBase(meshBuffer) {
}
unboxing_engine::CDefaultMeshRenderComponent::~CDefaultMeshRenderComponent() {
    ReleaseRenderContext();
}


void unboxing_engine::CDefaultMeshRenderComponent::ReleaseRenderContext() {
    if(mRenderSystem) {
        mRenderSystem->EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
    }
    mRenderContextHandle.reset();
}
void unboxing_engine::CDefaultMeshRenderComponent::Render(systems::IRenderSystem &renderSystem) {
    if (mIsDirty) {
        mRenderSystem = &renderSystem;
        if(mRenderContextHandle) {
            renderSystem.EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
            const CMeshBuffer &meshBuffer = GetMeshBuffer();
            mRenderContextHandle->renderBufferHandle = renderSystem.WriteRenderBufferData(meshBuffer);
            mRenderContextHandle->shaderHandle = renderSystem.GetDefaultShader();
        }
        mIsDirty = false;
    }

    if(mRenderContextHandle) {
        renderSystem.Render(*mRenderContextHandle);
    }
}
