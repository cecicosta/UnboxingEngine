#pragma once

#include "MeshBuffer.h"
#include "BoundingBox2D.h"
#include "UVector.h"

#include <cmath>

namespace unboxing_engine::primitive_utils {

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

    void AddVetex(CMeshBuffer &mesh, Vector3f point) {
        uint vertexOffset = 3 * mesh.nvertices;
        InsertCoordinates(&mesh.vertices[vertexOffset], vertexOffset, point.x, point.y, point.z);
        if (mesh.nvertices > 0) {
            uint elementOffset = 2 * mesh.nfaces;
            //Adding dummy vertice to complete a triangle while the render cannot only render lines 
            InsertCoordinates(&mesh.vertices[vertexOffset], vertexOffset, point.x, point.y, point.z);
            mesh.nvertices++;
            InsertCoordinates(&mesh.triangles[elementOffset], elementOffset, mesh.nvertices - 2, mesh.nvertices - 1, mesh.nvertices);
            mesh.nfaces++;
        }
        mesh.nvertices++;
    }

    template<typename T>
    void Lines(CMeshBuffer &mesh, const T& point) {
        AddVetex(mesh, point);
    }

    template<typename T, typename... Args>
    void Lines(CMeshBuffer &mesh, const T &point,Args... points) {
        AddVetex(mesh, point);
        Lines(mesh, points...);
    }

    //Create a cube mesh
    template <typename T, typename... Args>
    [[nodiscard]] std::unique_ptr<CMeshBuffer> Lines(const T &point_1, Args... points) {
        auto mesh = std::make_unique<CMeshBuffer>();
        constexpr std::size_t n = sizeof...(Args) + 1;
        mesh->nnormals = 0;
        mesh->ntexcoords = 0;
        mesh->nfaces = 0;
        mesh->vertices.resize(3 * 2 * n - 3);
        mesh->triangles.resize(3 * (n-1));
        mesh->nvertices = 0;

        //mesh->boundingBox = BoundingBox(Vector3f(-0.5f, -0.5f, -0.5f), Vector3f(0.5f, 0.5f, 0.5f));
        AddVetex(*mesh, point_1);
        Lines(*mesh, points...);
        return mesh;
    }

    //Create a cube mesh
    [[nodiscard]] CMeshBuffer *Cube() {
        auto mesh = new CMeshBuffer();
        mesh->boundingBox = CBoundingBox3D(Vector3f(-0.5f, -0.5f, -0.5f), Vector3f(0.5f, 0.5f, 0.5f));
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
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.0f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.5f);

        // Back Face (1,2)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 0, -1);
        InsertCoordinates(mesh->faces[1].vnormal, vNormalIdOffset, 1, 1, 1, 1);
        InsertCoordinates(mesh->faces[1].normal, fNormalOffset, 0, 0, -1);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.0f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.0f);


        // Top Face (1,3)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, 1, 0);
        InsertCoordinates(mesh->faces[2].vnormal, vNormalIdOffset, 2, 2, 2, 2);
        InsertCoordinates(mesh->faces[2].normal, fNormalOffset, 0, 1, 0);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.0f);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.0f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.5f);

        
        // Bottom Face (2,1)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, 0, -1, 0);
        InsertCoordinates(mesh->faces[3].vnormal, vNormalIdOffset, 3, 3, 3, 3);
        InsertCoordinates(mesh->faces[3].normal, fNormalOffset, 0, -1, 0);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 1.0f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 1.0f);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.0f, 0.5f);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);
        
        
        // Right face (2,2)
        InsertCoordinates(&mesh->normals[normalOffset], fNormalOffset, 1, 0, 0);
        InsertCoordinates(mesh->faces[4].vnormal, vNormalIdOffset, 4, 4, 4, 4);
        InsertCoordinates(mesh->faces[4].normal, normalOffset, 1, 0, 0);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 1.0f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 1.0f);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.333f, 0.5f);


        // Left Face (2,3)
        InsertCoordinates(&mesh->normals[normalOffset], normalOffset, -1, 0, 0);
        InsertCoordinates(mesh->faces[5].vnormal, vNormalIdOffset, 5, 5, 5, 5);
        InsertCoordinates(mesh->faces[5].normal, vNormalIdOffset, -1, 0, 0);
        // Bottom Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 0.5f);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 0.5f);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1.0f, 1.0f);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0.666f, 1.0f);
        
        return mesh;
    }

    //Create a cube mesh
    [[nodiscard]] std::unique_ptr<CMeshBuffer> Quad() {
        auto mesh = std::make_unique<CMeshBuffer>();
        auto boundingBox = CBoundingBox2D(Vector2f(-0.5f, -0.5f), Vector2f(0.5f, 0.5f));
        mesh->boundingBox = CBoundingBox3D(Vector3f(-0.5f, -0.5f, 0), Vector3f(0.5f, 0.5f, 0));
        mesh->nvertices = 4;
        mesh->nnormals = 2;
        mesh->ntexcoords = 4;
        mesh->nfaces = 2;
        mesh->vertices = boundingBox.GetVertices();
        mesh->triangles = boundingBox.GetTriangles();
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
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0, 0);
        // Bottom Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1, 0);
        // Top Right Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 1, 1);
        // Top Left Of The Texture and Quad
        InsertCoordinates(&mesh->texcoords[textureOffset], textureOffset, 0, 1);

        return mesh;
    }

    [[nodiscard]] inline std::unique_ptr<CMeshBuffer> Torus(float majorRadius = 2.4f, float minorRadius = 0.75f, unsigned int majorSegments = 48, unsigned int minorSegments = 16) {
        constexpr float pi = 3.14159265358979323846f;
        majorSegments = majorSegments < 3 ? 3 : majorSegments;
        minorSegments = minorSegments < 3 ? 3 : minorSegments;

        auto mesh = std::make_unique<CMeshBuffer>();
        mesh->nvertices = majorSegments * minorSegments;
        mesh->nfaces = majorSegments * minorSegments * 2;
        mesh->nnormals = 0;
        mesh->ntexcoords = 0;
        mesh->nmaterials = 0;
        mesh->vertices.reserve(mesh->nvertices * 3);
        mesh->triangles.reserve(mesh->nfaces * 3);
        mesh->faces.resize(mesh->nfaces);

        for (unsigned int i = 0; i < majorSegments; ++i) {
            float majorAngle = 2.0f * pi * static_cast<float>(i) / static_cast<float>(majorSegments);
            float majorCos = cosf(majorAngle);
            float majorSin = sinf(majorAngle);

            for (unsigned int j = 0; j < minorSegments; ++j) {
                float minorAngle = 2.0f * pi * static_cast<float>(j) / static_cast<float>(minorSegments);
                float minorCos = cosf(minorAngle);
                float minorSin = sinf(minorAngle);
                float ringRadius = majorRadius + minorRadius * minorCos;

                mesh->vertices.push_back(ringRadius * majorCos);
                mesh->vertices.push_back(ringRadius * majorSin);
                mesh->vertices.push_back(minorRadius * minorSin);
            }
        }

        for (unsigned int i = 0; i < majorSegments; ++i) {
            unsigned int nextI = (i + 1) % majorSegments;
            for (unsigned int j = 0; j < minorSegments; ++j) {
                unsigned int nextJ = (j + 1) % minorSegments;
                unsigned int current = i * minorSegments + j;
                unsigned int nextMajor = nextI * minorSegments + j;
                unsigned int nextMinor = i * minorSegments + nextJ;
                unsigned int nextBoth = nextI * minorSegments + nextJ;

                mesh->triangles.push_back(current);
                mesh->triangles.push_back(nextMajor);
                mesh->triangles.push_back(nextBoth);
                mesh->triangles.push_back(nextBoth);
                mesh->triangles.push_back(nextMinor);
                mesh->triangles.push_back(current);
            }
        }

        float boundsXY = majorRadius + minorRadius;
        mesh->boundingBox = CBoundingBox3D(Vector3f(-boundsXY, -boundsXY, -minorRadius), Vector3f(boundsXY, boundsXY, minorRadius));
        return mesh;
    }

    
}// namespace primitive_utils
