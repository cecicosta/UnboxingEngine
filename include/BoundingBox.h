#pragma once

#include "UVector.h"
#include <vector>

enum EFace : size_t {
    FRONT,
    BACK,
    TOP,
    BOTTOM,
    RIGHT,
    LEFT
};

class CBoundingBox3D {
public:
    CBoundingBox3D()  = default;
    CBoundingBox3D(const Vector3f &first, const Vector3f &second);
    CBoundingBox3D(const CBoundingBox3D& other);

    [[nodiscard]] Vector3f getFirst() const;
    [[nodiscard]] Vector3f getSecond() const;
    [[nodiscard]] Vector3f getPivot() const;

    [[nodiscard]] Vector3f GetVertex(EFace face, int vIndex);
    [[nodiscard]] std::vector<float> GetVertices() const;
    [[nodiscard]] std::vector<unsigned int> GetTriangles() const;
    [[nodiscard]] std::vector<unsigned int> GetVertexIdList(EFace face) const;

    CBoundingBox3D& operator=(const CBoundingBox3D& other);
private:
    void SetVertices(const Vector3f &first, const Vector3f &second);
    void SetTriangles();

    std::vector<unsigned int> mFaces;
    std::vector<const float*> mVertices;
    Vector3f mFirst;
    Vector3f mSecond;
    Vector3f mPivot;
};
