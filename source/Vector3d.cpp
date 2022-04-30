#include "Vector3d.h"
#include <cmath>

Vector3d::Vector3d() : Vector3d(0, 0, 0) {}
Vector3d::Vector3d(float x, float y, float z)
    : values({x, y, z}), x(values[0]), y(values[1]), z(values[2]) {}

Vector3d::Vector3d(const float array_values[]) : Vector3d(array_values[0], array_values[1], array_values[2]) {}

Vector3d::Vector3d(const Vector3d &v) : Vector3d(v.ToArray()) { }

Vector3d operator+(const Vector3d & v1, const Vector3d & v2) {
    Vector3d result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

Vector3d operator-(const Vector3d & v1, const Vector3d & v2) {
    Vector3d result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

Vector3d operator*(const Vector3d & v, float t) {
    Vector3d result;
    result.x = v.x * t;
    result.y = v.y * t;
    result.z = v.z * t;

    return result;
}

Vector3d operator*(float t, const Vector3d & v) {
    Vector3d result;
    result.x = v.x * t;
    result.y = v.y * t;
    result.z = v.z * t;

    return result;
}

Vector3d operator/(const Vector3d & v, float t) {
    return v * (1 / t);
}

float Vector3d::Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3d Vector3d::Normalized() const {
    float norma = std::sqrt(x * x + y * y + z * z);
    return Vector3d;
}

float Vector3d::DotProduct(const Vector3d & vet) const {
    return x * vet.x + y * vet.y + z * vet.z;
}

Vector3d Vector3d::CrossProduct(const Vector3d & B) const {
    Vector3d prod;
    prod.x = y * B.z - z * B.y;
    prod.y = z * B.x - x * B.z;
    prod.z = x * B.y - y * B.x;

    return prod;
}
const float *Vector3d::ToArray() const {
    return &values[0];
}

Vector3d &Vector3d::operator=(const Vector3d & v) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
}
