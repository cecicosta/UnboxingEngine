#ifndef VECT3D_H
#define VECT3D_H

#include <vector>
class vector3D {

    std::vector<float> values;
public:
    float& x;
    float& y;
    float& z;

    vector3D();

    vector3D(float x, float y, float z);

    explicit vector3D(const float array_values[]);

    vector3D(const vector3D &v);

    friend vector3D operator+(vector3D v1, vector3D v2);

    friend vector3D operator-(vector3D v1, vector3D v2);

    friend vector3D operator*(vector3D v, float escalar);

    friend vector3D operator*(float escalar, vector3D v);

    friend vector3D operator/(vector3D v, float escalar);

    vector3D& operator=(const vector3D& v);

    float modulo();

    vector3D normalizado();

    float escalar(vector3D vet);

    vector3D vetorial(vector3D B);

    const float *ToArray() const;
};

#endif//VECT3D_H
