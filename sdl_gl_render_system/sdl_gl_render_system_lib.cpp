#include "sdl_gl_render_system_lib.h"

#include "ShaderLibrary.h"
#include "Camera.h"
#include "MeshBuffer.h"
#include "SceneComposite.h"
#include "SceneCompositeLibrary/BasicSceneCompositeTypes.h"
#include "internal_components/IRenderComponent.h"

#include <GL/glew.h>
#include <SDL.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>



namespace {
void GetError() {
    static int error_count = 0;
    auto gl_error = glGetError();
    std::cout << "Get error count: " << error_count++ << std::endl;
    if (gl_error != 0) {
        std::cout << "Code: " << gl_error << std::endl;
    }
}

typedef enum t_attrib_id {
    attrib_position
} t_attrib_id;


void CreateView(std::uint32_t width, std::uint32_t heigth) {
    glClearColor(0.02f, 0.025f, 0.035f, 1.0f);
    glClearDepth(1.0f);
    glViewport(0, 0, static_cast<GLint>(width), static_cast<GLint>(heigth));
}

}// namespace

namespace unboxing_engine::systems {

struct SShaderHandle {
    ///Basic shader handler
    std::uint32_t program = -1;
};
struct SRenderBufferHandle {
    std::uint32_t vao = -1;  //Refers to the whole render context, including geometry, shaders and parameters
    std::uint32_t vbo = -1;  //Buffer handler for geometry
    std::uint32_t ebo = -1;  //Buffer handler for geometry vertex indices
    std::uint32_t ntriangles{};//Number of triangles the geometry has
};

struct STextureHandle {
    std::uint32_t texture = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    ETextureFormat format;
};

struct SRenderTarget {
    std::uint32_t framebuffer = 0;
    std::uint32_t texture = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    STextureHandle *textureHandle = nullptr;
    ERenderTargetKind renderTargetKind;
    bool clearEnabled = true;
};


class COpenGLRenderSystem::Impl {
public:
    Impl(const Camera& camera) : mCamera(camera) {}
    ~Impl() {
        ReleaseTextureRenderTargets();
        ReleaseRenderBuffers();
        ReleaseShaders();
       
        SDL_GL_DeleteContext(mGLContext);
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
    }

    [[nodiscard]] bool Initialize() {
        //Initialize SDL subsystems
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            std::cout << "Video initialization failed: " << SDL_GetError() << std::endl;
            return false;
        }

        //Initialize SDL_ttf
        //TTF_Init();

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);


        //Creates the window
        mWindow = SDL_CreateWindow("My Game Window",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   static_cast<GLint>(mCamera.mWidth),
                                   static_cast<GLint>(mCamera.mHeight),
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        // Create an OpenGL context associated with the window
        mGLContext = SDL_GL_CreateContext(mWindow);

        //TODO: Verify if by creating a new window, a new context needs to be
        // created and glew needs to be initialize for that context
        auto init_res = glewInit();
        if (init_res != GLEW_OK) {
            std::cout << glewGetErrorString(glewInit()) << std::endl;
            return false;
        }

        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

        (void) CompileShader(vertex_shader_source, fragment_shader_source);
        mSignedTextureDebugShader = CompileShader(signed_texture_debug_vertex_shader_source, signed_texture_debug_fragment_shader_source);
        CreateView(mCamera.mWidth, mCamera.mHeight);

        SRenderTarget renderTarget{0, 0, mCamera.mWidth, mCamera.mHeight, nullptr, ERenderTargetKind::DefaultFramebuffer};
        mRenderTarget.emplace(0, std::make_unique<SRenderTarget>(renderTarget));
        return true;
    }

    [[nodiscard]] SShaderHandle* CompileShader(const char* vertexShaderSrc, const char* fragmentShaderSrc) {
        unsigned int vertexShader;
        {
            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            int length = static_cast<int>(strlen(vertexShaderSrc));
            glShaderSource(vertexShader, 1, &vertexShaderSrc, &length);
            glCompileShader(vertexShader);

            int success;
            char infoLog[512];
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
                std::cout << "Vertex shader compilation failed: \n"
                          << infoLog << std::endl;
                glDeleteShader(vertexShader);
                return nullptr;
            }
        }

        // Create and compile fragment shader
        unsigned int fragmentShader;
        {
            fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            int length = static_cast<int>(strlen(fragmentShaderSrc));
            glShaderSource(fragmentShader, 1, &fragmentShaderSrc, &length);
            glCompileShader(fragmentShader);

            int success;
            char infoLog[512];
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
                std::cout << "Fragment shader compilation failed:\n"
                          << infoLog << std::endl;
                glDeleteShader(fragmentShader);
                glDeleteShader(vertexShader);
                return nullptr;
            }
        }

        // Create program and link shaders
        auto program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glBindAttribLocation(program, attrib_position, "i_position");

        glLinkProgram(program);

        {
            int success;
            char infoLog[512];
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(program, 512, nullptr, infoLog);
                std::cout << "Compiling shader program failed: \n"
                          << infoLog << std::endl;
                glDeleteProgram(program);
                glDeleteShader(fragmentShader);
                glDeleteShader(vertexShader);
                return nullptr;
            }
        }

        glUseProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        auto handle = std::make_unique<SShaderHandle>();
        handle->program = program;
        mShaders.emplace_back(std::move(handle));
        return mShaders[mShaders.size() - 1].get();
    }

    void EraseShaderData(const SShaderHandle& shaderHandle) {
        const auto shader = std::find_if(
            mShaders.begin(),
            mShaders.end(),
            [&shaderHandle](const auto& ownedShader) {
                return ownedShader.get() == &shaderHandle;
            });
        if (shader == mShaders.end()) {
            return;
        }

        if (mSignedTextureDebugShader == shader->get()) {
            mSignedTextureDebugShader = nullptr;
        }
        if ((*shader)->program != 0) {
            glDeleteProgram((*shader)->program);
        }
        mShaders.erase(shader);
    }

    [[nodiscard]] SRenderBufferHandle* WriteRenderBufferData(const unboxing_engine::CMeshBuffer& meshBuffer) {
        auto handle = std::make_unique<SRenderBufferHandle>();
        glGenVertexArrays(1, &handle->vao);
        glGenBuffers(1, &handle->vbo);
        glGenBuffers(1, &handle->ebo);

        glBindVertexArray(handle->vao);

        glBindBuffer(GL_ARRAY_BUFFER, handle->vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * meshBuffer.nvertices * sizeof(float), meshBuffer.vertices.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(attrib_position);
        glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) (0));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * meshBuffer.nfaces * sizeof(unsigned int), meshBuffer.triangles.data(), GL_DYNAMIC_DRAW);
        handle->ntriangles = meshBuffer.nfaces;

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        mRenderBuffers.emplace_back(std::move(handle));
        return mRenderBuffers[mRenderBuffers.size() - 1].get();
    }

    void EraseRenderBufferData(const SRenderBufferHandle& renderBufferHandle) {
        const auto renderBuffer = std::find_if(
            mRenderBuffers.begin(),
            mRenderBuffers.end(),
            [&renderBufferHandle](const auto& ownedBuffer) {
                return ownedBuffer.get() == &renderBufferHandle;
            });
        if (renderBuffer == mRenderBuffers.end()) {
            return;
        }

        glDeleteVertexArrays(1, &(*renderBuffer)->vao);
        glDeleteBuffers(1, &(*renderBuffer)->vbo);
        glDeleteBuffers(1, &(*renderBuffer)->ebo);
        mRenderBuffers.erase(renderBuffer);
    }
    [[nodiscard]] const Camera& GetCamera() const {
        return mCamera;
    }
    [[nodiscard]] const SShaderHandle* GetDefaultShader() const {
        return mShaders.begin() != mShaders.end() ? mShaders.begin()->get() : nullptr;
    }
    [[nodiscard]] const SShaderHandle* GetTexturePresentationShader() const {
        return mSignedTextureDebugShader;
    }
    void SetCamera(const Camera& camera) {
        mCamera = camera;
    }
    void Render(const SRenderContextHandle& renderContextHandle) {
        // TODO: Should store all render contexts and group by render configurations, so the render pass can avoid multiple redundant operations
        auto* renderTarget = renderContextHandle.renderTarget;
        if (!renderTarget) {
            renderTarget = mRenderTarget.at(0).get();
        }

        SetRenderTarget(*renderTarget);
        DrawRenderContext(renderContextHandle);
    }

    void DrawRenderContext(const SRenderContextHandle& renderContextHandle) const {
        if (!renderContextHandle.shaderHandle || !renderContextHandle.renderBufferHandle) {
            return;
        }

        const auto program = renderContextHandle.shaderHandle->program;
        glUseProgram(program);

        const SMaterial *material = nullptr;
        EPolygonMode polygonMode = EPolygonMode::Line;
        if(auto renderComponent = renderContextHandle.sceneComposite.GetComponent<IRenderComponent>()) {
            material = &renderComponent->GetMaterial();
            polygonMode = renderComponent->GetPolygonMode();
        }

        if (!material) {
            return;
        }

        glPolygonMode(GL_FRONT_AND_BACK,
                      !renderContextHandle.textureBindings.empty() || polygonMode == EPolygonMode::Fill ? GL_FILL : GL_LINE);

        auto modelMatrix = renderContextHandle.sceneComposite.GetWorldTransformation();
        auto viewMatrix = mCamera.GetViewMatrix();
        auto projectionMatrix = mCamera.GetProjectionMatrix();

        const GLint modelUniform = glGetUniformLocation(program, "u_model_matrix");
        if (modelUniform >= 0) {
            glUniformMatrix4fv(modelUniform, 1, GL_TRUE, modelMatrix.ToArray());
        }
        const GLint viewUniform = glGetUniformLocation(program, "u_view_matrix");
        if (viewUniform >= 0) {
            glUniformMatrix4fv(viewUniform, 1, GL_TRUE, viewMatrix.ToArray());
        }
        const GLint projectionUniform = glGetUniformLocation(program, "u_projection_matrix");
        if (projectionUniform >= 0) {
            glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, projectionMatrix.ToArray());
        }
        const GLint cameraPositionUniform = glGetUniformLocation(program, "u_camera_world_position");
        if (cameraPositionUniform >= 0) {
            const auto cameraPosition = mCamera.GetPosition();
            glUniform3fv(cameraPositionUniform, 1, cameraPosition.ToArray());
        }
        const GLint cameraFarUniform = glGetUniformLocation(program, "u_camera_far");
        if (cameraFarUniform >= 0) {
            glUniform1f(cameraFarUniform, mCamera.mZFar);
        }
        const GLint colorUniform = glGetUniformLocation(program, "color");
        if (colorUniform >= 0) {
            glUniform4fv(colorUniform, 1, material->materialDif);
        }
        for (size_t unit = 0; unit < renderContextHandle.textureBindings.size(); ++unit) {
            const auto& binding = renderContextHandle.textureBindings[unit];
            if (!binding.texture) {
                continue;
            }

            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
            glBindTexture(GL_TEXTURE_2D, binding.texture->texture);

            const GLint textureUniform = glGetUniformLocation(program, binding.uniformName.c_str());
            if (textureUniform >= 0) {
                glUniform1i(textureUniform, static_cast<GLint>(unit));
            }
        }

        const GLint visualizationScaleUniform = glGetUniformLocation(program, "u_visualization_scale");
        if (visualizationScaleUniform >= 0) {
            glUniform1f(visualizationScaleUniform, renderContextHandle.colorScale);
        }

        glBindVertexArray(renderContextHandle.renderBufferHandle->vao);
        glDrawElements(GL_TRIANGLES, renderContextHandle.renderBufferHandle->ntriangles * 3, GL_UNSIGNED_INT, nullptr);

        for (size_t unit = 0; unit < renderContextHandle.textureBindings.size(); ++unit) {
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTexture(GL_TEXTURE0);

    }

    void OnPreRender() {
        for (const auto &entry : mRenderTarget) {
            const auto &renderTarget = *entry.second;
            if (!renderTarget.clearEnabled) {
                continue;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.framebuffer);
            glViewport(0, 0, static_cast<GLint>(renderTarget.width), static_cast<GLint>(renderTarget.height));

            if (renderTarget.framebuffer == 0) {
                glClearColor(0.02f, 0.025f, 0.035f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            } else {
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
        }

        const auto &defaultTarget = *mRenderTarget.at(0);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultTarget.framebuffer);
        glViewport(0, 0, static_cast<GLint>(defaultTarget.width), static_cast<GLint>(defaultTarget.height));
    }
    void OnPostRender() {
        glDepthMask(GL_TRUE);
        SDL_GL_SwapWindow(mWindow);
        SDL_Delay(1);
    }

    STextureHandle * CreateTexture(const uint32_t width, const uint32_t height, const ETextureFormat format) {
        uint32_t textureId;

        GLint glFormat;
        GLenum glType;

        switch (format) {
            case ETextureFormat::RGBA32F:
            glFormat = GL_RGBA32F;
            glType = GL_FLOAT;
            break;
            case ETextureFormat::RGBA8U:
            default:
            glFormat = GL_RGBA;
            glType = GL_UNSIGNED_BYTE;
        };

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     glFormat,
                     static_cast<GLsizei>(width),
                     static_cast<GLsizei>(height),
                     0,
                     GL_RGBA,
                     glType,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
        STextureHandle textureHandle{textureId, width, height, format};
        mTextures.emplace(textureId, std::make_unique<STextureHandle>(textureHandle));
        return mTextures.at(textureId).get();
    }

    void EraseTextureData(const STextureHandle& textureHandle) {
        const auto texture = mTextures.find(textureHandle.texture);
        if (texture == mTextures.end() || texture->second.get() != &textureHandle) {
            return;
        }

        if (texture->second->texture != 0) {
            glDeleteTextures(1, &texture->second->texture);
        }
        mTextures.erase(texture);
    }

    [[nodiscard]] std::optional<STextureSnapshot> CaptureTexture(
        const STextureHandle& textureHandle,
        const STextureInspectionOptions& options) const {
        const auto texture = mTextures.find(textureHandle.texture);
        if (texture == mTextures.end() || texture->second.get() != &textureHandle ||
            textureHandle.width == 0 || textureHandle.height == 0) {
            return std::nullopt;
        }

        const size_t width = textureHandle.width;
        const size_t height = textureHandle.height;
        if (height > std::numeric_limits<size_t>::max() / width / 4) {
            return std::nullopt;
        }

        std::vector<float> pixels(width * height * 4);
        GLint previousTexture = 0;
        GLint previousPixelPackBuffer = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
        glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &previousPixelPackBuffer);

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, textureHandle.texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
        glBindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(previousPixelPackBuffer));

        const auto statistics = CalculateTextureStatistics(
            pixels.data(),
            pixels.size(),
            textureHandle.width,
            textureHandle.height,
            textureHandle.format,
            options);
        if (!statistics) {
            return std::nullopt;
        }

        STextureSnapshot snapshot;
        snapshot.width = textureHandle.width;
        snapshot.height = textureHandle.height;
        snapshot.format = textureHandle.format;
        snapshot.rgbaPixels = std::move(pixels);
        snapshot.statistics = *statistics;
        return snapshot;
    }

    SRenderTarget* CreateTextureRenderTarget(STextureHandle *textureHandle, const ERenderTargetKind renderTargetKind) {
        if (!textureHandle || textureHandle->texture == 0) {
            return nullptr;
        }

        SRenderTarget renderTarget;
        renderTarget.textureHandle = textureHandle;
        renderTarget.width = textureHandle->width;
        renderTarget.height = textureHandle->height;
        renderTarget.renderTargetKind = renderTargetKind;

        glGenFramebuffers(1, &renderTarget.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.framebuffer);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget.textureHandle->texture, 0);
        const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);

        const GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (framebufferStatus == GL_FRAMEBUFFER_COMPLETE) {
            const GLfloat initialColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
            glClearBufferfv(GL_COLOR, 0, initialColor);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Texture framebuffer initialization failed: " << framebufferStatus << std::endl;
            glDeleteFramebuffers(1, &renderTarget.framebuffer);
            return nullptr;
        }

        mRenderTarget.emplace(renderTarget.framebuffer, std::make_unique<SRenderTarget>(renderTarget));
        return mRenderTarget.at(renderTarget.framebuffer).get();
    }

    void SetRenderTargetClearEnabled(SRenderTarget& renderTarget, const bool enabled) {
        const auto target = mRenderTarget.find(renderTarget.framebuffer);
        if (target == mRenderTarget.end() || target->second.get() != &renderTarget) {
            return;
        }

        target->second->clearEnabled = enabled;
    }

    void EraseRenderTargetData(const SRenderTarget& renderTarget) {
        if (renderTarget.framebuffer == 0) {
            return;
        }

        const auto target = mRenderTarget.find(renderTarget.framebuffer);
        if (target == mRenderTarget.end() || target->second.get() != &renderTarget) {
            return;
        }

        glDeleteFramebuffers(1, &target->second->framebuffer);
        mRenderTarget.erase(target);
    }

    static void SetRenderTarget(const SRenderTarget &renderTarget) {
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.framebuffer);
        glViewport(0, 0, static_cast<GLint>(renderTarget.width), static_cast<GLint>(renderTarget.height));

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);

        switch (renderTarget.renderTargetKind) {
            case ERenderTargetKind::FloatingPointAccumulation:
                glBlendFunc(GL_ONE, GL_ONE);
            break;
            case ERenderTargetKind::DefaultFramebuffer:
            default:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    void ReleaseTextureRenderTargets() {
        for (const auto &entry : mRenderTarget) {
            if (entry.first != 0 && entry.second->framebuffer != 0) {
                glDeleteFramebuffers(1, &entry.second->framebuffer);
            }
        }
        mRenderTarget.clear();
        for (const auto &entry : mTextures) {
            if (entry.second && entry.second->texture != 0) {
                glDeleteTextures(1, &entry.second->texture);
            }
        }
        mTextures.clear();
    }

    void ReleaseRenderBuffers() {
        for (const auto& renderBuffer : mRenderBuffers) {
            glDeleteVertexArrays(1, &renderBuffer->vao);
            glDeleteBuffers(1, &renderBuffer->vbo);
            glDeleteBuffers(1, &renderBuffer->ebo);
        }
        mRenderBuffers.clear();
    }

    void ReleaseShaders() {
        for (const auto& shader : mShaders) {
            if (shader->program != 0) {
                glDeleteProgram(shader->program);
            }
        }
        mShaders.clear();
        mSignedTextureDebugShader = nullptr;
    }

private:
    ///Window and opengl handlers
    SDL_Window *mWindow = nullptr;
    SDL_GLContext mGLContext;
    Camera mCamera;
    SShaderHandle *mSignedTextureDebugShader = nullptr;
    std::vector<std::unique_ptr<SShaderHandle>> mShaders;
    std::vector<std::unique_ptr<SRenderBufferHandle>> mRenderBuffers;
    std::unordered_map<uint32_t, std::unique_ptr<STextureHandle>> mTextures;
    std::unordered_map<uint32_t, std::unique_ptr<SRenderTarget>> mRenderTarget;
};

COpenGLRenderSystem::COpenGLRenderSystem(const Camera& camera)
    : mImpl(std::make_unique<Impl>(camera)) {}

COpenGLRenderSystem::~COpenGLRenderSystem() = default;

bool COpenGLRenderSystem::Initialize() {
    return mImpl->Initialize(); 
}

SShaderHandle* COpenGLRenderSystem::CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) const {
    return mImpl->CompileShader(vertexShaderSrc, fragmentShaderSrc);
}

void COpenGLRenderSystem::EraseShaderData(const SShaderHandle &shaderHandle) {
    mImpl->EraseShaderData(shaderHandle);
}

SRenderBufferHandle* COpenGLRenderSystem::WriteRenderBufferData(const unboxing_engine::CMeshBuffer &meshBuffer) {
    return mImpl->WriteRenderBufferData(meshBuffer);
}

void COpenGLRenderSystem::EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) {
    mImpl->EraseRenderBufferData(renderBufferHandle);
}

void COpenGLRenderSystem::OnPreRender() {
    mImpl->OnPreRender();
}

void COpenGLRenderSystem::OnPostRender() {
    mImpl->OnPostRender();
}

const Camera &systems::COpenGLRenderSystem::GetCamera() const {
    return mImpl->GetCamera();
}

void COpenGLRenderSystem::SetCamera(const Camera &camera) {
    mImpl->SetCamera(camera);
}

const SShaderHandle *COpenGLRenderSystem::GetDefaultShader() const {
    return mImpl->GetDefaultShader();
}

const SShaderHandle *COpenGLRenderSystem::GetTexturePresentationShader() const {
    return mImpl->GetTexturePresentationShader();
}

const IRenderDebug &COpenGLRenderSystem::GetRenderDebug() const {
    return *this;
}

std::optional<STextureSnapshot> COpenGLRenderSystem::CaptureTexture(
    const STextureHandle &texture,
    const STextureInspectionOptions &options) const {
    return mImpl->CaptureTexture(texture, options);
}

std::optional<STextureStatistics> COpenGLRenderSystem::InspectTexture(
    const STextureHandle &texture,
    const STextureInspectionOptions &options) const {
    const auto snapshot = CaptureTexture(texture, options);
    return snapshot ? std::optional<STextureStatistics>(snapshot->statistics) : std::nullopt;
}

bool COpenGLRenderSystem::PrintTextureStatistics(
    const STextureHandle &texture,
    const std::string &label,
    const STextureInspectionOptions &options) const {
    const auto statistics = InspectTexture(texture, options);
    if (!statistics) {
        std::cout << "Texture inspection failed";
        if (!label.empty()) {
            std::cout << " for '" << label << "'";
        }
        std::cout << ".\n";
        return false;
    }

    std::cout << FormatTextureStatistics(*statistics, label);
    return true;
}

void COpenGLRenderSystem::Render(const SRenderContextHandle &renderContextHandle) {
    mImpl->Render(renderContextHandle);
}

STextureHandle *COpenGLRenderSystem::CreateTexture(uint32_t width, uint32_t height, ETextureFormat format) {
    return mImpl->CreateTexture(width, height, format);
}

void COpenGLRenderSystem::EraseTextureData(const STextureHandle &textureHandle) {
    mImpl->EraseTextureData(textureHandle);
}

SRenderTarget *COpenGLRenderSystem::CreateTextureRenderTarget(STextureHandle *textureHandle, const ERenderTargetKind renderTargetKind) {
    return mImpl->CreateTextureRenderTarget(textureHandle, renderTargetKind);
}

void COpenGLRenderSystem::SetRenderTargetClearEnabled(SRenderTarget &renderTarget, const bool enabled) {
    mImpl->SetRenderTargetClearEnabled(renderTarget, enabled);
}

void COpenGLRenderSystem::EraseRenderTargetData(const SRenderTarget &renderTarget) {
    mImpl->EraseRenderTargetData(renderTarget);
}

}// namespace unboxing_engine::systems
