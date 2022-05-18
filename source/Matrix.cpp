#include "Matrix.h"

#define TEMPLATE_PREDICATE_CLASS template<class T, int Rows, int Columns> \
Matrix<T, Rows, Columns>::
//Compose the template declaration and class identifier with return type given its specifier or qualifier as arguments
#define TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(Q, S) template<typename T, int Rows, int Columns> \
Q Matrix<T, Rows, Columns> S Matrix<T, Rows, Columns>::
//Compose the template declaration and class identifier with the combination of return type, specifiers, qualifiers or other predicate
#define TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(Q, U, S) template<typename T, int Rows, int Columns> \
Q U S Matrix<T, Rows, Columns>::

TEMPLATE_PREDICATE_CLASS Matrix() {
    std::fill(A, A + Rows * Columns, static_cast<T>(0));
}

TEMPLATE_PREDICATE_CLASS Matrix(const Quaternion &q) {
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

TEMPLATE_PREDICATE_CLASS Matrix(const std::initializer_list<T> &matrix) {
    memcpy_s(A, Rows * Columns * sizeof(T), data(matrix), matrix.size() * sizeof(T));
}

TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(, T, &)
at(std::uint32_t i) {
    return A[i];
}

TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(, T, &)
at(std::uint32_t i, std::uint32_t j) {
    return A[i * rows + j % cols];
}

TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(const, T, &)
at(std::uint32_t i, std::uint32_t j) const {
    return A[i * rows + j % cols];
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
swap_rows(int row_a, int row_b) const {
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

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
transpose() const {
    Matrix<T, Rows, Columns> m;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            m.at(j, i) = at(i, j);
        }
    }
    return m;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
inverse() {
    Matrix sol = Identity();
    Matrix<T, Rows, Columns> inverse;
    //    MathUt::GaussJordan(*this, sol, inverse);
    return inverse;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
Identity() {
    Matrix<T, Rows, Columns> identity;
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Columns; j++) {
            identity.at(i, j) = i == j ? 1 : 0;
        }
    }

    return identity;
}

TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(, float, const *)
ToArray() {
    return A;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
operator+(const Matrix<T, Rows, Columns> &m) const {
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

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
operator-(const Matrix<T, Rows, Columns> &m) const {
    Matrix<T, Rows, Columns> sub;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sub.at(i, j) = at(i, j) - m.at(i, j);
        }
    }
    return sub;
}

#define T_RESULT_COLUMNS template<int ResultColumns>
#define T_MATRIX Matrix<T, Rows, ResultColumns>
TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(T_RESULT_COLUMNS, T_MATRIX, )
operator*(Matrix<T, Columns, ResultColumns> right) const {
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
#undef T_MATRIX
#undef T_RESULT_COLUMNS

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
operator*(float a) const {
    Matrix<T, Rows, Columns> result;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result.at(i, j) = at(i, j) * a;
        }
    }
    return result;
}


#define T_Vector Vector<T, Columns - 1>
TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(, T_Vector, )
operator*(const T_Vector &v) const {
    T res[Columns - 1];
    for (int i = 0; i < Columns; ++i) {
        T soma = this->at(i, Columns);
        for (int j = 0; j < Columns; ++j) {
            soma += this->at(i, j) * v.ToArray()[j];
        }
        res[i] = soma;
    }
    return T_Vector(res);
}
#undef T_Vector


TEMPLATE_PREDICATE_ARG0_ARG1_ARG2_CLASS(, T, )
trace() {
    T trace = 0;
    for (int i = 0; i < Rows; ++i) {
        trace += at(i, i);
    }
    return trace;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
TranslationMatrix(const Vector3D<T> &translation) {
    auto m = Matrix<T, Columns>::Identity();
    for (int i = 0; i < 3; ++i) {
        m.at(i, Columns - 1) = translation.ToArray()[i];
    }
    return m;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
TranslationMatrix(const Vector2D<T> &translation) {
    auto m = Matrix<T, Columns>::Identity();
    for (int i = 0; i < 2; ++i) {
        m.at(i, Columns - 1) = translation.ToArray()[i];
    }
    return m;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
ScaleMatrix(const Vector3D<T> &scale) {
    auto m = Matrix<T, Columns>::Identity();
    for (int i = 0; i < 3; ++i) {
        m.at(i, i) = scale.ToArray()[i];
    }
    return m;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
ScaleMatrix(const Vector2D<T> &scale) {
    auto m = Matrix<T, Columns>::Identity();
    for (int i = 0; i < 2; ++i) {
        m.at(i, i) = scale.ToArray()[i];
    }
    return m;
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
RotationMatrix(const Vector3D<T> &axis, float angle) {
    Quaternion q(angle, axis);
    return Matrix<T, Rows, Columns>(q);
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
RotationMatrix(const Vector2D<T> &axis, float angle) {
    Quaternion q(angle, axis);
    return Matrix<T, Rows, Columns>(q);
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, )
RotationMatrix(const Quaternion &q) {
    return Matrix<T, Rows, Columns>(q);
}

TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(, &)
operator=(const Matrix<T, Rows, Columns> &m) {
    if (this != &m) {
        size_t size = Rows * Columns * sizeof(T);
        memcpy_s(A, size, m.A, size);
    }
    return *this;
}

#define T_ARG_MATRIX_PREDICATE template<int R, int C>
TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(T_ARG_MATRIX_PREDICATE, &)
operator=(const Matrix<T, R, C> &m) {
    static_assert((R < Rows) || (C < Columns), "Unable to assign matrix of higher dimensions to a matrix of lower dimensions.");
    size_t size = Rows * Columns * sizeof(T);
    memcpy_s(A, size, m.A, R * C * sizeof(T));
    return *this;
}
#undef T_ARG_MATRIX_PREDICATE

#define T_InputMatrix template<int R, int C>
TEMPLATE_PREDICATE_ARG0_RETURN_ARG1(T_InputMatrix, )
MergeMatrix(const Matrix<T, R, C> &m) {
    Matrix<T, Rows, Columns> result = *this;
    for (int i = 0; i < R; ++i) {
        for (int j = 0; j < C; ++j) {
            result.at(i, j) = i < Rows && j < Columns ? m.at(i, j) : this->at(i, j);
        }
    }
    return result;
}
#undef T_InputMatrix

template class Matrix<float, 3, 3>;
template class Matrix<float, 4, 4>;




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
//
