#include "systems/IRenderSystem.h"

namespace unboxing_engine::systems {

class COpenGLRenderSystem : public IRenderSystem {
public:
    COpenGLRenderSystem();
    virtual ~COpenGLRenderSystem();

    [[nodiscard]] bool Initialize(std::uint32_t width, std::uint32_t heigth) override;
    [[nodiscard]] std::unique_ptr<SShaderHandle> CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) override;
    [[nodiscard]] std::unique_ptr<SRenderBufferHandle> WriteRenderBufferData(const unboxing_engine::CMeshBuffer &meshBuffer) override;
    [[nodiscard]] void EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) override;

    void OnPreRender() override;
    void OnPostRender() override;
    void Render(const Camera& camera, const SRenderContextHandle &renderContextHandle) override;
};

}// namespace unboxing_engine
