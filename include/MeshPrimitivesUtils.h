#pragma once

#include "MeshBuffer.h"
#include "BoundingBox2D.h"
#include "UVector.h"

#include <algorithm>
#include <cmath>
#include <limits>

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

    inline void AddVetex(CMeshBuffer &mesh, Vector3f point) {
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
    [[nodiscard]] inline CMeshBuffer *Cube() {
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
    [[nodiscard]] inline std::unique_ptr<CMeshBuffer> Quad() {
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

    [[nodiscard]] inline std::unique_ptr<CMeshBuffer> Sphere(float radius, unsigned int longitudeSegments = 32, unsigned int latitudeSegments = 16) {
        constexpr float pi = 3.14159265358979323846f;
        radius = std::fabs(radius);
        longitudeSegments = longitudeSegments < 3 ? 3 : longitudeSegments;
        latitudeSegments = latitudeSegments < 2 ? 2 : latitudeSegments;

        auto mesh = std::make_unique<CMeshBuffer>();
        mesh->nnormals = 0;
        mesh->ntexcoords = 0;
        mesh->nmaterials = 0;

        const unsigned int ringCount = latitudeSegments - 1;
        mesh->nvertices = ringCount * longitudeSegments + 2;
        mesh->nfaces = 2 * longitudeSegments + (latitudeSegments - 2) * longitudeSegments * 2;
        mesh->vertices.reserve(mesh->nvertices * 3);
        mesh->triangles.reserve(mesh->nfaces * 3);
        mesh->faces.resize(mesh->nfaces);

        auto appendVertex = [&](const Vector3f &point) {
            mesh->vertices.push_back(point.x);
            mesh->vertices.push_back(point.y);
            mesh->vertices.push_back(point.z);
            return static_cast<unsigned int>(mesh->vertices.size() / 3 - 1);
        };

        auto appendTriangle = [&](unsigned int vertex0, unsigned int vertex1, unsigned int vertex2) {
            mesh->triangles.push_back(vertex0);
            mesh->triangles.push_back(vertex1);
            mesh->triangles.push_back(vertex2);
        };

        const unsigned int topIndex = appendVertex(Vector3f(0.0f, 0.0f, radius));
        for (unsigned int latitude = 1; latitude < latitudeSegments; ++latitude) {
            const float latitudeAngle = pi * static_cast<float>(latitude) / static_cast<float>(latitudeSegments);
            const float z = radius * cosf(latitudeAngle);
            const float ringRadius = radius * sinf(latitudeAngle);

            for (unsigned int longitude = 0; longitude < longitudeSegments; ++longitude) {
                const float longitudeAngle = 2.0f * pi * static_cast<float>(longitude) / static_cast<float>(longitudeSegments);
                appendVertex(Vector3f(ringRadius * cosf(longitudeAngle), ringRadius * sinf(longitudeAngle), z));
            }
        }
        const unsigned int bottomIndex = appendVertex(Vector3f(0.0f, 0.0f, -radius));

        auto ringVertex = [longitudeSegments](unsigned int ring, unsigned int longitude) {
            return 1 + ring * longitudeSegments + longitude;
        };

        for (unsigned int longitude = 0; longitude < longitudeSegments; ++longitude) {
            const unsigned int nextLongitude = (longitude + 1) % longitudeSegments;
            appendTriangle(topIndex, ringVertex(0, longitude), ringVertex(0, nextLongitude));
        }

        for (unsigned int ring = 0; ring + 1 < ringCount; ++ring) {
            const unsigned int nextRing = ring + 1;
            for (unsigned int longitude = 0; longitude < longitudeSegments; ++longitude) {
                const unsigned int nextLongitude = (longitude + 1) % longitudeSegments;
                const unsigned int current = ringVertex(ring, longitude);
                const unsigned int nextLatitude = ringVertex(nextRing, longitude);
                const unsigned int nextLongitudeCurrent = ringVertex(ring, nextLongitude);
                const unsigned int nextBoth = ringVertex(nextRing, nextLongitude);

                appendTriangle(current, nextLatitude, nextBoth);
                appendTriangle(nextBoth, nextLongitudeCurrent, current);
            }
        }

        const unsigned int lastRing = ringCount - 1;
        for (unsigned int longitude = 0; longitude < longitudeSegments; ++longitude) {
            const unsigned int nextLongitude = (longitude + 1) % longitudeSegments;
            appendTriangle(ringVertex(lastRing, longitude), bottomIndex, ringVertex(lastRing, nextLongitude));
        }

        mesh->nvertices = static_cast<unsigned int>(mesh->vertices.size() / 3);
        mesh->nfaces = static_cast<unsigned int>(mesh->triangles.size() / 3);
        const Vector3f bounds(radius, radius, radius);
        mesh->boundingBox = CBoundingBox3D(bounds * -1.0f, bounds);
        return mesh;
    }

    [[nodiscard]] inline std::unique_ptr<CMeshBuffer> Cylinder(
            const Vector3f &bottomCenter,
            const Vector3f &topCenter,
            float bottomRadius,
            float topRadius,
            unsigned int segments = 32) {
        constexpr float pi = 3.14159265358979323846f;
        constexpr float epsilon = 0.000001f;
        segments = segments < 3 ? 3 : segments;
        bottomRadius = std::fabs(bottomRadius);
        topRadius = std::fabs(topRadius);

        auto mesh = std::make_unique<CMeshBuffer>();
        mesh->nnormals = 0;
        mesh->ntexcoords = 0;
        mesh->nmaterials = 0;

        bool hasBounds = false;
        Vector3f boundsMin;
        Vector3f boundsMax;

        auto appendVertex = [&](const Vector3f &point) {
            mesh->vertices.push_back(point.x);
            mesh->vertices.push_back(point.y);
            mesh->vertices.push_back(point.z);

            if (!hasBounds) {
                boundsMin = point;
                boundsMax = point;
                hasBounds = true;
            } else {
                boundsMin.x = std::min(boundsMin.x, point.x);
                boundsMin.y = std::min(boundsMin.y, point.y);
                boundsMin.z = std::min(boundsMin.z, point.z);
                boundsMax.x = std::max(boundsMax.x, point.x);
                boundsMax.y = std::max(boundsMax.y, point.y);
                boundsMax.z = std::max(boundsMax.z, point.z);
            }

            return static_cast<unsigned int>(mesh->vertices.size() / 3 - 1);
        };

        auto appendTriangle = [&](unsigned int vertex0, unsigned int vertex1, unsigned int vertex2) {
            mesh->triangles.push_back(vertex0);
            mesh->triangles.push_back(vertex1);
            mesh->triangles.push_back(vertex2);
        };

        const bool hasBottomRadius = bottomRadius > epsilon;
        const bool hasTopRadius = topRadius > epsilon;
        if (!hasBottomRadius && !hasTopRadius) {
            appendVertex(bottomCenter);
            appendVertex(topCenter);
            mesh->nvertices = static_cast<unsigned int>(mesh->vertices.size() / 3);
            mesh->nfaces = 0;
            mesh->boundingBox = CBoundingBox3D(boundsMin, boundsMax);
            return mesh;
        }

        const Vector3f axis = topCenter - bottomCenter;
        const float height = axis.Length();
        const Vector3f axisDirection = height > epsilon ? axis / height : Vector3f(0.0f, 0.0f, 1.0f);
        const Vector3f reference = std::fabs(axisDirection.y) < 0.999f
                                           ? Vector3f(0.0f, 1.0f, 0.0f)
                                           : Vector3f(1.0f, 0.0f, 0.0f);
        Vector3f tangent = reference.CrossProduct(axisDirection);
        const float tangentLength = tangent.Length();
        tangent = tangentLength > epsilon ? tangent / tangentLength : Vector3f(1.0f, 0.0f, 0.0f);
        const Vector3f bitangent = axisDirection.CrossProduct(tangent).Normalized();

        std::vector<unsigned int> bottomRing;
        std::vector<unsigned int> topRing;
        bottomRing.reserve(segments);
        topRing.reserve(segments);
        mesh->vertices.reserve((segments * 2 + 2) * 3);
        mesh->triangles.reserve(segments * 12);

        for (unsigned int i = 0; i < segments; ++i) {
            const float angle = 2.0f * pi * static_cast<float>(i) / static_cast<float>(segments);
            const Vector3f radial = tangent * cosf(angle) + bitangent * sinf(angle);
            if (hasBottomRadius) {
                bottomRing.push_back(appendVertex(bottomCenter + radial * bottomRadius));
            }
            if (hasTopRadius) {
                topRing.push_back(appendVertex(topCenter + radial * topRadius));
            }
        }

        unsigned int bottomCenterIndex = std::numeric_limits<unsigned int>::max();
        unsigned int topCenterIndex = std::numeric_limits<unsigned int>::max();
        if (!hasBottomRadius) {
            bottomCenterIndex = appendVertex(bottomCenter);
        }
        if (!hasTopRadius) {
            topCenterIndex = appendVertex(topCenter);
        }
        if (hasBottomRadius) {
            bottomCenterIndex = appendVertex(bottomCenter);
        }
        if (hasTopRadius) {
            topCenterIndex = appendVertex(topCenter);
        }

        for (unsigned int i = 0; i < segments; ++i) {
            const unsigned int next = (i + 1) % segments;

            if (hasBottomRadius && hasTopRadius) {
                appendTriangle(bottomRing[i], bottomRing[next], topRing[next]);
                appendTriangle(bottomRing[i], topRing[next], topRing[i]);
            } else if (hasBottomRadius) {
                appendTriangle(bottomRing[i], bottomRing[next], topCenterIndex);
            } else if (hasTopRadius) {
                appendTriangle(bottomCenterIndex, topRing[next], topRing[i]);
            }

            if (hasBottomRadius) {
                appendTriangle(bottomCenterIndex, bottomRing[next], bottomRing[i]);
            }
            if (hasTopRadius) {
                appendTriangle(topCenterIndex, topRing[i], topRing[next]);
            }
        }

        mesh->nvertices = static_cast<unsigned int>(mesh->vertices.size() / 3);
        mesh->nfaces = static_cast<unsigned int>(mesh->triangles.size() / 3);
        mesh->faces.resize(mesh->nfaces);
        mesh->boundingBox = CBoundingBox3D(boundsMin, boundsMax);
        return mesh;
    }

    [[nodiscard]] inline std::unique_ptr<CMeshBuffer> Cylinder(
            const Vector3f &bottomCenter,
            const Vector3f &topCenter,
            float radius,
            unsigned int segments = 32) {
        return Cylinder(bottomCenter, topCenter, radius, radius, segments);
    }

    
}// namespace primitive_utils
