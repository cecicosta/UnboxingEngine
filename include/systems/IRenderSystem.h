#pragma once

#include "Matrix.h"
#include "material.h"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

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
class IRenderDebug;

enum class ETextureFormat {
    RGBA32F,
    RGBA8U,
};

enum class ERenderTargetBlendMode {
    Overwrite,
    Additive,
    Alpha
};

struct STextureBinding {
    std::string uniformName;
    const STextureHandle *texture = nullptr;
};

struct SRenderContextHandle {
    SRenderContextHandle(const SRenderBufferHandle* _renderBufferHandle, const SShaderHandle* _shaderHandle, const CSceneComposite& _sceneComposite, std::vector<STextureBinding> _textureBindings = {}, const SRenderTarget* _renderTarget = nullptr)
    : renderBufferHandle(_renderBufferHandle)
    , shaderHandle(_shaderHandle)
    , sceneComposite(_sceneComposite)
    , textureBindings(std::move(_textureBindings))
    , renderTarget(_renderTarget) {}
    const SRenderBufferHandle *renderBufferHandle;
    const SShaderHandle *shaderHandle;
    const CSceneComposite &sceneComposite;
    std::vector<STextureBinding> textureBindings;
    const SRenderTarget* renderTarget;
    float colorScale = 1.0f; // TODO: Embed this parameter into the shader handle. Follow the model of SetRenderTargetBlendMode. Perhaps for Shader handle, a method that would allow to add generic parameters to the shader.
};

class IRenderSystem: public UListener<core_events::IPreRenderListener, core_events::IPostRenderListener> {
public:
    ~IRenderSystem() override = default;

    [[nodiscard]] virtual bool Initialize() = 0;
    [[nodiscard]] virtual SShaderHandle* CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const = 0;
    virtual void EraseShaderData(const SShaderHandle &shaderHandle) = 0;
    [[nodiscard]] virtual SRenderBufferHandle* WriteRenderBufferData(const CMeshBuffer &meshBuffer) = 0;
    virtual void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) = 0;
    [[nodiscard]] virtual const Camera &GetCamera() const = 0;
    [[nodiscard]] virtual const SShaderHandle *GetDefaultShader() const = 0;
    [[nodiscard]] virtual const SShaderHandle *GetTexturePresentationShader() const = 0;
    [[nodiscard]] virtual const IRenderDebug &GetRenderDebug() const = 0;
    virtual void SetCamera(const Camera& camera) = 0;
    virtual void Render(const SRenderContextHandle &renderContextHandle) = 0;
    virtual STextureHandle *CreateTexture(uint32_t width, uint32_t height, ETextureFormat format) = 0;
    virtual void EraseTextureData(const STextureHandle &textureHandle) = 0;
    virtual SRenderTarget *CreateTextureRenderTarget(STextureHandle *textureHandle, ERenderTargetBlendMode blendMode) = 0;
    virtual void SetRenderTargetClearEnabled(SRenderTarget &renderTarget, bool enabled) = 0;
    virtual void SetRenderTargetBlendMode(SRenderTarget &renderTarget, ERenderTargetBlendMode blendMode) = 0;
    virtual void EraseRenderTargetData(const SRenderTarget &renderTarget) = 0;
};

}// namespace unboxing_engine
