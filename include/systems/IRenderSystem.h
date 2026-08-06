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
struct STextureHandle;
struct SRenderTarget;

enum class ETextureFormat {
    RGBA32F,
    RGBA8U,
};

enum class ERenderTargetKind {
    DefaultFramebuffer,
    FloatingPointAccumulation
};

struct SRenderContextHandle {
    SRenderContextHandle(const SRenderBufferHandle* _renderBufferHandle, const SShaderHandle* _shaderHandle, const CSceneComposite& _sceneComposite, const STextureHandle* _textureHandle = nullptr, const SRenderTarget* _renderTarget = nullptr, ERenderTargetKind _renderTargetKind = ERenderTargetKind::DefaultFramebuffer)
    : renderBufferHandle(_renderBufferHandle)
    , shaderHandle(_shaderHandle)
    , sceneComposite(_sceneComposite)
    , textureHandle(_textureHandle)
    , renderTarget(_renderTarget)
    , renderTargetKind(_renderTargetKind) {}
    const SRenderBufferHandle *renderBufferHandle;
    const SShaderHandle *shaderHandle;
    const CSceneComposite &sceneComposite;
    const STextureHandle *textureHandle;
    const SRenderTarget* renderTarget;
    ERenderTargetKind renderTargetKind;
};

class IRenderSystem: public UListener<core_events::IPreRenderListener, core_events::IPostRenderListener> {
public:
    ~IRenderSystem() override = default;

    [[nodiscard]] virtual bool Initialize() = 0;
    [[nodiscard]] virtual SShaderHandle* CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const = 0;
    [[nodiscard]] virtual SRenderBufferHandle* WriteRenderBufferData(const CMeshBuffer &meshBuffer) = 0;
    virtual void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) = 0;
    [[nodiscard]] virtual const Camera &GetCamera() const = 0;
    [[nodiscard]] virtual const SShaderHandle *GetDefaultShader() const = 0;
    virtual void SetCamera(const Camera& camera) = 0;
    virtual void Render(const SRenderContextHandle &renderContextHandle) = 0;
    virtual STextureHandle *CreateTexture(uint32_t width, uint32_t height, ETextureFormat format) = 0;
    virtual SRenderTarget *CreateTextureRenderTarget(uint32_t width, uint32_t height, ETextureFormat format) = 0;
};

}// namespace unboxing_engine
