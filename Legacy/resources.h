#ifndef RESOURCES_H
#define RESOURCES_H

#define TEXTURE 0
#define   MODEL 1
#define    FONT 2

#include "MeshBuffer.h"
#include "UnboxingEngine.h"
#include "glm.h"
#include "lista.h"
#include <stdio.h>
#include <string>

/*Carrega previamente os recursos do editor, objetos e texturas.*/

class Resources
{
    public:
    static List<Texture> textures;
    static List<CMeshBuffer> models;
    static List<int> fonts;

    public:
    static bool loadResource(char *name, int type, int &id);
    static void *getResource(int type, int id);
};

#endif
