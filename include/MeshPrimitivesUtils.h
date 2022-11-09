#pragma once

#include "MeshBuffer.h"

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
        mesh->boundingBox = BoundingBox(Vector3f(-0.5f, -0.5f, -0.5f), Vector3f(0.5f, 0.5f, 0.5f));
        mesh->nvertices = 8;
        mesh->nnormals = 12;
        mesh->ntexcoords = 24;
        mesh->nfaces = 12;
        mesh->vertices = mesh->boundingBox.GetVertices();
        mesh->triangles = mesh->boundingBox.GetTriangles();
        mesh->normals.resize(3 * mesh->nnormals);
        mesh->texcoords.resize(2 * mesh->ntexcoords);
        mesh->faces.resize(mesh->nfaces);

        uint textureOffset = 0;
        uint normalOffset = 0;
        uint fNormalOffset = 0;
        uint vNormalIdOffset = 0;

        // Front Face (1,1)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 0, 1);
        InsertCoordinates(mesh->faces[vNormalIdOffset].vnormal, vNormalIdOffset, 0, 0, 0, 0);
        InsertCoordinates(mesh->faces[fNormalOffset].normal, fNormalOffset, 0, 0, 1);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.0f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset,  -0.5f, -0.5f, 0.5f);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.0f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset,  0.5f, -0.5f, 0.5f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, 0.5f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.5f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, 0.5f);
        //InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 0, 1, 2);
        //InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 2, 3, 0);

        // Back Face (1,2)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 0, -1);
        InsertCoordinates(mesh->faces[1].vnormal, vNormalIdOffset, 1, 1, 1, 1);
        InsertCoordinates(mesh->faces[1].normal, fNormalOffset, 0, 0, -1);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.0f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, -0.5f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, -0.5f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, -0.5f);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.0f);
        //InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, -0.5f);
        //InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 4, 5, 6);
        //InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 6, 7, 4);

        // Top Face (1,3)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 1, 0);
        InsertCoordinates(mesh->faces[2].vnormal, vNormalIdOffset, 2, 2, 2, 2);
        InsertCoordinates(mesh->faces[2].normal, fNormalOffset, 0, 1, 0);
//        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, -0.5f);
//        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, 0.5f);
//        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, 0.5f);
//        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, -0.5f);
        //InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 3, 2, 6);
        //InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 6, 5, 3);

        
        // Bottom Face (2,1)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, -1, 0);
        InsertCoordinates(mesh->faces[3].vnormal, vNormalIdOffset, 3, 3, 3, 3);
        InsertCoordinates(mesh->faces[3].normal, fNormalOffset, 0, -1, 0);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 1.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, -0.5f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 1.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, -0.5f);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, 0.5f);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, 0.5f);
        
//        InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 0, 4, 7);
//        InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 7, 1, 0);
        
        
        // Right face (2,2)
        InsertCoordinates(&mesh->normals[normalOffset], fNormalOffset, 1, 0, 0);
        InsertCoordinates(mesh->faces[4].vnormal, vNormalIdOffset, 4, 4, 4, 4);
        InsertCoordinates(mesh->faces[4].normal, normalOffset, 1, 0, 0);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, -0.5f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 1.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, -0.5f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 1.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, 0.5f, 0.5f);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, 0.5f, -0.5f, 0.5f);
        
//        InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 7, 6, 2);
//        InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 2, 1, 7);

        // Left Face (2,3)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, -1, 0, 0);
        InsertCoordinates(mesh->faces[5].vnormal, vNormalIdOffset, 5, 5, 5, 5);
        InsertCoordinates(mesh->faces[5].normal, vNormalIdOffset, -1, 0, 0);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, -0.5f);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.5f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, -0.5f, 0.5f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 1.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, 0.5f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 1.0f);
//        InsertCoordinates(&mesh->vertices[vertexOffset], vertexOffset, -0.5f, 0.5f, -0.5f);
        
//        InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 3, 5, 4);
//        InsertCoordinates(&mesh->triangles[triangleOffset], triangleOffset, 4, 0, 3);
        
        return mesh;
    }
}// namespace primitive_utils