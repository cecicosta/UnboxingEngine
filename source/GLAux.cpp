#include "GLAux.h"
#include "Camera.h"

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <cstdio>
#include <iostream>


static const char *vertex_shader_source =
        "#version 150 core\n"
        "in vec3 i_position;\n"
        "out vec4 v_color;\n"
        "uniform mat4 u_projection_matrix;\n"
        "void main() {\n"
        "    v_color = vec4(0.5 + i_position.x, 0.5 + i_position.y, 0.5 + i_position.z, 1.0);\n"
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
    attrib_position,
    attrib_color
} t_attrib_id;

GLAux::GLAux(uint32_t width, uint32_t height, uint32_t bpp)
    : camera(std::make_unique<Camera>(width, height, 70.0f, 1, 1)), BPP(bpp) {}

void GLAux::CreateWindow(bool isOrthogonal = true) {
    //Initialize SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Video initialization failed: " << SDL_GetError() << std::endl;
        return;
    }

    //Inicializa a ttf
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

    //Inicializa a mWindow.
    mWindow = SDL_CreateWindow("My Game Window",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               camera->mWidth, camera->mHeight,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    {
        auto gl_error = glGetError();
        std::cout << "Error1: " << gl_error << std::endl;
        std::cout << "Error: " << gluErrorString(gl_error) << std::endl;
    }

    // Create an OpenGL context associated with the window
    mGLContext = SDL_GL_CreateContext(mWindow);

    {
        auto gl_error = glGetError();
        std::cout << "Error2: " << gl_error << std::endl;
        std::cout << "Error: " << gluErrorString(gl_error) << std::endl;
    }

    auto init_res = glewInit();
    if (init_res != GLEW_OK) {
        std::cout << glewGetErrorString(glewInit()) << std::endl;
    }

    {
        auto gl_error = glGetError();
        std::cout << "Error3: " << gl_error << std::endl;
        std::cout << "Error: " << gluErrorString(gl_error) << std::endl;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    //Create view port and frustum
    OrthogonalView(isOrthogonal);

    CreateBasicShader();
}

void GLAux::OrthogonalView(bool isOrthogonal) {

    if (isOrthogonal) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// Clear The Background Color To Blue
                                             //        glClearDepth(1.0);                   // Enables Clearing Of The Depth Buffer
                                             //        glEnable(GL_BLEND);
                                             //        // TODO: Check the need and advantage of having blend and alpha enabled together
                                             //        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                                             //        glAlphaFunc(GL_GREATER, 0.1f);// Set Alpha Testing
                                             //        glEnable(GL_ALPHA_TEST);
                                             //        glEnable(GL_TEXTURE_2D);
                                             //
                                             //        glDepthFunc(GL_LEQUAL);// Type Of Depth Testing
                                             //        glEnable(GL_DEPTH_TEST);

        glViewport(0, 0, camera->mWidth, camera->mHeight);
        //        glMatrixMode(GL_PROJECTION);
        //        glLoadIdentity();

        {
            auto gl_error = glGetError();
            std::cout << "Error13: " << gl_error << std::endl;
            std::cout << "Error: " << gluErrorString(gl_error) << std::endl;
        }
        //glOrtho(camera->mLeft, camera->mRight, camera->mBottom, camera->mTop, camera->mZNear, camera->mZFar);
        //        glMatrixMode(GL_MODELVIEW);
        //        glLoadIdentity();
        {
            auto gl_error = glGetError();
            std::cout << "Error14: " << gl_error << std::endl;
            std::cout << "Error: " << gluErrorString(gl_error) << std::endl;
        }

    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Enable Alpha Blending
        glEnable(GL_BLEND);
        glAlphaFunc(GL_GREATER, 0.1f);// Set Alpha Testing     (disable blending)
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_TEXTURE_2D);// Enable Texture Mapping
        //glEnable(GL_CULL_FACE);

        glDepthFunc(GL_LEQUAL); // Type Of Depth Testing
        glEnable(GL_DEPTH_TEST);// Enables Depth Testing

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glFrustum(camera->mLeft, camera->mRight, camera->mBottom, camera->mTop, camera->mZNear, camera->mZFar);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
}

void GLAux::draw2DObject(plane2D area, Texture *texture, float cor[4], float rect[4]) {

    glBindTexture(GL_TEXTURE_2D, texture->texID);

    glTranslatef(area.x, area.y, area.z);
    glColor4f(cor[0], cor[1], cor[2], cor[3]);
    glBegin(GL_QUADS);// Start Drawing A Quad
    glTexCoord2f(rect[0], rect[1]);
    glVertex3f(-area.w / 2, -area.h / 2, 0);// Bottom Left
    glTexCoord2f(rect[2], rect[1]);
    glVertex3f(area.w / 2, -area.h / 2, 0);// Bottom Right
    glTexCoord2f(rect[2], rect[3]);
    glVertex3f(area.w / 2, area.h / 2, 0);// Top Right
    glTexCoord2f(rect[0], rect[3]);
    glVertex3f(-area.w / 2, area.h / 2, 0);// Top Left
    glEnd();
    glLoadIdentity();
}

void GLAux::FPSCamera() {

    float x = 0;
    float y = 0;
    float z = 0;
    float rot_y = 0;
    float rot_x = 0;
    float cam_vel = 1;

    if (keyState) {
        switch (*keyState) {
            case SDLK_RIGHT:
                x = x - cam_vel;
            case SDLK_LEFT:
                x = x + cam_vel;
            case SDLK_UP:
                y = y - cam_vel;
            case SDLK_DOWN:
                y = y + cam_vel;
            case SDLK_o:
                z = z + cam_vel;
            case SDLK_i:
                z = z - cam_vel;
            case SDLK_a:
                rot_y = rot_y + cam_vel;
            case SDLK_d:
                rot_y = rot_y - cam_vel;
            case SDLK_w:
                rot_x = rot_x + cam_vel;
            case SDLK_s:
                rot_x = rot_x - cam_vel;
        }
    }

    double angX = mouseTracking_x * 180 / 400 - vel_x;
    double angY = mouseTracking_y * 180 / 300 - vel_y;
    vel_x = mouseTracking_x * 180 / 400;
    vel_y = mouseTracking_y * 180 / 300;

    //Obter o eixo x atual da câmera
    vector3D x_axis = camera->mDirection.vetorial(camera->mUp).normalizado();

    //Atualizar as proriedades da camera
    camera->mDirection = Matrix::rotationMatrix(angY, x_axis) * camera->mDirection;
    camera->mUp = Matrix::rotationMatrix(angY, x_axis) * camera->mUp;
    //Atualizar as proriedades da camera
    camera->mDirection = Matrix::rotationMatrix(angX, vector3D(0, 1, 0)) * camera->mDirection;
    camera->mUp = Matrix::rotationMatrix(angX, vector3D(0, 1, 0)) * camera->mUp;

    //Atualiza a matrix de transformação da câmera
    camera->mWorldToCamTransformation = Matrix::translationMatrix(camera->mPosition * -1) * camera->mWorldToCamTransformation;
    camera->mWorldToCamTransformation = Matrix::rotationMatrix(angY, x_axis) * camera->mWorldToCamTransformation;
    camera->mWorldToCamTransformation = Matrix::rotationMatrix(angX, vector3D(0, 1, 0)) * camera->mWorldToCamTransformation;

    //Transçada a camera->em seu sistema local
    camera->mPosition = camera->mPosition - x_axis.normalizado() * x - camera->mUp.normalizado() * y - camera->mDirection.normalizado() * z;

    camera->mWorldToCamTransformation = Matrix::translationMatrix(camera->mPosition) * camera->mWorldToCamTransformation;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(camera->mLeft, camera->mRight, camera->mBottom, camera->mTop, camera->mZNear, camera->mZFar);
    glMultMatrixf(camera->mWorldToCamTransformation.inverse().getMatrixGL());
    glMatrixMode(GL_MODELVIEW);
}

void GLAux::OnInput() {
    keyState = SDL_GetKeyboardState(nullptr);

    mouse_wheel = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                if (mousestate[L_BUTTON] == 1) {
                    mouseTracking_x = event.motion.x - mouse_click_x;
                    mouseTracking_y = event.motion.y - mouse_click_y;
                }
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;

                break;
            case SDL_MOUSEBUTTONDOWN:

                if (GLAux::event.button.button == SDL_BUTTON_LEFT && mouse_buttonPressed == false) {
                    mousestate[L_BUTTON] = 1;
                    mouse_click_x = event.motion.x;
                    mouse_click_y = event.motion.y;
                }
                if (GLAux::event.button.button == SDL_BUTTON_RIGHT && mouse_buttonPressed == false) {
                    mousestate[R_BUTTON] = 1;
                    mouse_click_x = event.motion.x;
                    mouse_click_y = event.motion.y;
                }
                mouse_buttonPressed = true;

                if (GLAux::event.type == SDL_MOUSEWHEEL) {
                    mouse_wheel = GLAux::event.wheel.y;
                }

                break;
            case SDL_MOUSEBUTTONUP:
                mouseTracking_x = 0;
                mouseTracking_y = 0;
                vel_x = 0;
                vel_y = 0;
                mouse_buttonPressed = false;

                if (GLAux::event.button.button == SDL_BUTTON_LEFT && mousestate[L_BUTTON] == 1) {
                    mousestate[L_BUTTON] = 0;
                }
                if (GLAux::event.button.button == SDL_BUTTON_RIGHT && mousestate[R_BUTTON] == 1) {
                    mousestate[R_BUTTON] = 0;
                }

                break;
            case SDL_QUIT:
                quit = true;
        }
    }

    if (keyState[SDLK_ESCAPE])
        quit = true;
}


Texture *GLAux::LoadTexture(char *filename) {

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
        return NULL;
    }

    return texture;
}

Texture *GLAux::CreateTextureFromSurface(SDL_Surface *sdlimage) {

    GLuint retval;
    void *raw;
    int w, h, i, j, bpp;
    Uint8 *srcPixel, *dstPixel;
    Uint32 truePixel;
    GLenum errorCode;

    if (!sdlimage) {
        printf("SDL_Image load error: %s\n", IMG_GetError());
        return NULL;
    }

    w = sdlimage->w;
    h = sdlimage->h;

    raw = (void *) malloc(w * h * 4);
    dstPixel = (Uint8 *) raw;

    SDL_LockSurface(sdlimage);

    bpp = sdlimage->format->BytesPerPixel;

    for (i = h - 1; i >= 0; i--) {
        for (j = 0; j < w; j++) {
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
                    return NULL;
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
        return NULL;
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

void GLAux::ApplySurface(int x, int y, SDL_Surface *origem, SDL_Surface *destino, SDL_Rect *clip = NULL) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    //Aplica a Imagem na mWindow.
    SDL_BlitSurface(origem, clip, destino, &offset);
}

SDL_Surface *GLAux::LoadSurface(char *nome) {
    auto *newImage = IMG_Load(nome);
    auto *newImageOptimized = SDL_ConvertSurface(newImage, newImage->format, 0);

    //Define a color key
    Uint32 colorkey = SDL_MapRGB(newImageOptimized->format, 0, 0xFF, 0xFF);
    SDL_SetColorKey(newImageOptimized, SDL_TRUE, colorkey);

    SDL_FreeSurface(newImage);
    return newImageOptimized;
}

SDL_Surface *GLAux::LoadSurfaceAlpha(char *nome) {
    auto *newImage = IMG_Load(nome);
    auto *newImageOptimized = SDL_ConvertSurfaceFormat(newImage, SDL_PIXELFORMAT_RGBA8888, 0);

    SDL_FreeSurface(newImage);

    return newImageOptimized;
}

//Função que estabelece o intervalo entre os quadros.
void GLAux::Timer(int tempo) {
    auto cont = SDL_GetTicks();
    //TODO: Change the way the FPS is controlled so the process is not interrupted
    while (SDL_GetTicks() - cont < tempo)
        ;
    //frame_cont++;
}

SDL_Rect *GLAux::CreateSurfaceClips(int linhas, int colunas, int intervalo_x, int intervalo_y) {
    int i, j, cont = 0;
    SDL_Rect *clip = nullptr;
    clip = (SDL_Rect *) malloc((linhas * colunas) * sizeof(SDL_Rect));
    for (i = 0; i < linhas; i++)
        for (j = 0; j < colunas; j++) {
            clip[cont].x = j * intervalo_x;
            clip[cont].y = i * intervalo_y;
            clip[cont].w = intervalo_x;
            clip[cont++].h = intervalo_y;
        }

    return clip;
}

void GLAux::Release() {
    SDL_GL_DeleteContext(mGLContext);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void GLAux::CreateBasicShader() {

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
    glBindAttribLocation(program, attrib_color, "i_color");

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

void GLAux::CreateArray(const float *vertices, size_t vSize, const unsigned int *indices, size_t iSize) {

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vSize * sizeof(float), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize * sizeof(int), indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 3, GL_FLOAT, GL_FALSE, (0)* sizeof(float), (void *) (0 * sizeof(float)));

}
void GLAux::RenderCanvas()  {
    SDL_GL_SwapWindow(mWindow);
    SDL_Delay(1);
}
