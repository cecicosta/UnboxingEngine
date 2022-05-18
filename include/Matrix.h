#pragma once

#include "Quaternion.h"
#include "Vector3Df.h"

#include <cstdint>

template<typename T, int Rows, int Columns = Rows>
class Matrix {
    friend class Matrix<T, 3, 3>;
    friend class Matrix<T, 4, 4>;

public:
    Matrix();

    Matrix(const std::initializer_list<T> &matrix);

    explicit Matrix(const Quaternion &q);

    virtual ~Matrix() = default;

    /// Get matrix element reference from array coordinate i
    /// \param i array coordinate of the matrix element to be returned
    /// \return element reference at array coordinates i
    T &at(std::uint32_t i);

    /// Get matrix element reference from coordinate i (row), j (column)
    /// \param i row of the matrix element to be returned
    /// \param j column of the matrix element to be returned
    /// \return element reference at coordinates i, j
    T &at(std::uint32_t i, std::uint32_t j);

    /// Get matrix const element reference from coordinate i (row), j (column)
    /// \param i row of the matrix element to be returned
    /// \param j column of the matrix element to be returned
    /// \return const element reference at coordinates i, j
    [[nodiscard]] const T &at(std::uint32_t i, std::uint32_t j) const;

    /// Created a new matrix from the current one with all elements of row_a and row_b swapped
    /// \param row_a row to be swapped with row_b
    /// \param row_b row to be swapped with row_a
    /// \return New matrix with swapped rows
    [[nodiscard]] Matrix swap_rows(int row_a, int row_b) const;

    /// Calculates the transpose of the current matrix
    /// \return the calculated transpose of the current matrix
    [[nodiscard]] Matrix transpose() const;

    /// Calculates the inverse of the current matrix
    /// \return the inverse of the current matrix
    Matrix inverse();

    static Matrix<T, Rows, Columns> Identity();

    float const *ToArray();

    Matrix<T, Rows, Columns> &operator=(const Matrix<T, Rows, Columns> &m);

    template<int R, int C>
    Matrix<T, Rows, Columns> &operator=(const Matrix<T, R, C> &m);

    Matrix<T, Rows, Columns> operator+(const Matrix<T, Rows, Columns> &m) const;

    Matrix<T, Rows, Columns> operator-(const Matrix<T, Rows, Columns> &m) const;

    /// Creates a new matrix from the multiplication of the current one by a second matrix
    /// \tparam OutC number of columns in the output and right side input matrix
    /// \param right operation right side matrix matching the requirement for number
    /// of rows equals the number of columns of the operation left side matrix
    /// \return Resulting matrix with number of rows equals left side matrix rows and number of
    /// columns equals right side matrix columns
    template<int ResultColumns>
    Matrix<T, Rows, ResultColumns> operator*(Matrix<T, Columns, ResultColumns> right) const;

    Vector<T, Columns - 1> operator*(const Vector<T, Columns - 1> &v) const;

    Matrix operator*(float a) const;

    T trace();

    template<int R, int C>
    Matrix<T, Rows, Columns> MergeMatrix(const Matrix<T, R, C> &m);

    static Matrix TranslationMatrix(const Vector2D<T> &translation);
    static Matrix TranslationMatrix(const Vector3D<T> &translation);

    static Matrix ScaleMatrix(const Vector2D<T> &scale);
    static Matrix ScaleMatrix(const Vector3D<T> &scale);

    static Matrix RotationMatrix(const Vector3D<T> &axis, float angle);
    static Matrix RotationMatrix(const Vector2D<T> &axis, float angle);

    static Matrix RotationMatrix(const Quaternion &q);


    inline constexpr size_t array_size() {
        return Rows * Columns;
    }

    const uint32_t rows = Rows;
    const uint32_t cols = Columns;

protected:
    float A[Rows * Columns]{};

    /// Method to complete the Faddeev-Leverrier recursion.
    /// \param m
    /// \param c
    /// \return
    std::vector<Matrix<T, Rows>> FaddeevLeVerrierRecursion(Matrix<T, Rows> m, float c[]) {
        const int n = m.rows;
        std::vector<Matrix<T, Rows>> solution(n);
        solution[0] = Identity();

        for (int k = 1; k < n; ++k) {
            solution[k] = m * solution[k - 1];
            c[n - k] = -solution[k].trace() / k;
            for (int i = 0; i < n; ++i) {
                solution[k].at(i, i) += c[n - k];
            }
        }
        auto t = m * solution[n - 1];
        c[0] = -t.trace() / n;
        return solution;
    }
};

template<typename T>
using Matrix3 = Matrix<T, 3, 3>;
template<typename T>
using Matrix4 = Matrix<T, 4, 4>;

class Matrix3f : public Matrix<float, 3, 3> {
public:
    Matrix3f()
        : Matrix() {}
    Matrix3f(std::initializer_list<float> arr) : Matrix(arr) {}

    explicit Matrix3f(const Quaternion& q) : Matrix(q) {}

    Matrix3f(const Matrix<float, 3, 3>& matrix) : Matrix(matrix){} //NOLINT(google-explicit-constructor)

    explicit Matrix3f(const Vector<float, 2> &v) {
        at(0, 2) = v.x;
        at(1, 2) = v.y;
        at(2, 2) = 1;
    }
};

class Matrix4f : public Matrix<float, 4, 4> {
public:
    Matrix4f()
        : Matrix() {}
    Matrix4f(std::initializer_list<float> arr) : Matrix(arr) {}

    explicit Matrix4f(const Quaternion& q) : Matrix(q) {}

    Matrix4f(const Matrix<float, 4, 4>& matrix) : Matrix(matrix) {} //NOLINT(google-explicit-constructor)

    explicit Matrix4f(const Vector<float, 3> &v) {
        at(0, 3) = v.x;
        at(1, 3) = v.y;
        at(2, 3) = v.z;
        at(3, 3) = 1;
    }
};