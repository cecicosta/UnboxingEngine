#include "sdl_gl_render_system_lib.h"

namespace unboxing_engine::systems {

COpenGLRenderSystem::COpenGLRenderSystem(const Camera& camera): mCamera(&camera) {}
COpenGLRenderSystem::~COpenGLRenderSystem() = default;

bool COpenGLRenderSystem::Initialize() { return true; }
SShaderHandle* COpenGLRenderSystem::CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const {return nullptr; }
void COpenGLRenderSystem::EraseShaderData(const SShaderHandle &) {}
SRenderBufferHandle* COpenGLRenderSystem::WriteRenderBufferData(const CMeshBuffer &) { return nullptr; }
void COpenGLRenderSystem::EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) {}
const Camera &COpenGLRenderSystem::GetCamera() const { return *mCamera; }
const SShaderHandle *COpenGLRenderSystem::GetDefaultShader() const { return nullptr; }
const SShaderHandle *COpenGLRenderSystem::GetTexturePresentationShader() const { return nullptr; }
const IRenderDebug &COpenGLRenderSystem::GetRenderDebug() const { return *this; }
std::optional<STextureSnapshot> COpenGLRenderSystem::CaptureTexture(
    const STextureHandle &,
    const STextureInspectionOptions &) const { return std::nullopt; }
std::optional<STextureStatistics> COpenGLRenderSystem::InspectTexture(
    const STextureHandle &,
    const STextureInspectionOptions &) const { return std::nullopt; }
bool COpenGLRenderSystem::PrintTextureStatistics(
    const STextureHandle &,
    const std::string &,
    const STextureInspectionOptions &) const { return false; }
void COpenGLRenderSystem::SetCamera(const Camera &camera) { mCamera = &camera; }
void COpenGLRenderSystem::Render(const SRenderContextHandle &) {};
STextureHandle *COpenGLRenderSystem::CreateTexture(uint32_t, uint32_t, ETextureFormat) { return nullptr; }
void COpenGLRenderSystem::EraseTextureData(const STextureHandle &) {}
SRenderTarget *COpenGLRenderSystem::CreateTextureRenderTarget(STextureHandle *, ERenderTargetKind) { return nullptr; }
void COpenGLRenderSystem::SetRenderTargetClearEnabled(SRenderTarget &, bool) {}
void COpenGLRenderSystem::EraseRenderTargetData(const SRenderTarget &) {}

void COpenGLRenderSystem::OnPreRender() {}
void COpenGLRenderSystem::OnPostRender() {}

}
