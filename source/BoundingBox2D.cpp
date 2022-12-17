#include "BoundingBox2D.h"


CBoundingBox2D::CBoundingBox2D(const Vector2f &first, const Vector2f &second)
    : m_vertices(12)
    , m_triangles(6) {
    m_first = first;
    m_second = second;
    SetVertices();
    SetTriangles();
}

std::vector<float> CBoundingBox2D::GetVertices() const {
    return m_vertices;
}

std::vector<unsigned int> CBoundingBox2D::GetTriangles() const {
    return m_triangles;
}


void CBoundingBox2D::SetVertices() {
    m_vertices[0] = m_first.x;
    m_vertices[1] = m_first.y;
    m_vertices[2] = 0;

    m_vertices[3] = m_first.x;
    m_vertices[4] = m_second.y;
    m_vertices[5] = 0;

    m_vertices[6] = m_second.x;
    m_vertices[7] = m_second.y;
    m_vertices[8] = 0;

    m_vertices[9] = m_second.x;
    m_vertices[10] = m_first.y;
    m_vertices[11] = 0;
}

void CBoundingBox2D::SetTriangles() {
    m_triangles[0] = 0;
    m_triangles[1] = 1;
    m_triangles[2] = 2;
    m_triangles[3] = 2;
    m_triangles[4] = 3;
    m_triangles[5] = 0;
}
