#include <iostream>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>

#include "glm.h" //.obj loader library

#include <GL/gl.h>
#include "GLAux.h"
#include "lista.h"
#include "lightsource.h"
#include "meshbuffer.h"
#include "octree.h"
#include "SceneElement.h"
#include "resources.h"
#include "draw.h"
#include "collisionutil.h"
#include "physicsutil.h"

#include <ctime>


//Includes para o jogo
//#include "mazeGenerator.h"


using namespace std;

void liberar();

int main(int arg, char* argv[])
{

    GLAux::Init( true );
    bool quit = false;
    srand(time(nullptr));
/*
    Labirinto l;
    l.inicializar();
    l.redimencionar(11,11);
    l.ronald_lab();
    l.ronald_print();
*/
    while(!GLAux::quit)
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        GLAux::Controller();
        GLAux::camFPS();
        glLoadIdentity();
        //GLAux::rayFromScreenCoord();


        GLAux::Timer(120);
        //SDL_GL_SwapBuffers();
    }
    liberar();
    return 0;
}

void liberar()
{
    Resources::models.deleteAll();
    Resources::textures.deleteAll();

    collut::mesh.deleteAll();
    collut::objects.deleteAll();
    collut::SEObjects.deleteAll();

    physicut::ph_objects.free();
}
