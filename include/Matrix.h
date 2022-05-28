#pragma once

#include "Quaternion.h"
#include "Vector3Df.h"

#include <cstdint>

template<typename T, int Rows, int Columns = Rows>
class Matrix {

public:
    Matrix() {
        std::fill(A, A + Rows * Columns, static_cast<T>(0));
    }

    /// Creates a Matrix from a list of values
    /// \param matrix initialization list with size equal to the number of rows x columns the Matrix will have, and of corresponding type
    Matrix(const std::initializer_list<T> &matrix) {
        memcpy_s(A, Rows * Columns * sizeof(T), data(matrix), matrix.size() * sizeof(T));
    }

    /// Creates a 2D translation Matrix from Vector<T, 2>
    /// \param v Position relative to the origin to which a translation Matrix will be created
    explicit Matrix(const Vector<float, 2> &v) {
        static_assert(Rows == 3 && Columns == 3, "Cannot construct translation matrix from Vector2D. Only type Matrix<T, 3, 3> is compatible.");
        at(0, 2) = v.x;
        at(1, 2) = v.y;
        at(2, 2) = 1;
        at(1, 1) = 1;
        at(0, 0) = 1;
    }

    /// Creates a 3D translation Matrix from Vector<T, 3>
    /// \param v Position relative to the origin to which a translation Matrix will be created
    explicit Matrix(const Vector<float, 3> &v) {
        static_assert(Rows == 4 && Columns == 4, "Cannot construct translation matrix from Vector3D. Only type Matrix<T, 4, 4> is compatible.");
        at(0, 3) = v.x;
        at(1, 3) = v.y;
        at(2, 3) = v.z;
        at(3, 3) = 1;
        at(2, 2) = 1;
        at(1, 1) = 1;
        at(0, 0) = 1;
    }

    /// Creates a rotation Matrix from a quaternion
    /// \param q Normalized quaternion representing the rotation
    explicit Matrix(const Quaternion &q) {
        float x2 = q.x * q.x;
        float y2 = q.y * q.y;
        float z2 = q.z * q.z;
        float xy = q.x * q.y;
        float xz = q.x * q.z;
        float yz = q.y * q.z;
        float wx = q.w * q.x;
        float wy = q.w * q.y;
        float wz = q.w * q.z;

        *this = Identity();
        at(0, 0) = 1.0f - 2.0f * (y2 + z2);
        at(0, 1) = 2.0f * (xy - wz);
        at(0, 2) = 2.0f * (xz + wy);
        at(1, 0) = 2.0f * (xy + wz);
        at(1, 1) = 1.0f - 2.0f * (x2 + z2);
        at(1, 2) = 2.0f * (yz - wx);
        at(2, 0) = 2.0f * (xz - wy);
        at(2, 1) = 2.0f * (yz + wx);
        at(2, 2) = 1.0f - 2.0f * (x2 + y2);
    }

    /// Constructs a copy of a matrix converting its elements to a different type
    /// \tparam U Arithmetic type of the original matrix
    /// \param m Original matrix from which the copy will be created converting its elements from T to U
    template<typename U>
    //NOLINTNEXTLINE(google-explicit-constructor)
    Matrix(const Matrix<U, Rows, Columns> &m) {
        static_assert(std::is_convertible<U, T>::value, "Matrix");
        for (int j = 0; j < array_size(); j++) {
            at(j) = static_cast<T>(m.at(j));
        }
    }

    virtual ~Matrix() = default;

    /// Get matrix element reference from array coordinate i
    /// \param i array coordinate of the matrix element to be returned
    /// \return element reference at array coordinates i
    T &at(std::uint32_t i) {
        return A[i];
    }

    /// Get a const reference to a element from the Matrix at the array index i
    /// \param i Array index of the a Matrix element
    /// \return The element of type T referred by the array index i
    [[nodiscard]] const T &at(std::uint32_t i) const {
        return A[i];
    }

    /// Get matrix element reference from coordinate i (row), j (column)
    /// \param i row of the matrix element to be returned
    /// \param j column of the matrix element to be returned
    /// \return element reference at coordinates i, j
    T &at(std::uint32_t i, std::uint32_t j) {
        return A[i * rows + j % cols];
    }

    /// Get matrix const element reference from coordinate i (row), j (column)
    /// \param i row of the matrix element to be returned
    /// \param j column of the matrix element to be returned
    /// \return const element reference at coordinates i, j
    [[nodiscard]] const T &at(std::uint32_t i, std::uint32_t j) const {
        return A[i * rows + j % cols];
    }

    /// Created a new matrix from the current one with all elements of row_a and row_b swapped
    /// \param row_a row to be swapped with row_b
    /// \param row_b row to be swapped with row_a
    /// \return New matrix with swapped rows
    [[nodiscard]] Matrix swap_rows(int row_a, int row_b) const {
        Matrix swapped(*this);
        if (row_a >= 0 && row_a < rows && row_b >= 0 && row_b < rows) {
            float swap;
            for (int k = 0; k < rows; k++) {
                swap = swapped.at(row_a, k);
                swapped.at(row_a, k) = swapped.at(row_b, k);
                swapped.at(row_b, k) = swap;
            }
        }
        return swapped;
    }

    /// Calculates the transpose of the current matrix
    /// \return the calculated transpose of the current matrix
    [[nodiscard]] Matrix transpose() const {
        Matrix<T, Rows, Columns> m;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                m.at(j, i) = at(i, j);
            }
        }
        return m;
    }

    /// Calculates the inverse of the current matrix
    /// \return the inverse of the current matrix
    Matrix inverse() {
        Matrix sol = Identity();
        Matrix<T, Rows, Columns> inverse;
        //    MathUt::GaussJordan(*this, sol, inverse);
        return inverse;
    }

    /// Create a identity matrix. The identity is the base for any transformation.
    /// \return An identity matrix. Which by definition have all elements on its main diagonal equal 1.
    static Matrix<T, Rows, Columns> Identity() {
        Matrix<T, Rows, Columns> identity;
        for (int i = 0; i < Rows; i++) {
            for (int j = 0; j < Columns; j++) {
                identity.at(i, j) = i == j ? 1 : 0;
            }
        }

        return identity;
    }

    /// Get a read only array reference containing the matrix elements.
    /// \return The Matrix as an read only array.
    T const *ToArray() {
        return A;
    }

    /// Get the matrix trace. By definition the sum of the elements of its main diagonal.
    /// \return Matrix trace value
    [[nodiscard]] T trace() {
        T trace = 0;
        for (int i = 0; i < Rows; ++i) {
            trace += at(i, i);
        }
        return trace;
    }

    ///
    /// \tparam R
    /// \tparam C
    /// \param m
    /// \return
    template<int R, int C>
    Matrix<T, Rows, Columns> MergeMatrix(const Matrix<T, R, C> &m) {
        Matrix<T, Rows, Columns> result = *this;
        for (int i = 0; i < R; ++i) {
            for (int j = 0; j < C; ++j) {
                result.at(i, j) = i < Rows && j < Columns ? m.at(i, j) : this->at(i, j);
            }
        }
        return result;
    }

    ///
    /// \param translation
    /// \return
    static inline Matrix TranslationMatrix(const Vector2D<T> &translation) {
        auto m = Matrix<T, Columns>::Identity();
        for (int i = 0; i < 2; ++i) {
            m.at(i, Columns - 1) = translation.ToArray()[i];
        }
        return m;
    }

    ///
    /// \param translation
    /// \return
    static inline Matrix TranslationMatrix(const Vector3D<T> &translation) {
        auto m = Matrix<T, Columns>::Identity();
        for (int i = 0; i < 3; ++i) {
            m.at(i, Columns - 1) = translation.ToArray()[i];
        }
        return m;
    }

    ///
    /// \param scale
    /// \return
    static inline Matrix ScaleMatrix(const Vector2D<T> &scale) {
        auto m = Matrix<T, Columns>::Identity();
        for (int i = 0; i < 2; ++i) {
            m.at(i, i) = scale.ToArray()[i];
        }
        return m;
    }

    ///
    /// \param scale
    /// \return
    static inline Matrix ScaleMatrix(const Vector3D<T> &scale) {
        auto m = Matrix<T, Columns>::Identity();
        for (int i = 0; i < 3; ++i) {
            m.at(i, i) = scale.ToArray()[i];
        }
        return m;
    }

    ///
    /// \param axis
    /// \param angle
    /// \return
    static inline Matrix RotationMatrix(const Vector3D<T> &axis, float angle) {
        Quaternion q(angle, axis);
        return Matrix<T, Rows, Columns>(q);
    }

    ///
    /// \param axis
    /// \param angle
    /// \return
    static inline Matrix RotationMatrix(const Vector2D<T> &axis, float angle) {
        Quaternion q(angle, axis);
        return Matrix<T, Rows, Columns>(q);
    }

    ///
    /// \param q
    /// \return
    static inline Matrix RotationMatrix(const Quaternion &q) {
        return Matrix<T, Rows, Columns>(q);
    }

    [[nodiscard]] inline constexpr size_t array_size() const {
        return Rows * Columns;
    }


    Matrix<T, Rows, Columns> &operator=(const Matrix<T, Rows, Columns> &m) {
        if (this != &m) {
            size_t size = Rows * Columns * sizeof(T);
            memcpy_s(A, size, m.A, size);
        }
        return *this;
    }

    template<int R, int C>
    Matrix<T, Rows, Columns> &operator=(const Matrix<T, R, C> &m) {
        static_assert((R < Rows) || (C < Columns), "Unable to assign matrix of higher dimensions to a matrix of lower dimensions.");
        size_t size = Rows * Columns * sizeof(T);
        memcpy_s(A, size, m.A, R * C * sizeof(T));
        return *this;
    }

    Matrix<T, Rows, Columns> operator+(const Matrix<T, Rows, Columns> &m) const {
        if (rows == m.rows && cols == m.cols) {
            Matrix<T, Rows, Columns> sum;
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    sum.at(i, j) = at(i, j) + m.at(i, j);
                }
            }
            return sum;
        }
        return {};
    }

    Matrix<T, Rows, Columns> operator-(const Matrix<T, Rows, Columns> &m) const {
        Matrix<T, Rows, Columns> sub;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                sub.at(i, j) = at(i, j) - m.at(i, j);
            }
        }
        return sub;
    }

    Vector<T, Columns - 1> operator*(const Vector<T, Columns - 1> &v) const {
        T res[Columns - 1];
        for (int i = 0; i < Columns; ++i) {
            T soma = this->at(i, Columns);
            for (int j = 0; j < Columns; ++j) {
                soma += this->at(i, j) * v.ToArray()[j];
            }
            res[i] = soma;
        }
        return Vector<T, Columns - 1>(res);
    }

    /// Creates a new matrix from the multiplication of the current one by a second matrix
    /// \tparam OutC number of columns in the output and right side input matrix
    /// \param right operation right side matrix matching the requirement for number
    /// of rows equals the number of columns of the operation left side matrix
    /// \return Resulting matrix with number of rows equals left side matrix rows and number of
    /// columns equals right side matrix columns
    template<int ResultColumns>
    Matrix<T, Rows, ResultColumns> operator*(Matrix<T, Columns, ResultColumns> right) const {
        float sum;
        Matrix<T, Rows, ResultColumns> product;
        for (int i = 0; i < right.cols; i++) {
            for (int j = 0; j < rows; j++) {
                sum = 0;
                for (int k = 0; k < cols; k++) {
                    sum = sum + at(j, k) * right.at(k, i);
                }
                product.at(j, i) = sum;
            }
        }
        return product;
    }

    Matrix operator*(float a) const {
        Matrix<T, Rows, Columns> result;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                result.at(i, j) = at(i, j) * a;
            }
        }
        return result;
    }

    bool operator==(const Matrix<T, Rows, Columns> &m) const {
        return memcmp(A, m.A, array_size() * sizeof(T)) == 0;
    }


    const uint32_t rows = Rows;
    const uint32_t cols = Columns;

private:
    T A[Rows * Columns]{};

//    /// Method to complete the Faddeev-Leverrier recursion.
//    /// \param m
//    /// \param c
//    /// \return
//    std::vector<Matrix<T, Rows>> FaddeevLeVerrierRecursion(Matrix<T, Rows> m, T c[]) {
//        const int n = m.rows;
//        std::vector<Matrix<T, Rows>> solution(n);
//        solution[0] = Identity();
//
//        for (int k = 1; k < n; ++k) {
//            solution[k] = m * solution[k - 1];
//            c[n - k] = -solution[k].trace() / k;
//            for (int i = 0; i < n; ++i) {
//                solution[k].at(i, i) += c[n - k];
//            }
//        }
//        auto t = m * solution[n - 1];
//        c[0] = -t.trace() / n;
//        return solution;
//    }
};

using Matrix3i = Matrix<int, 3, 3>;
using Matrix3f = Matrix<float, 3, 3>;

using Matrix4i = Matrix<int, 4, 4>;
using Matrix4f = Matrix<float, 4, 4>;

//Matrix Matrix::inverse(){
//
//    int n = order;
//    float c[n];
//    Matrix d(order);
//    Matrix *s = fl(*this, c);
//    for (int i=0; i<n; ++i)
//      for (int j=0; j<n; ++j)
//        d.index(i,j) = -s[n-1].at(i,j)/c[0];
//}
//
//// Method to complete the Faddeev-Leverrier recursion.
//
//Matrix *Matrix::fl(Matrix a, float c[]) {
//    int n = a.order;
//    Matrix s[order];
//    for (int i=0; i<n; ++i)
//        s[i] = Matrix(order);
//
//    for (int i=0; i<n; ++i) s[0].at(i,i) = 1;
//    for (int k=1; k<n; ++k) {
//      s[k] =a*s[k-1];
//      c[n-k] = -tr(a*s[k-1])/k;
//      for (int i=0; i<n; ++i)
//      s[k].at(i,i) += c[n-k];
//    }
//      c[0] = -tr(a*s[n-1])/n;
//    return s;
//}
