#ifndef MESHBUFFER_H
#define MESHBUFFER_H
#include "lista.h"
#include "boundingbox.h"
#include "material.h"
#include "glm.h"
#include "matrix.h"

typedef unsigned int uint;
typedef struct vertexcoord{
    uint id;
    float coord[3];
}vertex;

typedef struct normalcoord{
    uint id;
    float coord[3];
}normal;

typedef struct texcoord
{
    uint id;
    float coord[2];
}texcoord;

typedef struct face{
    uint id;
    uint matID;
    uint n;
    bool hasChecked;
    uint vertex[4];
    uint vnormal[4];
    uint texUV[4];
    float normal[3];
    float color[4];
}face;



class meshBuffer
{

    public:
    BoundingBox boundingBox;
    vertexcoord *vertices;
    normalcoord *normals;
    texcoord    *texcoords;
    face *faces;
    material *materials;
    uint nvertices;
    uint nnormals;
    uint ntexcoords;
    uint nfaces;
    uint nmaterials;

    static meshBuffer *createMeshBufferOBJ( GLMmodel *model );
    void draw();
};

#endif //MESHBUFFER_H
