#include "sdl_gl_render_system_lib.h"

namespace unboxing_engine::systems {

COpenGLRenderSystem::COpenGLRenderSystem(const Camera& camera): mCamera(&camera) {}
COpenGLRenderSystem::~COpenGLRenderSystem() = default;

bool COpenGLRenderSystem::Initialize() { return true; }
SShaderHandle* COpenGLRenderSystem::CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const {return nullptr; }
SRenderBufferHandle* COpenGLRenderSystem::WriteRenderBufferData(const CMeshBuffer &) { return nullptr; }
void COpenGLRenderSystem::EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) {}
const Camera &COpenGLRenderSystem::GetCamera() const { return *mCamera; }
const SShaderHandle *COpenGLRenderSystem::GetDefaultShader() const { return nullptr; }
const SShaderHandle *COpenGLRenderSystem::GetTexturePresentationShader() const { return nullptr; }
void COpenGLRenderSystem::SetCamera(const Camera &camera) { mCamera = &camera; }
void COpenGLRenderSystem::Render(const SRenderContextHandle &) {};
STextureHandle *COpenGLRenderSystem::CreateTexture(uint32_t, uint32_t, ETextureFormat) { return nullptr; }
SRenderTarget *COpenGLRenderSystem::CreateTextureRenderTarget(STextureHandle *, ERenderTargetKind) { return nullptr; }

void COpenGLRenderSystem::OnPreRender() {}
void COpenGLRenderSystem::OnPostRender() {}

}
