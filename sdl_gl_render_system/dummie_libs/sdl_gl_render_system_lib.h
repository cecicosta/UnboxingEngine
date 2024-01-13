#pragma once

#include "systems/IRenderSystem.h"
#include "Camera.h"

namespace unboxing_engine::systems {

class COpenGLRenderSystem : public IRenderSystem {
public:
    COpenGLRenderSystem(const Camera& camera);
    ~COpenGLRenderSystem() override;

    [[nodiscard]] bool Initialize() override;
    [[nodiscard]] SShaderHandle* CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const override;
    [[nodiscard]] SRenderBufferHandle* WriteRenderBufferData(const CMeshBuffer &) override;
    void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) override;
    [[nodiscard]] const Camera &GetCamera() const override;
    [[nodiscard]] const SShaderHandle *GetDefaultShader() const override;
    void SetCamera(const Camera &camera) override;
    void Render(const SRenderContextHandle &) override;

    void OnPreRender() override;
    void OnPostRender() override;
    private: 
    const Camera* mCamera;
};

}// namespace unboxing_engine::systems
