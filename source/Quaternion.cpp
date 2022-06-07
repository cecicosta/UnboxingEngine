#include "Quaternion.h"

#include "UVector.h"

#include <cmath>

constexpr static float s_PI = 3.14159265359;

Quaternion::Quaternion() = default;

Quaternion::Quaternion(const Vector3f &v)
    : Vector3f(v)
    , w(0) {}

Quaternion::Quaternion(const Vector2f &v)
    : Vector3f(v.x, v.y, 0)
    , w(0) {}

Quaternion::Quaternion(float angle, Vector3f axi) {
    float angle_rad = s_PI * angle / 180.0f;

    axi = axi.Normalized();
    axi = axi * sinf(angle_rad / 2.f);

    w = cosf(angle_rad / 2.f);
    x = axi.x;
    y = axi.y;
    z = axi.z;
}

Quaternion::Quaternion(float angle, const Vector2f &axi)
    : Quaternion(angle, Vector3f(axi.x, axi.y, 0)) {}

Quaternion::Quaternion(float angle, const Vector3<int> &axi)
    : Quaternion(angle, Vector3f(static_cast<float>(axi.x), static_cast<float>(axi.y), static_cast<float>(axi.z))) {}

Quaternion::Quaternion(float angle, const Vector2<int> &axi)
    : Quaternion(angle, Vector2f(static_cast<float>(axi.x), static_cast<float>(axi.y))) {}

Quaternion::Quaternion(float w, float x, float y, float z) {
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
}

Quaternion operator+(const Quaternion &q1, const Quaternion &q2) {
    Quaternion novo;
    novo.w = q1.w + q2.w;
    novo.x = q1.x + q2.x;
    novo.y = q1.y + q2.y;
    novo.z = q1.z + q2.z;
    return novo;
}

Quaternion operator-(const Quaternion &q1, const Quaternion &q2) {
    Quaternion novo;
    novo.w = q1.w - q2.w;
    novo.x = q1.x - q2.x;
    novo.y = q1.y - q2.y;
    novo.z = q1.z - q2.z;
    return novo;
}

Quaternion operator*(const Quaternion &v, float t) {
    Quaternion novo;
    novo.w = v.w * t;
    novo.x = v.x * t;
    novo.y = v.y * t;
    novo.z = v.z * t;

    return novo;
}

Quaternion operator*(float escalar, const Quaternion &v) {
    Quaternion novo;
    novo.w = v.w * escalar;
    novo.x = v.x * escalar;
    novo.y = v.y * escalar;
    novo.z = v.z * escalar;

    return novo;
}

Quaternion operator*(const Quaternion &q1, const Quaternion &q2) {
    Vector3f v1(q1.x, q1.y, q1.z);
    Vector3f v2(q2.x, q2.y, q2.z);
    float w = q1.w * q2.w - v1.DotProduct(v2);
    Vector3f v = q1.w * v2 + q2.w * v1 + v1.CrossProduct(v2);

    return {w, v.x, v.y, v.z};
}

Quaternion operator/(Quaternion v, float escalar) {
    return v * (1 / escalar);
}

float Quaternion::Length() {
    return sqrt(w * w + x * x + y * y + z * z);
}

Quaternion Quaternion::conjugado() {
    return {w, -x, -y, -z};
}

Quaternion Quaternion::normalizado() {
    float norma = sqrtf(w * w + x * x + y * y + z * z);
    return {w / norma, x / norma, y / norma, z / norma};
}

Quaternion Quaternion::inverso() {
    float pInt = w * w + x * x + y * y + z * z;

    if (pInt == 0) return {0, 0, 0, 0};

    return {w / pInt, -x / pInt, -y / pInt, -z / pInt};
}

//Retorna o angulo de rotação em um determinado eixo
Vector3f Quaternion::Euler() {
    float PI = 3.1415;
    Vector3f ang;
    float test = x * y + z * w;
    if (test > 0.499) {// singularity at north pole
        ang.y = 2 * atan2(x, w);
        ang.z = PI / 2;
        ang.x = 0;
        return ang;
    }
    if (test < -0.499) {// singularity at south pole
        ang.y = -2 * atan2(x, w);
        ang.z = -PI / 2;
        ang.x = 0;
        return ang;
    }
    float sqx = x * x;
    float sqy = y * y;
    float sqz = z * z;
    ang.y = atan2(2 * y * w - 2 * x * z, 1 - 2 * sqy - 2 * sqz);
    ang.z = asin(2 * test);
    ang.x = atan2(2 * x * w - 2 * y * z, 1 - 2 * sqx - 2 * sqz);
    return ang;
}

