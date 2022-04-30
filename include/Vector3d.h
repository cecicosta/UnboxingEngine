#ifndef VECT3D_H
#define VECT3D_H

#include <vector>
class Vector3d {

    std::vector<float> values;
public:
    float& x;
    float& y;
    float& z;

    Vector3d();

    Vector3d(float x, float y, float z);

    explicit Vector3d(const float array_values[]);

    Vector3d(const Vector3d &v);

    friend Vector3d operator+(const Vector3d & v1, const Vector3d & v2);

    friend Vector3d operator-(const Vector3d & v1, const Vector3d & v2);

    friend Vector3d operator*(const Vector3d & v, float escalar);

    friend Vector3d operator*(float escalar, const Vector3d & v);

    friend Vector3d operator/(const Vector3d & v, float escalar);

    Vector3d & operator=(const Vector3d & v);

    float Length() const;

    Vector3d Normalized() const;

    float DotProduct(const Vector3d & vet) const;

    Vector3d CrossProduct(const Vector3d & B) const;

    const float *ToArray() const;
};

#endif//VECT3D_H
