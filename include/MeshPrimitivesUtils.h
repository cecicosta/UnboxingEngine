#pragma once

#include "meshbuffer.h"

namespace primitive_utils {

    template<typename  T1, typename  T2>
    void InsertCoordinates(T1 *vertices, uint &offset, T2 v) {
        vertices[0] = v;
        offset++;
    }

    template<typename  T1, typename  T2, typename ... T3>
    void InsertCoordinates(T1 *vertices, uint &offset, T2 v, T3... arg) {
        vertices[0] = v;
        InsertCoordinates(++vertices, ++offset, arg...);
    }


    //Create a cube mesh
    [[nodiscard]] CMeshBuffer *Cube() {
        auto mesh = new CMeshBuffer();
        mesh->nvertices = 24;
        mesh->nnormals = 6;
        mesh->ntexcoords = 6;
        mesh->nfaces = 6;
        mesh->vertices = new float[3 * mesh->nvertices];
        mesh->normals = new float[3 * mesh->nnormals];
        mesh->texcoords = new float[2 * mesh->ntexcoords];
        mesh->faces = new face[mesh->nfaces];

        uint vertexOffset = 0;
        uint textureOffset = 0;
        uint normalOffset = 0;
        uint fNormalOffset = 0;
        uint vNormalIdOffset = 0;
        
        // Front Face (1,1)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 0, 1);
        InsertCoordinates(mesh->faces[vNormalIdOffset].vnormal, vNormalIdOffset, 0, 0, 0, 0);
        InsertCoordinates(mesh->faces[fNormalOffset].normal, fNormalOffset, 0, 0, 1);
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset,  -0.5f, -0.5f, 0.5f);// Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset,  0.5f, -0.5f, 0.5f);// Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, 0.5f);// Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, 0.5f);// Top Left Of The Texture and Quad
        // Back Face (1,2)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 0, -1);
        InsertCoordinates(mesh->faces[1].vnormal, vNormalIdOffset, 1, 1, 1, 1);
        InsertCoordinates(mesh->faces[1].normal, fNormalOffset, 0, 0, -1);
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, -0.5f);// Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, -0.5f);// Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, -0.5f);// Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, -0.5f);// Bottom Left Of The Texture and Quad
        // Top Face (1,3)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 1, 0);
        InsertCoordinates(mesh->faces[2].vnormal, vNormalIdOffset, 2, 2, 2, 2);
        InsertCoordinates(mesh->faces[2].normal, fNormalOffset, 0, 1, 0);
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, -0.5f);// Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, 0.5f);// Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, 0.5f);// Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, -0.5f);// Top Right Of The Texture and Quad
        // Bottom Face (2,1)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, -1, 0);
        InsertCoordinates(mesh->faces[3].vnormal, vNormalIdOffset, 3, 3, 3, 3);
        InsertCoordinates(mesh->faces[3].normal, fNormalOffset, 0, -1, 0);
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 1.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, -0.5f);// Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 1.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, -0.5f);// Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, 0.5f);// Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, 0.5f);// Bottom Right Of The Texture and Quad
        // Right face (2,2)
        InsertCoordinates(&mesh->normals[normalOffset], fNormalOffset, 1, 0, 0);
        InsertCoordinates(mesh->faces[4].vnormal, vNormalIdOffset, 4, 4, 4, 4);
        InsertCoordinates(mesh->faces[4].normal, normalOffset, 1, 0, 0);
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, -0.5f);// Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 1.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, -0.5f);// Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 1.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, 0.5f);// Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, 0.5f);// Bottom Left Of The Texture and Quad
        // Left Face (2,3)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, -1, 0, 0);
        InsertCoordinates(mesh->faces[5].vnormal, vNormalIdOffset, 5, 5, 5, 5);
        InsertCoordinates(mesh->faces[5].normal, vNormalIdOffset, -1, 0, 0);
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, -0.5f);// Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.5f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, 0.5f);// Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 1.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, 0.5f);// Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 1.0f);
        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, -0.5f);// Top Left Of The Texture and Quad

        return mesh;
    }
}// namespace primitive_utils