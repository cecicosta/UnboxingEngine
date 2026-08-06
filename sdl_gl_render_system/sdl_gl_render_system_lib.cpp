#include "sdl_gl_render_system_lib.h"

#include "Camera.h"
#include "MeshBuffer.h"
#include "SceneComposite.h"
#include "SceneCompositeLibrary/BasicSceneCompositeTypes.h"
#include "internal_components/IRenderComponent.h"

#include <GL/glew.h>
#include <SDL.h>

#include <iostream>
#include <memory>

static const char *vertex_shader_source =
    "#version 150 core\n"
    "in vec3 i_position;\n"
    "uniform vec4 color;\n"
    "out vec4 v_color;\n"
    "uniform mat4 u_projection_matrix;\n"
    "void main() {\n"
    "    v_color = color;\n"
    "    gl_Position = u_projection_matrix * vec4( i_position.x, i_position.y, i_position.z, 1.0 );\n"
    "}\n";

static const char *fragment_shader_source =
    "#version 150\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}\n";

static const char *signed_texture_debug_vertex_shader_source =
    "#version 150 core\n"
    "in vec2 i_position;\n"
    "out vec2 v_uv;\n"
    "void main() {\n"
    "    v_uv = i_position * 0.5 + 0.5;\n"
    "    gl_Position = vec4(i_position, 0.0, 1.0);\n"
    "}\n";

static const char *signed_texture_debug_fragment_shader_source =
    "#version 150 core\n"
    "uniform sampler2D u_signed_texture;\n"
    "uniform float u_visualization_scale;\n"
    "in vec2 v_uv;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    float signedValue = texture(u_signed_texture, v_uv).a * u_visualization_scale;\n"
    "    vec3 negativeColor = vec3(clamp(-signedValue, 0.0, 1.0), 0.0, 0.0);\n"
    "    vec3 positiveColor = vec3(0.0, clamp(signedValue, 0.0, 1.0), 0.0);\n"
    "    o_color = vec4(negativeColor + positiveColor, 1.0);\n"
    "}\n";

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
    //glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);

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
};


class COpenGLRenderSystem::Impl {
public:
    Impl(const Camera& camera) : mCamera(camera) {}
    ~Impl() {
        ReleaseFloatingPointRenderTarget();
        ReleaseScreenQuad();
        mRenderBuffers.clear();
        mShaders.clear();
       
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
        CreateScreenQuad();
        CreateView(mCamera.mWidth, mCamera.mHeight);

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
        glDeleteVertexArrays(1, &renderBufferHandle.vao);
        glDeleteBuffers(1, &renderBufferHandle.vbo);
        glDeleteBuffers(1, &renderBufferHandle.ebo);

    }
    [[nodiscard]] const Camera& GetCamera() const {
        return mCamera;
    }
    [[nodiscard]] const SShaderHandle* GetDefaultShader() const {
        return mShaders.begin() != mShaders.end() ? mShaders.begin()->get() : nullptr;
    }
    void SetCamera(const Camera& camera) {
        mCamera = camera;
    }
    void Render(const SRenderContextHandle& renderContextHandle) {
        if (renderContextHandle.renderTargetKind == ERenderTargetKind::FloatingPointAccumulation) {
            RenderToFloatingPointTarget(renderContextHandle);
            return;
        }
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

        glPolygonMode(GL_FRONT_AND_BACK, polygonMode == EPolygonMode::Fill ? GL_FILL : GL_LINE);
        const GLint projectionUniform = glGetUniformLocation(program, "u_projection_matrix");
        if (projectionUniform >= 0) {
            glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, (mCamera.GetProjectionMatrix() * mCamera.GetViewMatrix() * renderContextHandle.sceneComposite.GetWorldTransformation()).ToArray());
        }
        const GLint colorUniform = glGetUniformLocation(program, "color");
        if (colorUniform >= 0) {
            glUniform4fv(colorUniform, 1, material->materialDif);
        }
        glBindVertexArray(renderContextHandle.renderBufferHandle->vao);
        glDrawElements(GL_TRIANGLES, renderContextHandle.renderBufferHandle->ntriangles * 3, GL_UNSIGNED_INT, nullptr);

    }

    void OnPreRender() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, static_cast<GLint>(mCamera.mWidth), static_cast<GLint>(mCamera.mHeight));
        glClearColor(0.02f, 0.025f, 0.035f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }
    void OnPostRender() {
        glDepthMask(GL_TRUE);
        SDL_GL_SwapWindow(mWindow);
        SDL_Delay(1);
    }

    bool EnsureFloatingPointRenderTarget() {
        if (mFloatingPointRenderTarget.framebuffer != 0 &&
            mFloatingPointRenderTarget.width == mCamera.mWidth &&
            mFloatingPointRenderTarget.height == mCamera.mHeight) {
            return true;
        }

        ReleaseFloatingPointRenderTarget();

        mFloatingPointRenderTarget.width = mCamera.mWidth;
        mFloatingPointRenderTarget.height = mCamera.mHeight;

        glGenFramebuffers(1, &mFloatingPointRenderTarget.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFloatingPointRenderTarget.framebuffer);

        glGenTextures(1, &mFloatingPointRenderTarget.texture);
        glBindTexture(GL_TEXTURE_2D, mFloatingPointRenderTarget.texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA32F,
                     static_cast<GLsizei>(mFloatingPointRenderTarget.width),
                     static_cast<GLsizei>(mFloatingPointRenderTarget.height),
                     0,
                     GL_RGBA,
                     GL_FLOAT,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFloatingPointRenderTarget.texture, 0);
        const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);

        const GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Floating-point framebuffer initialization failed: " << framebufferStatus << std::endl;
            ReleaseFloatingPointRenderTarget();
            return false;
        }

        return true;
    }

    void RenderToFloatingPointTarget(const SRenderContextHandle& renderContextHandle) {
        if (!EnsureFloatingPointRenderTarget()) {
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, mFloatingPointRenderTarget.framebuffer);
        glViewport(0, 0, static_cast<GLint>(mFloatingPointRenderTarget.width), static_cast<GLint>(mFloatingPointRenderTarget.height));
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        DrawRenderContext(renderContextHandle);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, static_cast<GLint>(mCamera.mWidth), static_cast<GLint>(mCamera.mHeight));
        RenderSignedTextureDebugView();

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void CreateScreenQuad() {
        if (mScreenQuadVao != 0) {
            return;
        }

        const float vertices[] = {
                -1.0f, -1.0f,
                1.0f, -1.0f,
                -1.0f, 1.0f,
                1.0f, 1.0f};

        glGenVertexArrays(1, &mScreenQuadVao);
        glGenBuffers(1, &mScreenQuadVbo);

        glBindVertexArray(mScreenQuadVao);
        glBindBuffer(GL_ARRAY_BUFFER, mScreenQuadVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(attrib_position);
        glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void RenderSignedTextureDebugView() const {
        if (!mSignedTextureDebugShader || mFloatingPointRenderTarget.texture == 0 || mScreenQuadVao == 0) {
            return;
        }

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        const auto program = mSignedTextureDebugShader->program;
        glUseProgram(program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mFloatingPointRenderTarget.texture);
        glUniform1i(glGetUniformLocation(program, "u_signed_texture"), 0);
        glUniform1f(glGetUniformLocation(program, "u_visualization_scale"), 1.0f);

        glBindVertexArray(mScreenQuadVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void ReleaseFloatingPointRenderTarget() {
        if (mFloatingPointRenderTarget.texture != 0) {
            glDeleteTextures(1, &mFloatingPointRenderTarget.texture);
        }
        if (mFloatingPointRenderTarget.framebuffer != 0) {
            glDeleteFramebuffers(1, &mFloatingPointRenderTarget.framebuffer);
        }
        mFloatingPointRenderTarget = {};
    }

    void ReleaseScreenQuad() {
        if (mScreenQuadVbo != 0) {
            glDeleteBuffers(1, &mScreenQuadVbo);
        }
        if (mScreenQuadVao != 0) {
            glDeleteVertexArrays(1, &mScreenQuadVao);
        }
        mScreenQuadVbo = 0;
        mScreenQuadVao = 0;
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
            glType = GL_UNSIGNED_INT;
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
        STextureHandle textureHandle{width, height, textureId, format};
        mTextures.emplace(textureId, std::make_unique<STextureHandle>(textureHandle));
        return mTextures.at(textureId).get();
    }

    void RenderToTexture(const SRenderContextHandle & renderContextHandle, STextureHandle *textureHandle) {

    }

private:
    ///Window and opengl handlers
    SDL_Window *mWindow = nullptr;
    SDL_GLContext mGLContext;
    Camera mCamera;
    SShaderHandle *mSignedTextureDebugShader = nullptr;
    SRenderTarget mFloatingPointRenderTarget;
    std::uint32_t mScreenQuadVao = 0;
    std::uint32_t mScreenQuadVbo = 0;
    std::vector<std::unique_ptr<SShaderHandle>> mShaders;
    std::vector<std::unique_ptr<SRenderBufferHandle>> mRenderBuffers;
    std::unordered_map<uint32_t, std::unique_ptr<STextureHandle>> mTextures;
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

void COpenGLRenderSystem::Render(const SRenderContextHandle &renderContextHandle) {
    mImpl->Render(renderContextHandle);
}

STextureHandle *COpenGLRenderSystem::CreateTexture(uint32_t width, uint32_t height, ETextureFormat format) {
    return mImpl->CreateTexture(width, height, format);
}

void COpenGLRenderSystem::RenderToTexture(const SRenderContextHandle &renderContextHandle, STextureHandle *textureHandle) {
    mImpl->RenderToTexture(renderContextHandle, textureHandle);
}

}// namespace unboxing_engine::systems
