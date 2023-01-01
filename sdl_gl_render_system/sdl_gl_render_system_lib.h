#pragma once

#include "systems/IRenderSystem.h"

namespace unboxing_engine::systems {

class COpenGLRenderSystem : public IRenderSystem {
public:
    COpenGLRenderSystem(const Camera& camera);
    ~COpenGLRenderSystem() override;

    [[nodiscard]] bool Initialize() override;
    [[nodiscard]] std::unique_ptr<SShaderHandle> CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const override;
    [[nodiscard]] std::unique_ptr<SRenderBufferHandle> WriteRenderBufferData(const unboxing_engine::CMeshBuffer &meshBuffer) override;
    void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) override;
    [[nodiscard]] const Camera &GetCamera() const override;
    [[nodiscard]] const SShaderHandle &GetShader(std::uint32_t id) const override;
    void SetCamera(const Camera &camera) override;
    void Render(const SRenderContextHandle &renderContextHandle) override;

    void OnPreRender() override;
    void OnPostRender() override;

private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};

}// namespace unboxing_engine::systems
