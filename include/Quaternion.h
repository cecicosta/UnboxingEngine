#pragma once
#include "UVector.h"

class Quaternion : public Vector3f {
public:
    float w{};

    Quaternion();
    explicit Quaternion(const Vector3f &v);
    explicit Quaternion(const Vector2f &v);
    Quaternion(float w, float x, float y, float z);

    Quaternion(float angle, Vector3f axi);
    Quaternion(float angle, const Vector3<int>& axi);
    Quaternion(float angle, const Vector2f & axi);
    Quaternion(float angle, const Vector2<int>& axi);

    friend Quaternion operator+(const Quaternion &v1, const Quaternion &v2);

    friend Quaternion operator-(const Quaternion &v1, const Quaternion &v2);

    friend Quaternion operator*(const Quaternion &v, float escalar);

    friend Quaternion operator*(float escalar, const Quaternion &v);

    friend Quaternion operator*(const Quaternion &q1, const Quaternion &q2);

    friend Quaternion operator/(Quaternion v, float escalar);

    float Length();

    Quaternion conjugado();

    Quaternion normalizado();

    Quaternion inverso();

    //Retorna os angulos de rotação de euler
    Vector3f Euler();
};
