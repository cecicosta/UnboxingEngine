#include "GLAux.h"
#include "draw.h"

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>
#include <cstdio>

float GLAux::mouseTracking_x = 0;
float GLAux::mouseTracking_y = 0;
float GLAux::vel_x = 0;
float GLAux::vel_y = 0;
float GLAux::mouse_x = 0;
float GLAux::mouse_y = 0;
float GLAux::mouse_click_x = 0;
float GLAux::mouse_click_y = 0;

int GLAux::WIDTH = 800;
int GLAux::HEIGHT = 600;
int GLAux::BPP = 32;
GLdouble GLAux::xmin, GLAux::xmax, GLAux::ymin,
        GLAux::ymax, GLAux::fovy, GLAux::zNear,
        GLAux::zFar, GLAux::aspect;

SDL_Window *GLAux::screen;
SDL_Event GLAux::event;
Uint8 const *GLAux::keyState;
float GLAux::position[3];
int GLAux::mousestate[3];
bool GLAux::mouse_buttonPressed = false;
int GLAux::mouse_wheel = 0;
bool GLAux::quit = false;

vector3D GLAux::cam_pos = vector3D(0, 0, 0);
vector3D GLAux::cam_dir = vector3D(0, 0, -1);
vector3D GLAux::cam_up = vector3D(0, 1, 0);
quaternion GLAux::q = quaternion(0, vector3D(1, 0, 0));
Matrix GLAux::transfomation = Matrix::identity(4);
Matrix GLAux::worldToCam = Matrix::identity(4);

void GLAux::Init(bool ortho = true) {
    //Inicializa todos os subsistemas da SDL.
    SDL_Init(SDL_INIT_EVERYTHING);
    //Inicializa a ttf
    TTF_Init();
    //Inicializa a screen.
    screen = SDL_CreateWindow("My Game Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              640, 480,
                              SDL_WINDOW_OPENGL);

    //Inicializa o opengl
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    OrthogonalView(ortho, position);
}


void GLAux::OrthogonalView(bool ortho, float pos[3] = position) {

    if (ortho) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.5f);// Clear The Background Color To Blue
        glClearDepth(1.0);                   // Enables Clearing Of The Depth Buffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glAlphaFunc(GL_GREATER, 0.1f);// Set Alpha Testing     (disable blending)
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_TEXTURE_2D);

        glDepthFunc(GL_LEQUAL);// Type Of Depth Testing
        glEnable(GL_DEPTH_TEST);

        glViewport(0, 0, WIDTH, HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-WIDTH / 2, WIDTH / 2, -HEIGHT / 2, HEIGHT / 2, 1000.0, -1000.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// Enable Alpha Blending (disable alpha testing)
        glEnable(GL_BLEND);
        glAlphaFunc(GL_GREATER, 0.1f);// Set Alpha Testing     (disable blending)
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_TEXTURE_2D);// Enable Texture Mapping
        //glEnable(GL_CULL_FACE);

        glDepthFunc(GL_LEQUAL); // Type Of Depth Testing
        glEnable(GL_DEPTH_TEST);// Enables Depth Testing

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();


        fovy = 90.0;
        zNear = 1.0;
        zFar = 1000.0;
        aspect = (GLfloat) WIDTH / (GLfloat) HEIGHT;

        ymax = zNear * tan(fovy * M_PI / 360.0);
        ymin = -ymax;
        xmin = ymin * aspect;
        xmax = ymax * aspect;

        glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
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


void GLAux::drawSphere(vector3D position, float radius) {
    float RAD_90 = 1.570796;
    float RAD_15 = 0.261799;
    glColor3f(1, 0, 0);
    glTranslatef(position.x, position.y, position.z);
    glRotatef(-25, 0, 0, 1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, radius * sin(RAD_90), cos(RAD_90));
    glVertex3f(0, radius * sin(RAD_90 + RAD_15), radius * cos(RAD_90 + RAD_15));
    for (int j = 1; j <= 24; j++)
        glVertex3f(radius * sin(RAD_15 * j) * cos(RAD_90 + RAD_15), radius * sin(RAD_90 + RAD_15), radius * cos(RAD_90 + RAD_15) * cos(RAD_15 * j));
    glEnd();
    glColor3f(0, 1, 0);
    glRotatef(180, 1, 0, 0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, radius * sin(RAD_90), cos(RAD_90));
    glVertex3f(0, radius * sin(RAD_90 + RAD_15), radius * cos(RAD_90 + RAD_15));
    for (int j = 1; j <= 24; j++)
        glVertex3f(radius * sin(RAD_15 * j) * cos(RAD_90 + RAD_15), radius * sin(RAD_90 + RAD_15), radius * cos(RAD_90 + RAD_15) * cos(RAD_15 * j));
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for (int j = 1; j <= 24; j++)
        for (int i = 1; i <= 12; i++) {
            glColor3f(0, 0.5, 0.5);
            glVertex3f(radius * sin(RAD_15 * (j - 1)) * cos(RAD_90 + RAD_15 * i), radius * sin(RAD_90 + RAD_15 * i), radius * cos(RAD_90 + RAD_15 * i) * cos(RAD_15 * (j - 1)));
            glColor3f(0.5, 0.5, 0);
            glVertex3f(radius * sin(RAD_15 * j) * cos(RAD_90 + RAD_15 * i), radius * sin(RAD_90 + RAD_15 * i), radius * cos(RAD_90 + RAD_15 * i) * cos(RAD_15 * j));
        }
    glEnd();
    glColor3f(1, 1, 1);
    glLoadIdentity();
}


void GLAux::rayFromScreenCoord(int scr_x, int scr_y, vector3D &point, vector3D &dir) {
    float fovy = 90.0;
    float zNear = 1.0;
    float zFar = 1.0;
    float aspect = (GLfloat) WIDTH / (GLfloat) HEIGHT;

    float ymax = zNear * tan(fovy * M_PI / 360.0);
    float ymin = -ymax;
    float xmin = ymin * aspect;
    float xmax = ymax * aspect;

    float x, y, z;
    z = -1;
    x = (2 * scr_x * xmax) / WIDTH - xmax;
    y = (2 * scr_y * ymin) / HEIGHT + ymax;

    dir = (worldToCam * vector3D(x, y, z)) - cam_pos;
    point = cam_pos;
}

vector3D GLAux::convert2DTo3DCoord(float X, float Y, float dist) {
    float x, y, z;
    dist = dist + position[2];
    //Converte coordenadas x e y da viewport para coordenadas x e y do plano a distância 1 da camera
    z = (-1);
    x = (xmax * (X - WIDTH / 2)) / (WIDTH / 2);
    y = (ymax * Y - HEIGHT / 2) / (-HEIGHT / 2);

    //Calcula o vetor ray unitario
    vector3D unit;
    unit.x = x / sqrt(x * x + y * y + z * z);
    unit.y = y / sqrt(x * x + y * y + z * z);
    unit.z = z / sqrt(x * x + y * y + z * z);

    float T;
    vector3D ray;

    T = dist / unit.z;
    ray.x = T * unit.x - position[0];
    ray.y = T * unit.y - position[1];
    ray.z = dist;
    return ray;
}


void GLAux::camFPS() {

    float x = 0;
    float y = 0;
    float z = 0;
    float rot_y = 0;
    float rot_x = 0;
    float cam_vel = 1;

    if (GLAux::keyState[SDLK_RIGHT])
        x = x - cam_vel;
    if (GLAux::keyState[SDLK_LEFT])
        x = x + cam_vel;
    if (GLAux::keyState[SDLK_UP])
        y = y - cam_vel;
    if (GLAux::keyState[SDLK_DOWN])
        y = y + cam_vel;
    if (GLAux::keyState[SDLK_o])
        z = z + cam_vel;
    if (GLAux::keyState[SDLK_i])
        z = z - cam_vel;
    if (GLAux::keyState[SDLK_a])
        rot_y = rot_y + cam_vel;
    if (GLAux::keyState[SDLK_d])
        rot_y = rot_y - cam_vel;
    if (GLAux::keyState[SDLK_w])
        rot_x = rot_x + cam_vel;
    if (GLAux::keyState[SDLK_s])
        rot_x = rot_x - cam_vel;

    double angX = mouseTracking_x * 180 / 400 - vel_x;
    double angY = mouseTracking_y * 180 / 300 - vel_y;
    vel_x = mouseTracking_x * 180 / 400;
    vel_y = mouseTracking_y * 180 / 300;

    //Obter o eixo x atual da câmera
    vector3D x_axis = cam_dir.vetorial(cam_up).normalizado();

    //Atualizar as proriedades da camera
    cam_dir = Matrix::rotationMatrix(angY, x_axis) * cam_dir;
    cam_up = Matrix::rotationMatrix(angY, x_axis) * cam_up;
    //Atualizar as proriedades da camera
    cam_dir = Matrix::rotationMatrix(angX, vector3D(0, 1, 0)) * cam_dir;
    cam_up = Matrix::rotationMatrix(angX, vector3D(0, 1, 0)) * cam_up;

    //Atualiza a matrix de transformação da câmera
    worldToCam = Matrix::translationMatrix(cam_pos * -1) * worldToCam;
    worldToCam = Matrix::rotationMatrix(angY, x_axis) * worldToCam;
    worldToCam = Matrix::rotationMatrix(angX, vector3D(0, 1, 0)) * worldToCam;

    //Transçada a camera em seu sistema local
    cam_pos = cam_pos - x_axis.normalizado() * x - cam_up.normalizado() * y - cam_dir.normalizado() * z;

    worldToCam = Matrix::translationMatrix(cam_pos) * worldToCam;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
    glMultMatrixf(worldToCam.inverse().getMatrixGL());
    glMatrixMode(GL_MODELVIEW);
}

void GLAux::setCam(vector3D pos, float ang, vector3D eixo, vector3D point) {
    cam_dir = Matrix::rotationMatrix(ang, eixo) * cam_dir;
    cam_up = Matrix::rotationMatrix(ang, eixo) * cam_up;

    //Atualiza a matrix de transformação da câmera
    worldToCam = Matrix::translationMatrix(cam_pos * -1) * worldToCam;
    worldToCam = Matrix::rotationMatrix(ang, eixo) * worldToCam;

    //Transçada a camera em seu sistema local
    cam_pos = pos;

    worldToCam = Matrix::translationMatrix(pos) * worldToCam;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
    glMultMatrixf(worldToCam.inverse().getMatrixGL());
    glMatrixMode(GL_MODELVIEW);
}

void GLAux::Controller() {
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
        return NULL;
    }

    return texture;
}

void GLAux::ApplySurface(int x, int y, SDL_Surface *origem, SDL_Surface *destino, SDL_Rect *clip = NULL) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    //Aplica a Imagem na screen.
    SDL_BlitSurface(origem, clip, destino, &offset);
}

SDL_Surface *GLAux::LoadSurface(char *nome) {
    auto* newImage = IMG_Load(nome);
    auto* newImageOptimized = SDL_ConvertSurface(newImage, newImage->format, 0);

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
    SDL_Rect *clip = NULL;
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
