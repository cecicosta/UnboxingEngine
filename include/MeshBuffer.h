#pragma once

#include "BoundingBox.h"
#include "material.h"

#include <memory>

namespace unboxing_engine {

typedef unsigned int uint;
typedef struct face {
    uint id;
    uint vertex[4];
    uint vnormal[4];
    uint texUV[4];
    float normal[3];
} face;


class CMeshBuffer {

public:
    BoundingBox boundingBox;
    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<float> normals;
    std::vector<unsigned int> triangles; //List of triangle vertices indexes.
    std::vector<face> faces;
    //material *materials;
    uint nvertices;
    uint nnormals;
    uint ntexcoords;
    uint nfaces;
    uint nmaterials;
    SMaterial material;
    
    [[nodiscard]] std::unique_ptr<float*> &&GetVertexDataArray() const;
};

}// namespace unboxing_engine