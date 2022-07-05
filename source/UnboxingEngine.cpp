#include "UnboxingEngine.h"

#include <Camera.h>
#include <CoreEvents.h>
#include <IComposite.h>
#include <MeshBuffer.h>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <cstdio>
#include <iostream>

namespace unboxing_engine {

    static const char *vertex_shader_source =
            "#version 150 core\n"
            "in vec3 i_position;\n"
            "out vec4 v_color;\n"
            "uniform mat4 u_projection_matrix;\n"
            "void main() {\n"
            "    v_color = vec4(0.5 + i_position.y, 0.5 + i_position.y, 0.5 + i_position.z, 1.0);\n"
            "    gl_Position = u_projection_matrix * vec4( i_position.x, i_position.y, i_position.z, 1.0 );\n"
            "}\n";

    static const char *fragment_shader_source =
            "#version 130\n"
            "in vec4 v_color;\n"
            "out vec4 o_color;\n"
            "void main() {\n"
            "    o_color = v_color;\n"
            "}\n";

    typedef enum t_attrib_id {
        attrib_position
    } t_attrib_id;

    CCore::CCore(uint32_t width, uint32_t height, uint32_t bpp)
        : camera(std::make_unique<Camera>(width, height, 70.0f, 1.f, 1.f)), BPP(bpp) {}

    void CCore::Start() {
        CreateWindow();
        for (auto l: GetListeners<core_events::IStartListener>()) {
            l->OnStart();
        }
    }
    void CCore::Run() {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        while (!HasQuit()) {
            glClear(GL_COLOR_BUFFER_BIT);

            OnInput();

            WritePendingRenderData();

            Render();
            for (auto &&listener: GetListeners<core_events::IUpdateListener>()) {
                listener->OnUpdate();
            }
        }
    }
    void CCore::Render() {
        glUseProgram(program);

        for (auto &&listener: GetListeners<core_events::IPreRenderListener>()) {
            listener->OnPreRender();
        }

        camera->mTransformation = Matrix4f::RotationMatrix(1, Vector3f(1, 0, 0)) * camera->mTransformation;
        glUniformMatrix4fv(glGetUniformLocation(program, "u_projection_matrix"), 1, GL_FALSE, camera->mTransformation.ToArray());
        for (auto &&data: mRenderQueue) {
            glBindVertexArray(data.vao);
            glDrawElements(GL_TRIANGLES, data.mMeshBuffer->nfaces * 3, GL_UNSIGNED_INT, nullptr);
        }
        RenderCanvas();

        for (auto &&listener: GetListeners<core_events::IPostRenderListener>()) {
            listener->OnPostRender();
        }
    }

    void CCore::CreateWindow() {
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

        CreateBasicShader();
    }

    void CCore::CreateView() const {
        glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// Clear The Background Color Out Blue
                                             //    glClearDepth(1.0);                   // Enables Clearing Of The Depth Buffer
                                             //    glEnable(GL_BLEND);
                                             //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Enable Alpha Blending
        //glEnable(GL_CULL_FACE);

        //    glEnable(GL_DEPTH_TEST);// Enables Depth Testing
        //    glDepthFunc(GL_LEQUAL); // Type Of Depth Testing

        glViewport(0, 0, static_cast<GLint>(camera->mWidth), static_cast<GLint>(camera->mHeight));
    }

    void CCore::OnInput() {
        keyState = SDL_GetKeyboardState(nullptr);

        mCursor.scrolling = 0;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    if (mCursor.cursorState[L_BUTTON] == 1) {
                        mCursor.draggingX = event.motion.x - mCursor.buttonPressedX;
                        mCursor.draggingY = event.motion.y - mCursor.buttonPressedY;
                    }
                    mCursor.x = event.motion.x;
                    mCursor.y = event.motion.y;

                    break;
                case SDL_MOUSEBUTTONDOWN:

                    if (CCore::event.button.button == SDL_BUTTON_LEFT && !mCursor.isButtonPressed) {
                        mCursor.cursorState[L_BUTTON] = 1;
                        mCursor.buttonPressedX = event.motion.x;
                        mCursor.buttonPressedY = event.motion.y;
                    }
                    if (CCore::event.button.button == SDL_BUTTON_RIGHT && !mCursor.isButtonPressed) {
                        mCursor.cursorState[R_BUTTON] = 1;
                        mCursor.buttonPressedX = event.motion.x;
                        mCursor.buttonPressedY = event.motion.y;
                    }
                    mCursor.isButtonPressed = true;

                    if (CCore::event.type == SDL_MOUSEWHEEL) {
                        mCursor.scrolling = CCore::event.wheel.y;
                    }

                    break;
                case SDL_MOUSEBUTTONUP:
                    mCursor.draggingX = 0;
                    mCursor.draggingY = 0;
                    mCursor.draggingSpeedX = 0;
                    mCursor.draggingSpeedY = 0;
                    mCursor.isButtonPressed = false;

                    if (CCore::event.button.button == SDL_BUTTON_LEFT && mCursor.cursorState[L_BUTTON] == 1) {
                        mCursor.cursorState[L_BUTTON] = 0;
                    }
                    if (CCore::event.button.button == SDL_BUTTON_RIGHT && mCursor.cursorState[R_BUTTON] == 1) {
                        mCursor.cursorState[R_BUTTON] = 0;
                    }

                    break;
                case SDL_QUIT:
                    quit = true;
            }
        }

        mCursor.draggingSpeedX = static_cast<float>(mCursor.draggingX) * 180.f / 400.f;
        mCursor.draggingSpeedY = static_cast<float>(mCursor.draggingY) * 180.f / 300.f;

        if (keyState[SDLK_ESCAPE])
            quit = true;

        for (auto &&listener: GetListeners<core_events::IInputListener>()) {
            listener->OnInput();
        }
    }

    void CCore::UpdateFlyingController() {
        Vector3f velocity;
        Vector3f rotation;
        if (keyState) {
            if (keyState[SDLK_RIGHT]) {
                velocity.x = -1;
            }
            if (keyState[SDLK_LEFT]) {
                velocity.x = 1;
            }
            if (keyState[SDLK_UP]) {
                velocity.y = -1;
            }
            if (keyState[SDLK_DOWN]) {
                velocity.y = 1;
            }
            if (keyState[SDLK_o]) {
                velocity.z = 1;
            }
            if (keyState[SDLK_i]) {
                velocity.z = -1;
            }
        }

        rotation.x = mCursor.draggingX * 180 / 400 - mCursor.draggingSpeedX;
        rotation.y = mCursor.draggingY * 180 / 300 - mCursor.draggingSpeedY;

        camera->FPSCamera(velocity, rotation);
    }

    Texture *CCore::LoadTexture(char *filename) {

        GLuint retval;
        SDL_Surface *sdlimage;
        void *raw;
        int w, h, bpp;
        Uint8 *srcPixel, *dstPixel;
        Uint32 truePixel;
        GLenum errorCode;

        //Pode substituir aki a forma de carregar a surface
        sdlimage = LoadSurfaceAlpha(filename);


        if (!sdlimage) {
            FILE *f;
            f = fopen("debug.txt", "w");
            fprintf(f, "SDL_Image load error: %s\n", IMG_GetError());
            fclose(f);
            return nullptr;
        }

        w = sdlimage->w;
        h = sdlimage->h;

        raw = (void *) malloc(w * h * 4);
        dstPixel = (Uint8 *) raw;

        SDL_LockSurface(sdlimage);

        bpp = sdlimage->format->BytesPerPixel;

        for (auto i = h - 1; i >= 0; i--) {
            for (auto j = 0; j < w; j++) {
                srcPixel = (Uint8 *) sdlimage->pixels + i * sdlimage->pitch + j * bpp;
                switch (bpp) {
                    case 1:
                        truePixel = *srcPixel;
                        break;

                    case 2:
                        truePixel = *(Uint16 *) srcPixel;
                        break;

                    case 3:
                        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                            truePixel = srcPixel[0] << 16 | srcPixel[1] << 8 | srcPixel[2];
                        } else {
                            truePixel = srcPixel[0] | srcPixel[1] << 8 | srcPixel[2] << 16;
                        }
                        break;

                    case 4:
                        truePixel = *(Uint32 *) srcPixel;
                        break;

                    default:
                        printf("Image bpp of %d unusable\n", bpp);
                        SDL_UnlockSurface(sdlimage);
                        SDL_FreeSurface(sdlimage);
                        free(raw);
                        return nullptr;
                }

                SDL_GetRGBA(truePixel, sdlimage->format, &(dstPixel[0]), &(dstPixel[1]), &(dstPixel[2]), &(dstPixel[3]));
                dstPixel++;
                dstPixel++;
                dstPixel++;
                dstPixel++;
            }
        }

        SDL_UnlockSurface(sdlimage);
        SDL_FreeSurface(sdlimage);

        while (glGetError()) { ; }

        glGenTextures(1, &retval);
        glBindTexture(GL_TEXTURE_2D, retval);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        errorCode = glGetError();
        if (errorCode != 0) {
            if (errorCode == GL_OUT_OF_MEMORY) {
                printf("Out of texture memory!\n");
            }

            glDeleteTextures(1, &retval);
            free(raw);
            return nullptr;
        }

        //gluBuild2DMipmaps( GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *)raw);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *) raw);
        glGenerateMipmap(retval);
        auto *texture = (Texture *) malloc(sizeof(Texture));

        texture->imageData = (int *) raw;
        texture->bpp = bpp;
        texture->width = w;
        texture->height = h;
        texture->texID = retval;
        texture->type = GL_RGBA;

        errorCode = glGetError();
        if (errorCode != 0) {
            if (errorCode == GL_OUT_OF_MEMORY) {
                printf("Out of texture memory!\n");
            }

            glDeleteTextures(1, &retval);
            free(raw);
            return nullptr;
        }

        return texture;
    }

    Texture *CCore::CreateTextureFromSurface(SDL_Surface *sdlSurface) {

        GLuint retval;
        void *raw;
        int w, h, i, j, bpp;
        Uint8 *srcPixel, *dstPixel;
        Uint32 truePixel;
        GLenum errorCode;

        if (!sdlSurface) {
            printf("SDL_Image load error: %s\n", IMG_GetError());
            return nullptr;
        }

        w = sdlSurface->w;
        h = sdlSurface->h;

        raw = (void *) malloc(w * h * 4);
        dstPixel = (Uint8 *) raw;

        SDL_LockSurface(sdlSurface);

        bpp = sdlSurface->format->BytesPerPixel;

        for (i = h - 1; i >= 0; i--) {
            for (j = 0; j < w; j++) {
                srcPixel = (Uint8 *) sdlSurface->pixels + i * sdlSurface->pitch + j * bpp;
                switch (bpp) {
                    case 1:
                        truePixel = *srcPixel;
                        break;

                    case 2:
                        truePixel = *(Uint16 *) srcPixel;
                        break;

                    case 3:
                        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                            truePixel = srcPixel[0] << 16 | srcPixel[1] << 8 | srcPixel[2];
                        } else {
                            truePixel = srcPixel[0] | srcPixel[1] << 8 | srcPixel[2] << 16;
                        }
                        break;

                    case 4:
                        truePixel = *(Uint32 *) srcPixel;
                        break;

                    default:
                        printf("Image bpp of %d unusable\n", bpp);
                        SDL_UnlockSurface(sdlSurface);
                        SDL_FreeSurface(sdlSurface);
                        free(raw);
                        return nullptr;
                }

                SDL_GetRGBA(truePixel, sdlSurface->format, &(dstPixel[0]), &(dstPixel[1]), &(dstPixel[2]), &(dstPixel[3]));
                dstPixel++;
                dstPixel++;
                dstPixel++;
                dstPixel++;
            }
        }

        SDL_UnlockSurface(sdlSurface);
        SDL_FreeSurface(sdlSurface);

        while (glGetError()) { ; }

        glGenTextures(1, &retval);
        glBindTexture(GL_TEXTURE_2D, retval);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        errorCode = glGetError();
        if (errorCode != 0) {
            if (errorCode == GL_OUT_OF_MEMORY) {
                printf("Out of texture memory!\n");
            }

            glDeleteTextures(1, &retval);
            free(raw);
            return nullptr;
        }

        //gluBuild2DMipmaps( GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *)raw);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (Uint8 *) raw);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);


        Texture *texture = (Texture *) malloc(sizeof(Texture));

        texture->imageData = (int *) raw;
        texture->bpp = bpp;
        texture->width = w;
        texture->height = h;
        texture->texID = retval;
        texture->type = GL_RGBA;

        errorCode = glGetError();
        if (errorCode != 0) {
            if (errorCode == GL_OUT_OF_MEMORY) {
                printf("Out of texture memory!\n");
            }

            glDeleteTextures(1, &retval);
            free(raw);
            return nullptr;
        }

        return texture;
    }

    void CCore::ApplySurface(int x, int y, SDL_Surface *origem, SDL_Surface *destino, SDL_Rect *clip = NULL) {
        SDL_Rect offset;
        offset.x = x;
        offset.y = y;
        //Aplica a Imagem na mWindow.
        SDL_BlitSurface(origem, clip, destino, &offset);
    }

    SDL_Surface *CCore::LoadSurface(char *nome) {
        auto *newImage = IMG_Load(nome);
        auto *newImageOptimized = SDL_ConvertSurface(newImage, newImage->format, 0);

        //Define a color key
        Uint32 colorkey = SDL_MapRGB(newImageOptimized->format, 0, 0xFF, 0xFF);
        SDL_SetColorKey(newImageOptimized, SDL_TRUE, colorkey);

        SDL_FreeSurface(newImage);
        return newImageOptimized;
    }

    SDL_Surface *CCore::LoadSurfaceAlpha(char *nome) {
        auto *newImage = IMG_Load(nome);
        auto *newImageOptimized = SDL_ConvertSurfaceFormat(newImage, SDL_PIXELFORMAT_RGBA8888, 0);

        SDL_FreeSurface(newImage);

        return newImageOptimized;
    }

    SDL_Rect *CCore::CreateSurfaceClips(int linhas, int colunas, int width, int height) {
        int i, j, cont = 0;
        SDL_Rect *clip = nullptr;
        clip = (SDL_Rect *) malloc((linhas * colunas) * sizeof(SDL_Rect));
        for (i = 0; i < linhas; i++)
            for (j = 0; j < colunas; j++) {
                clip[cont].x = j * width;
                clip[cont].y = i * height;
                clip[cont].w = width;
                clip[cont++].h = height;
            }

        return clip;
    }

    void CCore::Release() {
        SDL_GL_DeleteContext(mGLContext);
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
        for (auto &&listener: GetListeners<core_events::IReleaseListener>()) {
            listener->OnRelease();
        }
    }

    void CCore::CreateBasicShader() {
        // Create and compile vertex shader
        unsigned int vertexShader;
        {
            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            int length = static_cast<int>(strlen(vertex_shader_source));
            glShaderSource(vertexShader, 1, &vertex_shader_source, &length);
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
            int length = static_cast<int>(strlen(vertex_shader_source));
            glShaderSource(fragmentShader, 1, &fragment_shader_source, &length);
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
        program = glCreateProgram();
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
    }

    void CCore::WritePendingRenderData() {
        for (auto &&data: mPendingWriteQueue) {
            glGenVertexArrays(1, &data.vao);
            glGenBuffers(1, &data.vbo);
            glGenBuffers(1, &data.ebo);

            glBindVertexArray(data.vao);

            glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
            glBufferData(GL_ARRAY_BUFFER, 3 * data.mMeshBuffer->nvertices * sizeof(float), data.mMeshBuffer->vertices, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(attrib_position);
            glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, 0, (void *) (0));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * data.mMeshBuffer->nfaces * sizeof(unsigned int), data.mMeshBuffer->triangles, GL_DYNAMIC_DRAW);

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            mRenderQueue.emplace_back(data);
        };
        mPendingWriteQueue.clear();
    }
    void CCore::GetError() {
        static int error_count = 0;
        auto gl_error = glGetError();
        std::cout << "Get error count: " << error_count++ << std::endl;
        if (gl_error != 0) {
            std::cout << "Code: " << gl_error << std::endl;
            std::cout << "Message: " << gluErrorString(gl_error) << std::endl;
        }
    }
    void CCore::RenderCanvas() {
        SDL_GL_SwapWindow(mWindow);
        SDL_Delay(1);
    }

    void CCore::RegisterSceneElement(const CMeshBuffer &mesh) {
        mPendingWriteQueue.emplace_back(SRenderContext(mesh));
    }

    void CCore::RegisterEventListener(UListener<> &listener) {
        RegisterListener(listener);
    }

    void CCore::UnregisterEventListener(UListener<> &listener) {
        UnregisterListener(listener);
    }

}//namespace unboxing_engine