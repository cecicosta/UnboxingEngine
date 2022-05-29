#pragma once
#include "Matrix.h"

namespace MathUt {

/// Factors a matrix as the product of a lower triangular matrix and an upper triangular matrix
/// (see matrix decomposition). The product sometimes includes a permutation matrix as well. LU
/// decomposition can be viewed as the matrix form of Gaussian elimination. It is also a key step
/// when inverting a matrix or computing the determinant of a matrix. (wikipedia)
/// \tparam T Matrix elements type.
/// \tparam R Number of row.
/// \tparam C Number of columns
/// \param m Matrix to be factorized/decomposed
/// \param L Lower triangular matrix output result of the method application.
/// \param U Upper triangular matrix output result of the method application.
/// \return true if the factorization was successful, false if it was not possible.
template<typename T, int R, int C>
bool LUDecomposition(Matrix<T, R, C> m, Matrix<T, R, C> &L, Matrix<T, R, C> &U);

///
/// \tparam T Matrix elements type.
/// \tparam R Number of row.
/// \tparam C Number of columns
/// \param scalar
/// \param constant
/// \param solution
/// \return
template<typename T, int R, int C>
bool GaussJordan(const Matrix<T, R, C> &scalar, const Matrix<T, R, C> &constant, Matrix<T, R, C> &solution);


template<typename T, int R, int C>
bool pivot(Matrix<T, R, C> &m, int pivoting_row);
}// namespace MathUt
