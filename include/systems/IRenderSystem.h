#pragma once

#include "Matrix.h"
#include "material.h"
#include <cstdint>
#include <memory>

#include "CoreEvents.h"

namespace unboxing_engine {
class CMeshBuffer;
class Camera;
}

namespace unboxing_engine::systems {

struct SShaderHandle;
struct SRenderBufferHandle;
struct SRenderContextHandle {
    std::unique_ptr<SRenderBufferHandle> renderBufferHandle;
    const SShaderHandle *shaderHandle;
    const Matrix<float, 4, 4> *transformation;
    const SMaterial *material;
    int compositeId;
};

class IRenderSystem: public UListener<core_events::IPreRenderListener, core_events::IPostRenderListener> {
public:
    virtual ~IRenderSystem() = default;

    [[nodiscard]] virtual bool Initialize() = 0;
    [[nodiscard]] virtual std::unique_ptr<SShaderHandle> CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const = 0;
    [[nodiscard]] virtual std::unique_ptr<SRenderBufferHandle> WriteRenderBufferData(const unboxing_engine::CMeshBuffer &meshBuffer) = 0;
    [[nodiscard]] virtual void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) = 0;
    [[nodiscard]] virtual const Camera &GetCamera() const = 0;
    [[nodiscard]] virtual const SShaderHandle &GetShader(std::uint32_t id) const = 0;
    virtual void SetCamera(const Camera& camera) = 0;

    virtual void Render(const SRenderContextHandle &renderContextHandle) = 0;
};

}// namespace unboxing_engine
