#ifndef RESOURCES_H
#define RESOURCES_H

#define TEXTURE 0
#define   MODEL 1
#define    FONT 2

#include <string>
#include <stdio.h>
#include "lista.h"
#include "glm.h"
#include "GLAux.h"
#include "meshbuffer.h"

/*Carrega previamente os recursos do editor, objetos e texturas.*/

class Resources
{
    public:
    static List<Texture> textures;
    static List<meshBuffer> models;
    static List<int> fonts;

    public:
    static bool loadResource(char *name, int type, int &id);
    static void *getResource(int type, int id);
};

#endif
