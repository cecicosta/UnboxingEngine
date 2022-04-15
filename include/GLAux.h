#pragma once

#include "SDL_events.h"
#include "SDL_timer.h"
#include "SDL_video.h"
#include "matrix.h"
#include "quaternion.h"
#include "sphere.h"
#include "texture.h"
#include "vector3D.h"
#include "Camera.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

#define L_BUTTON 0
#define R_BUTTON 1

#define WHITE SDL_MapRGB(GLAux::screen->format, 255, 255, 255)
#define BLACK SDL_MapRGB(GLAux::screen->format, 0, 0, 0)
#define RED SDL_MapRGB(GLAux::screen->format, 255, 0, 0)
#define GREEN SDL_MapRGB(GLAux::screen->format, 0, 255, 0)
#define BLUE SDL_MapRGB(GLAux::screen->format, 0, 0, 255)
#define YELLOW SDL_MapRGB(GLAux::screen->format, 255, 255, 0)


class SDL_Surface;
union SDL_Event;
class SDL_Rect;
class SDL_Window;

typedef struct plane2D {
    float x;
    float y;
    float z;
    float w;
    float h;
} plane2D;

class GLAux {
public:
    GLAux(uint32_t width, uint32_t height, uint32_t bpp);
    ~GLAux() = default;

    //Create controllable camera
    void FPSCamera();

    std::unique_ptr<Camera> camera;

    //Shader properties
    std::uint32_t vao = -1;

    std::uint32_t program = -1;

private:
    //SDL input event handling
    SDL_Event event;

    //Mouse input attributes
    float mouseTracking_x = 0;
    float mouseTracking_y = 0;
    float vel_x = 0;
    float vel_y = 0;
    float mouse_x = 0;
    float mouse_y = 0;
    float mouse_click_x = 0;
    float mouse_click_y = 0;
    bool mouse_buttonPressed = false;
    int mouse_wheel = -1;
    int mousestate[3]{0, 0, 0};

    //Keyboard input attributes
    std::uint8_t const *keyState;

    //Screen attributes
    bool quit = false;
    std::uint32_t BPP = 32;


    SDL_Window *mWindow = nullptr;
    SDL_GLContext mGLContext = nullptr;

    std::uint32_t vbo = -1;
    std::uint32_t ebo = -1;

public:
    //Create new window and setup view
    void CreateWindow(bool isOrthogonal);
    //Capture user input
    void OnInput();
    //Timer
    void Timer(int tempo);
    //Carrega uma imagem como textura.
    Texture *LoadTexture(char *filename);
    //Converte uma SDL_surface para uma textura opengl
    Texture *CreateTextureFromSurface(SDL_Surface *sdlimage);
    //Alterna entre a vis�o de perspectiva e a vi�o ortogonal
    void OrthogonalView(bool isOrthogonal);
    //Desenha um plano com uma textura
    void draw2DObject(plane2D area, Texture *texture, float cor[4], float rect[4]);
    //Retorna coordenadas de raio do mouse, dado o comprimento deste no eixo Z

    void CreateBasicShader();
    void Release();

    //Aplica uma surface em outra surface.
    void ApplySurface(int x, int y, SDL_Surface *origem, SDL_Surface *destino, SDL_Rect *clip);
    //Carrega uma imagem.
    SDL_Surface *LoadSurface(char *nome);
    //Carrega uma imagem com transparencia
    SDL_Surface *LoadSurfaceAlpha(char *nome);
    //Divide os sprits da surface.
    SDL_Rect *CreateSurfaceClips(int linhas, int colunas, int intervalo_x, int intervalo_y);
    void CreateArray(const float *vertices, size_t vSize, const unsigned int *indices, size_t iSize);
    void RenderCanvas();
    [[nodiscard]] bool HasQuit() { return quit; }
};
