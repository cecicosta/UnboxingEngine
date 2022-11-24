#pragma once

#include <vector>

template<class T, int Axis>
class Vector {
public:
    Vector();
    virtual ~Vector() = default;

    Vector(T x, T y, T z);

    Vector(T x, T y);

    explicit Vector(const T array_values[]);

    Vector(const Vector<T, Axis> &v);

    Vector<T, Axis> operator+(const Vector<T, Axis> &v2) const;

    Vector<T, Axis> operator-(const Vector<T, Axis> &v2) const;

    Vector<T, Axis> operator*(T t) const;

    friend Vector<T, Axis> operator*(T t, const Vector<T, Axis> &v) {
        return v * t;
    }

    Vector<T, Axis> operator/(T t) const;

    Vector<T, Axis> &operator=(const Vector<T, Axis> &v);

    bool operator==(const Vector<T, Axis> &other) const;

    [[nodiscard]] T Length() const;

    [[nodiscard]] Vector<T, Axis> Normalized() const;

    [[nodiscard]] T DotProduct(const Vector<T, Axis> &vet) const;

    [[nodiscard]] Vector<T, Axis> CrossProduct(const Vector<T, Axis> &B) const;

    [[nodiscard]] const T *ToArray() const;

protected:
    std::vector<T> values;
public:
    T &x;
    T &y;
    T &z;
};

template<typename T>
using Vector2 = Vector<T, 2>;
template<typename T>
using Vector3 = Vector<T, 3>;

using Vector2i = Vector<int, 2>;
using Vector3i = Vector<int, 3>;

using Vector2f = Vector<float, 2>;
using Vector3f = Vector<float, 3>;