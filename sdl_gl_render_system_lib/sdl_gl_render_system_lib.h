// sdl_gl_render_system_lib.h : Header file for your target.

#pragma once

#include "systems/IRenderSystem.h"

namespace unboxing_engine::systems {

class COpenGLRenderSystem : public IRenderSystem {
public:
    COpenGLRenderSystem();
    ~COpenGLRenderSystem() override;

    [[nodiscard]] bool Initialize(std::uint32_t width, std::uint32_t heigth) override;
    [[nodiscard]] std::unique_ptr<SShaderHandle> CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const override;
    [[nodiscard]] std::unique_ptr<SRenderBufferHandle> WriteRenderBufferData(const unboxing_engine::CMeshBuffer &meshBuffer) const override;
    [[nodiscard]] void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) const override;

    void OnPreRender() override;
    void OnPostRender() override;
    void Render(const Camera &camera, const SRenderContextHandle &renderContextHandle) const override;

private:
    ///Window and opengl handlers
    SDL_Window *mWindow = nullptr;
    SDL_GLContext mGLContext = nullptr;
};

}// namespace unboxing_engine::systems
