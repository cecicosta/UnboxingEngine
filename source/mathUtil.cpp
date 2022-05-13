#include "mathUtil.h"
#include <iostream>
#include <vector>
using namespace std;


bool MathUt::fatoracaoLU(Matrix m, Matrix &L, Matrix &U)
{
    int n = m.order;

    // Criando a matriz "L". Inicialmente, ela é uma matriz identidade:
    L = Matrix::Identity(n);

    // Criando a matriz "U". Inicialmente, ela é uma cópia da matriz "A":
    U = m;

    // Preencher os valores de "L" e "U":
    for (int k=0 ; k<n ; k++)
    {
        for (int i=(k+1) ; i<n ; i++)
        {
            // Testa o pivô:
            if (U.at(k, k) == 0 )
            {
                if( pivot(U, k) == false )
                {
                    cout << ">>> Resolver sistema: o pivo a["<<k<<","<<k<<"] e ZERO!!!"<<endl;
                    return(false);
                }
            }
            // Calcula o "m(i,k)":
            double m = (U.at(i, k)) / (U.at(k, k));


            // Matriz "L": recebe o valor de m na posição (i,k):
            L.at(i, k) = m;

            // Matriz "A": linha "i" recebe ela mesma menos "m(i,k)" multiplicado pela linha "k":

            for (int j=0 ; j<n ; j++)
                U.at(i, j) = U.at(i, j) - m * U.at(k, j);
            // Para evitar valores próximos de zero, o elemento "a(i,k)" recebe logo zero:
            U.at(i, k) = 0;
        }
    }
    return(true);

}


bool MathUt::GaussJordan(const Matrix &coef, const Matrix &tInd, Matrix &solucao)
{
    const int ordem = coef.order;
    for( int t=0; t<tInd.m_columns; t++ )
    {

        Matrix copy;

        copy = coef;
        std::vector<float> b(ordem);
        for(int i=0;i<ordem;i++)
            b[i]= tInd.at(i, t);



        for(int k=0; k<ordem; k++ )
        {
            float pivo = copy.at(k, k);

            if( pivo == 0 )
            {
                if(!pivot(copy, k))
                {
                    cout << ">>> Resolver sistema por gauss jordan: o pivo a["<<k<<","<<k<<"] e ZERO!!!"<<endl;
                    return(false);
                }
            }

            for(int i=0;i<ordem;i++)
            {
                if(i!=k)
                {

                    float m = copy.at(i, k)/pivo;
                    for(int j=0;j<ordem;j++)
                    {
                        copy.at(i, j) = copy.at(i, j)- copy.at(k, j)*m;
                    }
                    b[i]= b[i]-b[k]*m;
                }
            }
        }

        for(int i=ordem-1; i>=0;i--)
        {
            solucao.at(i, t) = (b[i])/ copy.at(i, i);
        }
    }
}


bool MathUt::pivot(Matrix &m, int i)
{

    float maior= m.at(i, i);
    int k=i;

    for(int j=0;j<m.order; j++)
    {
        if(m.at(j, i) > maior)
        {
            maior = m.at(j, i);
            k=j;
        }
    }

    if( maior == 0 )
    {
        cout<<"Nao eh possivel rsolver o sistema";
        return false;
    }

    if(k==i)
    {
        cout<<"Pivotação não é necessaria\n";
        return true;
    }

    m.swap_rows(i, k);

    return true;

}

