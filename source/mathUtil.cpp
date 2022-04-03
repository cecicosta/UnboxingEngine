#include "mathUtil.h"
#include <iostream>
using namespace std;


bool MathUt::fatoracaoLU(Matrix m, Matrix &L, Matrix &U)
{
    int n = m.order;

    // Criando a matriz "L". Inicialmente, ela é uma matriz identidade:
    L = Matrix::identity(n);

    // Criando a matriz "U". Inicialmente, ela é uma cópia da matriz "A":
    U = m;

    // Preencher os valores de "L" e "U":
    for (int k=0 ; k<n ; k++)
    {
        for (int i=(k+1) ; i<n ; i++)
        {
            // Testa o pivô:
            if ( U.index(k,k) == 0 )
            {
                if( pivot(U, k) == false )
                {
                    cout << ">>> Resolver sistema: o pivo a["<<k<<","<<k<<"] e ZERO!!!"<<endl;
                    return(false);
                }
            }
            // Calcula o "m(i,k)":
            double m = (U.index(i,k)) / (U.index(k,k));


            // Matriz "L": recebe o valor de m na posição (i,k):
            L.index(i,k) = m;

            // Matriz "A": linha "i" recebe ela mesma menos "m(i,k)" multiplicado pela linha "k":

            for (int j=0 ; j<n ; j++)
                U.index(i,j) = U.index(i,j) - m * U.index(k,j);
            // Para evitar valores próximos de zero, o elemento "a(i,k)" recebe logo zero:
            U.index(i,k) = 0;
        }
    }
    return(true);

}


bool MathUt::GaussJordan(Matrix coef, Matrix tInd, Matrix &solucao)
{

    solucao = Matrix(tInd.size_i, tInd.size_j);

    for( int t=0; t<tInd.size_j; t++ )
    {

        Matrix copy;
        int ordem = coef.order;
        copy = coef;
        float b[ordem];
        for(int i=0;i<ordem;i++)
            b[i]=tInd.index(i,t);



        for(int k=0; k<ordem; k++ )
        {
            float pivo = copy.index(k,k);

            if( pivo == 0 )
            {
                if( pivot(copy, k) == false )
                {
                    cout << ">>> Resolver sistema por gauss jordan: o pivo a["<<k<<","<<k<<"] e ZERO!!!"<<endl;
                    return(false);
                }
            }

            for(int i=0;i<ordem;i++)
            {
                if(i!=k)
                {

                    float m = copy.index(i,k)/pivo;
                    for(int j=0;j<ordem;j++)
                    {
                        copy.index(i,j) = copy.index(i,j)-copy.index(k,j)*m;
                    }
                    b[i]= b[i]-b[k]*m;
                }
            }
        }

        for(int i=ordem-1; i>=0;i--)
        {
            solucao.index(i, t) = (b[i])/copy.index(i,i);
        }
    }
}


bool MathUt::pivot(Matrix &m, int i)
{

    float maior=m.index(i,i);
    int k=i;

    for(int j=0;j<m.order; j++)
    {
        if(m.index(j,i) > maior)
        {
            maior = m.index(j,i);
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

    m.switchLines(i,k);

    return true;

}

