#include "MathUtils.h"
#include <iostream>
#include <vector>
using namespace std;

template<typename T, int R, int C>
bool MathUt::LUDecomposition(Matrix<T, R, C> m, Matrix<T, R, C> &L, Matrix<T, R, C> &U) {
    int order = m.order;

    // Criando a matriz "L". Inicialmente, ela é uma matriz identidade:
    L = Matrix<T, R, C>::Identity(order);

    // Criando a matriz "U". Inicialmente, ela é uma cópia da matriz "A":
    U = m;

    // Preencher os valores de "L" e "U":
    for (int k = 0; k < order; k++) {
        for (int i = (k + 1); i < order; i++) {
            // Testa o pivô:
            if (U.at(k, k) == 0) {
                if (pivot(U, k) == false) {
                    cout << ">>> Resolver sistema: o pivo a[" << k << "," << k << "] e ZERO!!!" << endl;
                    return (false);
                }
            }
            // Calcula o "m(i,k)":
            double m = (U.at(i, k)) / (U.at(k, k));


            // Matriz "L": recebe o valor de m na posição (i,k):
            L.at(i, k) = m;

            // Matriz "A": linha "i" recebe ela mesma menos "m(i,k)" multiplicado pela linha "k":

            for (int j = 0; j < order; j++) {
                U.at(i, j) = U.at(i, j) - m * U.at(k, j);
            }
            // Para evitar valores próximos de zero, o elemento "a(i,k)" recebe logo zero:
            U.at(i, k) = 0;
        }
    }
    return (true);
}

template<typename T, int R, int C>
bool MathUt::GaussJordan(const Matrix<T, R, C> &scalar, const Matrix<T, R, C> &constant, Matrix<T, R, C> &solution) {
    const int order = scalar.rows;
    for (int t = 0; t < constant.cols; t++) {

        Matrix<T, R, C> copy = scalar;
        std::vector<T> v_constants(order);
        for (int i = 0; i < order; i++) {
            v_constants[i] = constant.at(i, t);
        }

        for (int k = 0; k < order; k++) {
            auto pivot = copy.at(k, k);

            if (pivot == 0) {
                if (!pivot(copy, k)) {
                    //The system cannot be solved by Gaussian elimination
                    return false;
                }
            }

            for (int i = 0; i < order; i++) {
                if (i != k) {

                    auto m = copy.at(i, k) / pivot;
                    for (int j = 0; j < order; j++) {
                        copy.at(i, j) = copy.at(i, j) - copy.at(k, j) * m;
                    }
                    v_constants[i] = v_constants[i] - v_constants[k] * m;
                }
            }
        }

        for (int i = order - 1; i >= 0; i--) {
            solution.at(i, t) = (v_constants[i]) / copy.at(i, i);
        }
    }
    return true;
}

template<typename T, int R, int C>
bool MathUt::pivot(Matrix<T, R, C> &m, int pivoting_row) {

    float greater = m.at(pivoting_row, pivoting_row);
    int pivoting_column = pivoting_row;

    for (int j = 0; j < m.rows; j++) {
        if (m.at(j, pivoting_row) > greater) {
            greater = m.at(j, pivoting_row);
            pivoting_column = j;
        }
    }

    // Pivoting is not possible
    if (greater == 0) {
        return false;
    }

    // Swapping is not necessary
    if (pivoting_column == pivoting_row) {
        return true;
    }

    m.swap_rows(pivoting_row, pivoting_column);

    return true;
}
