#pragma once

#include "matrix.h"
#include "quaternion.h"
#include "sphere.h"
#include "texture.h"
#include "vector3D.h"
#include "SDL_video.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

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
    static bool mouse_buttonPressed;
    static int mouse_wheel;

    static quaternion q;
    static vector3D cam_pos;
    static vector3D cam_dir;
    static vector3D cam_up;
    static Matrix transfomation;
    static Matrix worldToCam;

    static float mouseTracking_x;
    static float mouseTracking_y;
    static float vel_x;
    static float vel_y;
    static float mouse_x;
    static float mouse_y;
    static float mouse_click_x;
    static float mouse_click_y;

    //Atributos da screen.
    static bool quit;
    static int WIDTH;
    static int HEIGHT;
    static int BPP;
    //Configuraes da matriz de perspectiva
    static double xmin, xmax, ymin, ymax, fovy, zNear, zFar, aspect;

    static SDL_Window *screen;
    static SDL_Event event;
    static SDL_GLContext mGLContext;
    static std::uint8_t const *keyState;
    static float position[3];
    static int mousestate[3];

    static std::uint32_t vao;
    static std::uint32_t program;

    //Inicia os subsistemas da SDL.
    static void Init(bool ortho);
    //Responsavel por capturar os comandos do usuario.
    static void Controller();
    //Timer.
    static void Timer(int tempo);
    //Carrega uma imagem como textura.
    static Texture *LoadTexture(char *filename);
    //Converte uma SDL_surface para uma textura opengl
    static Texture *CreateTextureFromSurface(SDL_Surface *sdlimage);
    //Alterna entre a vis�o de perspectiva e a vi�o ortogonal
    static void OrthogonalView(bool ortho, float pos[3]);
    //Desenha um plano com uma textura
    static void draw2DObject(plane2D area, Texture *texture, float cor[4], float rect[4]);
    //Desenha esfera dada a posi��o e o raio
    static void drawSphere(vector3D position, float radius);
    //Retorna coordenadas de raio do mouse, dado o comprimento deste no eixo Z
    static void rayFromScreenCoord(int scr_x, int scr_y, vector3D &point, vector3D &dir);
    //Converte coordenadas 2D para 3D
    static vector3D convert2DTo3DCoord(float X, float Y, float dist);
    //Controlador da camera
    static void camFPS();
    static void setCam(const vector3D& pos, float ang, const vector3D& axi, const vector3D& point);
    static void CreateBasicShader();
    static void ReleaseCurrentGLContext();


    //Aplica uma surface em outra surface.
    static void ApplySurface(int x, int y, SDL_Surface *origem, SDL_Surface *destino, SDL_Rect *clip);
    //Carrega uma imagem.
    static SDL_Surface *LoadSurface(char *nome);
    //Carrega uma imagem com transparencia
    static SDL_Surface *LoadSurfaceAlpha(char *nome);
    //Divide os sprits da surface.
    static SDL_Rect *CreateSurfaceClips(int linhas, int colunas, int intervalo_x, int intervalo_y);
    static void CreateArray();

    static std::vector<float> vertices;
};
