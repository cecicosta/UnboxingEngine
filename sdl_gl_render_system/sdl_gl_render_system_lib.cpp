#include "sdl_gl_render_system_lib.h"

#include "Camera.h"
#include "MeshBuffer.h"
#include "SceneComposite.h"
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
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);// Clear The Background Color Out Blue
                                         //    glClearDepth(1.0);                   // Enables Clearing Of The Depth Buffer
                                         //    glEnable(GL_BLEND);
                                         //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Enable Alpha Blending
    //glEnable(GL_CULL_FACE);

    //    glEnable(GL_DEPTH_TEST);// Enables Depth Testing
    //    glDepthFunc(GL_LEQUAL); // Type Of Depth Testing

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
    std::uint32_t ntriangles;//Number of triangles the geometry has
};


class COpenGLRenderSystem::Impl {
public:
    Impl(const Camera& camera) : mCamera(camera) {}
    ~Impl() {
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

        mShaders.emplace_back(CompileShader(vertex_shader_source, fragment_shader_source));
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
        glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, 0, (void *) (0));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * meshBuffer.nfaces * sizeof(unsigned int), meshBuffer.triangles.data(), GL_DYNAMIC_DRAW);

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
    void Render(const SRenderContextHandle& renderContextHandle) const {
        auto program = renderContextHandle.shaderHandle->program;
        glUseProgram(program);

        const SMaterial *material;
        if(auto renderComponent = renderContextHandle.sceneComposite.GetComponent<IRenderComponent>()) {
            material = &renderComponent->GetMaterial();
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix"), 1, GL_FALSE, (mCamera.mTransformation * renderContextHandle.sceneComposite.GetTransformation()).ToArray());
        glUniform4fv(glGetUniformLocation(program, "color"), 1, material->materialDif);
        glBindVertexArray(renderContextHandle.renderBufferHandle->vao);
        glDrawElements(GL_TRIANGLES, renderContextHandle.renderBufferHandle->ntriangles * 3, GL_UNSIGNED_INT, nullptr);

    }

    void OnPreRender() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void OnPostRender() {
        SDL_GL_SwapWindow(mWindow);
        SDL_Delay(1);
    }

private:
    ///Window and opengl handlers
    SDL_Window *mWindow = nullptr;
    SDL_GLContext mGLContext;
    Camera mCamera;
    std::vector<std::unique_ptr<SShaderHandle>> mShaders;
    std::vector<std::unique_ptr<SRenderBufferHandle>> mRenderBuffers;
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

}// namespace unboxing_engine::systems
