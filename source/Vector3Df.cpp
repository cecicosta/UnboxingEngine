#include "Vector3Df.h"
#include <cmath>

#define TEMPLATE_PREDICATE_CLASS template<class T, int Axis> \
Vector<T, Axis>::
#define TEMPLATE_PREDICATE_ARG0_RETURN_ARG1 template<class T, int Axis> \
Vector<T, Axis> Vector<T, Axis>::
#define TEMPLATE_PREDICATE_WITH_RETURN_REF template<class T, int Axis> \
Vector<T, Axis> &Vector<T, Axis>::

TEMPLATE_PREDICATE_CLASS Vector()
    : Vector(0, 0, 0) {}
TEMPLATE_PREDICATE_CLASS Vector(T x_, T y_, T z_)
    : values({x_, y_, z_})
    , x(values[0])
    , y(values[1])
    , z(values[2]) {}

TEMPLATE_PREDICATE_CLASS Vector(T x, T y) : Vector(x, y, static_cast<T>(0)) {}

TEMPLATE_PREDICATE_CLASS Vector(const T array_values[])
    : Vector(array_values[0], array_values[1], array_values[2]) {}

TEMPLATE_PREDICATE_CLASS Vector(const Vector<T, Axis> &v)
    : Vector(v.ToArray()) {}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1 operator+(const Vector<T, Axis> &v2) const {
    Vector<T, Axis> result;
    result.x = x + v2.x;
    result.y = y + v2.y;
    result.z = z + v2.z;
    return result;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1 operator-(const Vector<T, Axis> &v2) const {
    Vector<T, Axis> result;
    result.x = x - v2.x;
    result.y = y - v2.y;
    result.z = z - v2.z;
    return result;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1 operator*(T t) const {
    Vector<T, Axis> result;
    result.x = x * t;
    result.y = y * t;
    result.z = z * t;

    return result;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1 operator/(T t) const {
    return *this * (1 / t);
}

TEMPLATE_PREDICATE_WITH_RETURN_REF operator=(const Vector<T, Axis> &v) {
    if (this != &v) {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
    }
    return *this;
}

template<class T, int Axis>
T Vector<T, Axis>::Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1 Normalized() const {
    T length = Length();
    return {x / length, y / length, z / length};
}

template<class T, int Axis>
T Vector<T, Axis>::DotProduct(const Vector<T, Axis> &vet) const {
    return x * vet.x + y * vet.y + z * vet.z;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1 CrossProduct(const Vector<T, Axis> &B) const {
    Vector<T, Axis> prod;
    prod.x = y * B.z - z * B.y;
    prod.y = z * B.x - x * B.z;
    prod.z = x * B.y - y * B.x;

    return prod;
}

template<class T, int Axis>
const T *Vector<T, Axis>::ToArray() const {
    return &values[0];
}

template class Vector<float, 2>;
template class Vector<float, 3>;
template<typename T>
class Vector<T, 2>;
template<typename T>
class Vector<T, 3>;

#undef TEMPLATE_PREDICATE_CLASS
#undef TEMPLATE_PREDICATE_ARG0_RETURN_ARG1
#undef TEMPLATE_PREDICATE_WITH_RETURN_REF
