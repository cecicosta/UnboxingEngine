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

class BoundingBox {
public:
    BoundingBox()  = default;
    BoundingBox(const Vector3f &first, const Vector3f &second);
    BoundingBox(const BoundingBox& other);

    [[nodiscard]] Vector3f getFirst() const;
    [[nodiscard]] Vector3f getSecond() const;
    [[nodiscard]] Vector3f getPivot() const;

    [[nodiscard]] std::vector<float> GetVertices() const;
    [[nodiscard]] std::vector<unsigned int> GetTriangles() const;

    BoundingBox& operator=(const BoundingBox& other);
private:
    void SetVertices(const Vector3f &first, const Vector3f &second);
    void SetTriangles();

    std::vector<unsigned int> mFaces;
    std::vector<const float*> mVertices;
    Vector3f mFirst;
    Vector3f mSecond;
    Vector3f mPivot;
};
