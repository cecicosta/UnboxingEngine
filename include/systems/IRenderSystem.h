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
    std::unique_ptr<SShaderHandle> shaderHandle;
    const Matrix<float, 4, 4> *transformation;
    const SMaterial *material;
    int compositeId;
};

class IRenderSystem: public UListener<core_events::IPreRenderListener, core_events::IPostRenderListener> {
public:
    virtual ~IRenderSystem() = default;

    [[nodiscard]] virtual bool Initialize(std::uint32_t width, std::uint32_t heigth) = 0;
    [[nodiscard]] virtual std::unique_ptr<SShaderHandle> CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) = 0;
    [[nodiscard]] virtual std::unique_ptr<SRenderBufferHandle> WriteRenderBufferData(const unboxing_engine::CMeshBuffer &meshBuffer) = 0;
    [[nodiscard]] virtual void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) = 0;

    virtual void Render(const Camera &camera, const SRenderContextHandle &renderContextHandle) = 0;
};

}// namespace unboxing_engine
