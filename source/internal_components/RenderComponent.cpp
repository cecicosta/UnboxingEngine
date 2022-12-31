#include "internal_components/RenderComponent.h"

#include "systems/IRenderSystem.h"


unboxing_engine::CDefaultMeshRenderComponent::CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
    : CRenderComponentBase(meshBuffer) {
}

void unboxing_engine::CDefaultMeshRenderComponent::Render(const systems::IRenderSystem &renderSystem) {
    if (mIsDirty) {
        renderSystem.EraseRenderBufferData(*mRenderContextHandle.renderBufferHandle);
        const CMeshBuffer &meshBuffer = GetMeshBuffer();
        mRenderContextHandle.renderBufferHandle = renderSystem.WriteRenderBufferData(meshBuffer);
        mRenderContextHandle.shaderHandle = &renderSystem.GetShader(0);
        mIsDirty = false;
    }

    renderSystem.Render(mRenderContextHandle);
}
