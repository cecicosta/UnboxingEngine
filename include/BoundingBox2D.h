#pragma once

#include "UVector.h"

#include <vector>

class CBoundingBox2D {
public:
    CBoundingBox2D(const Vector2f &first, const Vector2f &second);
    ~CBoundingBox2D() = default;

    std::vector<float> GetVertices() const;
    std::vector<unsigned int> GetTriangles() const;

private:
    void SetVertices();
    void SetTriangles();

    std::vector<unsigned int> m_triangles;
    std::vector<float> m_vertices;
    Vector2f m_first;
    Vector2f m_second;
};