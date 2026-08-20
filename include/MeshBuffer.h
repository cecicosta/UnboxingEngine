#pragma once

#include "BoundingBox.h"

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
    CBoundingBox3D boundingBox;
    std::vector<float> vertices;
    std::vector<float> texcoords;
    std::vector<float> normals;
    std::vector<unsigned> triangles; //List of triangle vertices indexes.
    std::vector<face> faces;
    //material *materials;
    unsigned nvertices = 0;
    unsigned nnormals = 0;
    unsigned ntexcoords = 0;
    unsigned nfaces = 0;
    unsigned nmaterials = 0;
    
    [[nodiscard]] std::unique_ptr<float*> GetVertexDataArray() const;

    CMeshBuffer() = default;
    CMeshBuffer(size_t numberOfVertices) {
        nnormals = 0;
        ntexcoords = 0;
        nfaces = 0;
        vertices.resize(3 * 2 * numberOfVertices - 3);
        triangles.resize(3 * (numberOfVertices-1));
        nvertices = 0;
    }
};

}// namespace unboxing_engine