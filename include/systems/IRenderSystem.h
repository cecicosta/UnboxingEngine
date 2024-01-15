#pragma once

#include "Matrix.h"
#include "material.h"
#include <cstdint>
#include <memory>

#include "CoreEvents.h"

namespace unboxing_engine {
class CSceneComposite;
class CMeshBuffer;
class Camera;
}

namespace unboxing_engine::systems {

struct SShaderHandle;
struct SRenderBufferHandle;
struct SRenderContextHandle {
    SRenderContextHandle(const SRenderBufferHandle* _renderBufferHandle, const SShaderHandle* _shaderHandle, const CSceneComposite& _sceneComposite)
    : renderBufferHandle(_renderBufferHandle)
    , shaderHandle(_shaderHandle)
    , sceneComposite(_sceneComposite) {}
    const SRenderBufferHandle *renderBufferHandle;
    const SShaderHandle *shaderHandle;
    const CSceneComposite &sceneComposite;
};

class IRenderSystem: public UListener<core_events::IPreRenderListener, core_events::IPostRenderListener> {
public:
    virtual ~IRenderSystem() = default;

    [[nodiscard]] virtual bool Initialize() = 0;
    [[nodiscard]] virtual SShaderHandle* CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const = 0;
    [[nodiscard]] virtual SRenderBufferHandle* WriteRenderBufferData(const CMeshBuffer &meshBuffer) = 0;
    [[nodiscard]] virtual void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) = 0;
    [[nodiscard]] virtual const Camera &GetCamera() const = 0;
    [[nodiscard]] virtual const SShaderHandle *GetDefaultShader() const = 0;
    virtual void SetCamera(const Camera& camera) = 0;

    virtual void Render(const SRenderContextHandle &renderContextHandle) = 0;
};

}// namespace unboxing_engine
