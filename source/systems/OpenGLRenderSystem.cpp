#include "systems/OpenGLRenderSystem.h"

#include "MeshBuffer.h"
#include "Camera.h"

#include <GL/glew.h>
#include <SDL.h>

#include <memory>


namespace unboxing_engine::systems {

namespace {
void GetError() {
    static int error_count = 0;
    auto gl_error = glGetError();
    std::cout << "Get error count: " << error_count++ << std::endl;
    if (gl_error != 0) {
        std::cout << "Code: " << gl_error << std::endl;
    }

    void CreateView() const {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);// Clear The Background Color Out Blue
                                             //    glClearDepth(1.0);                   // Enables Clearing Of The Depth Buffer
                                             //    glEnable(GL_BLEND);
                                             //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Enable Alpha Blending
        //glEnable(GL_CULL_FACE);

        //    glEnable(GL_DEPTH_TEST);// Enables Depth Testing
        //    glDepthFunc(GL_LEQUAL); // Type Of Depth Testing

        glViewport(0, 0, static_cast<GLint>(camera->mWidth), static_cast<GLint>(camera->mHeight));
    }
}
}// namespace

struct SShaderHandle {
    ///Basic shader handler
    std::uint32_t program = -1;
};
struct SRenderBufferHandle {
    std::uint32_t vao = -1;//Refers to the whole render context, including geometry, shaders and parameters
    std::uint32_t vbo = -1;//Buffer handler for geometry
    std::uint32_t ebo = -1;//Buffer handler for geometry vertex indices
    std::uint32_t ntriangles; //Number of triangles the geometry has
};

COpenGLRenderSystem::COpenGLRenderSystem() {
}

COpenGLRenderSystem::~COpenGLRenderSystem() {
    SDL_GL_DeleteContext(mGLContext);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}


bool COpenGLRenderSystem::Initialize(std::uint32_t width, std::uint32_t heigth) {
    //Initialize SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Video initialization failed: " << SDL_GetError() << std::endl;
        return;
    }

    //Initialize SDL_ttf
    TTF_Init();

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
                               static_cast<GLint>(camera->mWidth),
                               static_cast<GLint>(camera->mHeight),
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    // Create an OpenGL context associated with the window
    mGLContext = SDL_GL_CreateContext(mWindow);

    //TODO: Verify if by creating a new window, a new context needs to be
    // created and glew needs to be initialize for that context
    auto init_res = glewInit();
    if (init_res != GLEW_OK) {
        std::cout << glewGetErrorString(glewInit()) << std::endl;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    CreateView();

    return false;
}

std::unique_ptr<SShaderHandle> COpenGLRenderSystem::CompileShader(const char *vertexShaderSrc, const char *fragmentShaderSrc) {
    unsigned int vertexShader;
    {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        int length = static_cast<int>(strlen(vertexhaderSrc));
        glShaderSource(vertexShader, 1, &vertexhaderSrc, &length);
        glCompileShader(vertexShader);

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cout << "Vertex shader compilation failed: \n"
                      << infoLog << std::endl;
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
        }
    }

    glUseProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    auto handle = std::make_unique<SShaderHandle>();
    handle->program = program;
    return handle;
}

std::unique_ptr<SRenderBufferHandle> COpenGLRenderSystem::WriteRenderBufferData(const unboxing_engine::CMeshBuffer &meshBuffer) {
    suto handle = std::make_unique<SRenderBufferHandle>();
    glGenVertexArrays(1, &handle.vao);
    glGenBuffers(1, &handle.vbo);
    glGenBuffers(1, &handle.ebo);

    glBindVertexArray(data.vao);

    glBindBuffer(GL_ARRAY_BUFFER, handle.vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * meshBuffer.nvertices * sizeof(float), meshBuffer.vertices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, 0, (void *) (0));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * meshBuffer.nfaces * sizeof(unsigned int), meshBuffer.triangles.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return handle;
}

void COpenGLRenderSystem::EraseRenderBufferData(const SRenderBufferHandle &renderBufferHandle) {
    glDeleteVertexArrays(1, &context.vao);
    glDeleteBuffers(1, &context.vbo);
    glDeleteBuffers(1, &context.ebo);
}

void COpenGLRenderSystem::OnPreRender() {
    glClear(GL_COLOR_BUFFER_BIT);
}

void COnpenGLRenderSystem::OnPostRender() {
    SDL_GL_SwapWindow(mWindow);
    SDL_Delay(1);
}

void COpenGLRenderSystem::Render(const SRenderContextHandle &renderContextHandle) {
    auto program = renderContextHandle.shaderHandle->program;
    glUseProgram(program);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix"), 1, GL_FALSE, (camera->mTransformation * data.mSceneComposite->GetTransformation()).ToArray());
    glUniform4fv(glGetUniformLocation(program, "color"), 1, render->GetMaterial().materialDif);
    glBindVertexArray(renderContextHandle.renderBufferHandle->vao);
    glDrawElements(GL_TRIANGLES, meshBuffer.nfaces * 3, GL_UNSIGNED_INT, nullptr);
}

void COpenGLRenderSystemRender(const SRenderContextHandle &renderContextHandle) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix"), 1, GL_FALSE, (camera->mTransformation * renderContextHandle.transformation.ToArray());
    glUniform4fv(glGetUniformLocation(program, "color"), 1,renderContextHandle.material.materialDif);
    glBindVertexArray(renderContextHandle.renderBufferHandle->vao);
    glDrawElements(GL_TRIANGLES, renderContextHandle.ntriangles * 3, GL_UNSIGNED_INT, nullptr);
}
}// namespace unboxing_engine::systems
