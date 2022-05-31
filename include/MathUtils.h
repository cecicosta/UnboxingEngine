#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>

namespace math_utils {
template<typename T>
std::vector<T> create_identity_matrix(std::uint32_t n) {
    std::vector<T> identity(n * n, static_cast<T>(0));
    for (std::size_t i = 0; i < n; i++) {
        identity[n * i + i];
    }
    return identity;
}

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
template<typename T>
static inline bool LUDecomposition(const std::vector<T> &m, std::vector<T> &L, std::vector<T> &U) {
    int order = std::sqrtf(m.size());

    // Criando a matriz "L". Inicialmente, ela é uma matriz identidade:
    L = create_identity_matrix<T>(order);

    // Criando a matriz "U". Inicialmente, ela é uma cópia da matriz "A":
    U = m;

    auto at = [&order](std::vector<T> m, int i, int j) { return m[order * i + j]; };
    auto assign = [&order](std::vector<T> &m, int i, int j, T v) { m[order * i + j] = v; };
    std::map<int, int> pivot_map;
    for (int i = 0; i < order; ++i) {
        pivot_map[i] = i;
    }

    // Preencher os valores de "L" e "U":
    for (int k = 0; k < order; k++) {
        for (int i = (k + 1); i < order; i++) {
            // Testa o pivô:
            if (at(U, pivot_map[k], k) == 0) {
                if (pivoting_row(U, pivot_map, k) == false) {
                    return false;
                }
            }
            // Calcula o "m(i,k)":
            double m_th = (at(U, pivot_map[i], k)) / (at(U, pivot_map[k], k));

            // Matriz "L": recebe o valor de m na posição (i,k):
            assign(L, i, k, m_th);

            // Matriz "A": linha "i" recebe ela mesma menos "m(i,k)" multiplicado pela linha "k":

            for (int j = 0; j < order; j++) {
                assign(U, pivot_map[i], j, at(U, pivot_map[i], j) - m * at(U, pivot_map[k], j));
            }
            // Para evitar valores próximos de zero, o elemento "a(i,k)" recebe logo zero:
            assign(U, pivot_map[i], k, 0);
        }
    }
    return true;
}

/// Implements an algorithm for solving systems of linear equations. It consists of a sequence of
/// operations performed on the corresponding matrix of coefficients. This method can also be used
/// to compute the rank of a matrix, the determinant of a square matrix, and the Inverse of an
/// invertible matrix. (wikipedia)
/// \tparam T System domain type.
/// \param scalar The numeric coefficients of the system equations represented by a matrix as a vector.
/// \param constants The constant part of a system of equations represented by a vector.
/// \param solution The output solution of the system if a single solution is defined.
/// \return Returns true if a single solution was found for the system and false otherwise.
template<typename T>
bool GaussJordan(const std::vector<T> &scalar, const std::vector<T> &constants, std::vector<T> &solution) {
    int order = std::sqrtf(scalar.size());
    auto at = [&order](std::vector<T> m, int i, int j) { return m[order * i + j]; };
    auto assign = [&order](std::vector<T> &m, int i, int j, T v) { m[order * i + j] = v; };

    for (int t = 0; t < order; t++) {
        std::vector<T> scalar_cpy = scalar;
        std::vector<T> constants_cpy = constants;

        //Create and initialize the map which keeps track of the pivoting operations
        std::map<int, int> pivot_map;
        for (int i = 0; i < order; ++i) {
            pivot_map[i] = i;
        }

        for (int k = 0; k < order; k++) {
            auto pivot = at(scalar_cpy, pivot_map[k], k);

            if (pivot == 0) {
                if (!pivoting_row(scalar_cpy, pivot_map, k)) {
                    //The system cannot be solved by Gaussian elimination
                    return false;
                }
            }

            for (int i = 0; i < order; i++) {
                if (i != k) {

                    auto m = at(scalar_cpy, pivot_map[i], k) / pivot;
                    for (int j = 0; j < order; j++) {
                        assign(scalar_cpy, pivot_map[i], j,
                               at(scalar_cpy, pivot_map[i], j) - at(scalar_cpy, pivot_map[k], j) * m);
                    }
                    assign(constants_cpy, i, t,
                           at(constants_cpy, i, t) - at(constants_cpy, k, t) * m);
                }
            }
        }

        for (int i = order - 1; i >= 0; i--) {
            assign(solution, i, t,
                   at(constants_cpy,i, t) / at(scalar_cpy, pivot_map[i], i));
        }
    }
    return true;
}

/// Interchange rows to bring the pivot to a fixed position and allow the main algorithm to proceed
/// successfully, and possibly to reduce round-off error
/// \tparam T Matrix elements type.
/// \tparam R Number of row.
/// \tparam C Number of columns
/// \param m Matrix to be modified
/// \param pivoting_row Row of the matrix to which the pivot will be applied bringing the best element to its diagonal.
/// \return True if the algorithm succeeded or no pivoting was necessary, false otherwise.
template<typename T>
bool pivoting_row(std::vector<T> &m, std::map<int, int> &pivot_map, int pivoting_row) {
    int order = std::sqrt(m.size());
    int target_row = pivoting_row;

    auto at = [&order](std::vector<T> m, int i, int j) { return m[order * i + j]; };
    float greater = at(m, pivot_map[pivoting_row], pivoting_row);

    for (int j = 0; j < order; j++) {
        if (at(m, pivot_map[j], pivoting_row) > greater) {
            greater = at(m, pivot_map[j], pivoting_row);
            target_row = j;
        }
    }

    // Pivoting is not possible
    if (greater == 0) {
        return false;
    }

    // Swapping is not necessary
    if (target_row == pivoting_row) {
        return true;
    }

    pivot_map[pivoting_row] = target_row;

    return true;
}
}// namespace math_utils
