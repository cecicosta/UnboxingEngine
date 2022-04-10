#include "vector3D.h"
#include <math.h>

vector3D::vector3D() : vector3D(0, 0, 0) {}
vector3D::vector3D(float x, float y, float z)
    : values({x, y, z}), x(values[0]), y(values[1]), z(values[2]) {}

vector3D::vector3D(const float array_values[]) : vector3D(array_values[0], array_values[1], array_values[2]) {}

vector3D::vector3D(const vector3D &v) : vector3D(v.ToArray()) { }

vector3D operator+(vector3D v1, vector3D v2) {
    vector3D novo;
    novo.x = v1.x + v2.x;
    novo.y = v1.y + v2.y;
    novo.z = v1.z + v2.z;
    return novo;
}

vector3D operator-(vector3D v1, vector3D v2) {
    vector3D novo;
    novo.x = v1.x - v2.x;
    novo.y = v1.y - v2.y;
    novo.z = v1.z - v2.z;
    return novo;
}

vector3D operator*(vector3D v, float escalar) {
    vector3D novo;
    novo.x = v.x * escalar;
    novo.y = v.y * escalar;
    novo.z = v.z * escalar;

    return novo;
}

vector3D operator*(float escalar, vector3D v) {
    vector3D novo;
    novo.x = v.x * escalar;
    novo.y = v.y * escalar;
    novo.z = v.z * escalar;

    return novo;
}

vector3D operator/(vector3D v, float escalar) {
    return v * (1 / escalar);
}

float vector3D::modulo() {
    return sqrt(x * x + y * y + z * z);
}

vector3D vector3D::normalizado() {
    float norma = sqrt(x * x + y * y + z * z);
    return vector3D(x / norma, y / norma, z / norma);
}

float vector3D::escalar(vector3D vet) {
    return x * vet.x + y * vet.y + z * vet.z;
}

vector3D vector3D::vetorial(vector3D B) {
    vector3D prod;
    prod.x = y * B.z - z * B.y;
    prod.y = z * B.x - x * B.z;
    prod.z = x * B.y - y * B.x;

    return prod;
}
const float *vector3D::ToArray() const {
    return &values[0];
}

vector3D& vector3D::operator=(const vector3D& v) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
}
