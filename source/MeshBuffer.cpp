#include "MeshBuffer.h"
#include <cassert>
#include <memory>

namespace unboxing_engine {

// TODO: Study how to optimize this copy or if the copy is necessary
std::unique_ptr<float *> &&CMeshBuffer::GetVertexDataArray() const {
    auto *vertexData = new float[nvertices * 3 + ntexcoords * 2 + nnormals * 4];
    for (int i = 0; i < nfaces; i++) {
        memcpy(vertexData, &vertices[faces[i].vertex[0]], 3 * sizeof(float));
        memcpy(vertexData, &texcoords[faces[i].texUV[0]], 2 * sizeof(float));
        memcpy(vertexData, &normals[faces[i].vnormal[0]], 3 * sizeof(float));

        memcpy(vertexData, &vertices[faces[i].vertex[1]], 3 * sizeof(float));
        memcpy(vertexData, &texcoords[faces[i].texUV[1]], 2 * sizeof(float));
        memcpy(vertexData, &normals[faces[i].vnormal[1]], 3 * sizeof(float));

        memcpy(vertexData, &vertices[faces[i].vertex[2]], 3 * sizeof(float));
        memcpy(vertexData, &texcoords[faces[i].texUV[2]], 2 * sizeof(float));
        memcpy(vertexData, &normals[faces[i].vnormal[2]], 3 * sizeof(float));
    }
    return std::move(std::make_unique<float *>(vertexData));
}
}// namespace unboxing_engine