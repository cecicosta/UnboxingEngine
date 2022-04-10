#pragma once

#include <memory>

#include "boundingbox.h"
#include "glm.h"
#include "lista.h"
#include "material.h"
#include "matrix.h"

typedef unsigned int uint;
typedef struct vertexcoord {
    uint id;
    float coord[3];
} vertex;

typedef struct normalcoord {
    uint id;
    float coord[3];
} normal;

typedef struct texcoord {
    uint id;
    float coord[2];
} texcoord;

typedef struct face {
    uint id;
    uint matID;
    uint n;
    bool hasChecked;
    uint vertex[4];
    uint vnormal[4];
    uint texUV[4];
    float normal[3];
    float color[4];
} face;


class meshBuffer {

public:
    BoundingBox boundingBox;
    vertexcoord *vertices;
    normalcoord *normals;
    texcoord *texcoords;
    face *faces;
    material *materials;
    uint nvertices;
    uint nnormals;
    uint ntexcoords;
    uint nfaces;
    uint nmaterials;

    const uint8_t kFaceStrideOffsetBytes = 24 * sizeof(float);
    const uint8_t kVertexStrideOffsetBytes = 5 * sizeof(float);
    const uint8_t kTextureStrideOffsetBytes = 6 * sizeof(float);
    const uint8_t kNormalsStrideOffsetBytes = 5 * sizeof(float);

    static meshBuffer *createMeshBufferOBJ(GLMmodel *model);
    std::unique_ptr<float*> &&GetVertexDataArray() const;
};

