#ifndef MATHUT_H
#define MATHUT_H
#include "matrix.h"

namespace MathUt
{
    bool fatoracaoLU(Matrix m, Matrix &L, Matrix &U);
    bool GaussJordan(const Matrix &coef, const Matrix &tInd, Matrix &solucao);
    bool pivot(Matrix &m, int i);
}

#endif
