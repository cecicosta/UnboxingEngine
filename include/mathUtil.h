#pragma once
#include "Matrix.h"

namespace MathUt {
bool fatoracaoLU(Matrix4f m, Matrix4f &L, Matrix4f &U);
bool GaussJordan(const Matrix4f &coef, const Matrix4f &tInd, Matrix4f &solucao);
bool pivot(Matrix4f &m, int i);
}// namespace MathUt
