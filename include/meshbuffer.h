#pragma once

#include <memory>


//#include "glm.h"
//#include "boundingbox.h"
//#include "lista.h"
//#include "material.h"
//#include "matrix.h"

typedef unsigned int uint;
typedef struct face {
    uint id;
    uint matID;
    uint n;
    bool hasChecked;
    uint vertex[4];
    uint vnormal[4];
    uint texUV[4];
    float color[4];
    float normal[3];
} face;


class CMeshBuffer {

public:
//    BoundingBox boundingBox;
    float *vertices;
    float *texcoords;
    float *normals;
    unsigned int *triangles; //List of triangle vertices indexes.
    face *faces;
//    material *materials;
    uint nvertices;
    uint nnormals;
    uint ntexcoords;
    uint nfaces;
    uint nmaterials;

//    static CMeshBuffer *createMeshBufferOBJ(GLMmodel *model);
    std::unique_ptr<float*> &&GetVertexDataArray() const;
};

