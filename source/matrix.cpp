#include "matrix.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "mathUtil.h"

using namespace std;

Matrix::Matrix()
{
    order = 0;
    size_i  = 0;
    size_j = 0;
    A = NULL;
}

Matrix::Matrix(const int n) {
    order = n;
    size_i  = n;
    size_j = n;
    if( n == 0 )
        A = NULL;
    else
        A = new float[n*n];
}

Matrix::Matrix( Matrix const &m )
{
    A = new float[m.order*m.order];
    order = m.order;
    size_i = m.size_i;
    size_j = m.size_j;

    for(int i=0; i<order*order; i++)
        A[i] = m.A[i];
}

Matrix::Matrix(int size_i, int size_j)
{

    if( size_i == 0 || size_j == 0 )
    {
        A = NULL;
        return;
    }

    if( size_i > size_j )
        order = size_i;
    else
        order = size_j;

    this->size_i = size_i;
    this->size_j = size_j;

    A = new float[order*order];

}

Matrix::Matrix( float m[], int n )
{
    order = n;
    size_i  = n;
    size_j = n;
    if( n == 0 )
        A = NULL;
    else
        A = new float[n*n];

    for( int i=0; i<n*n; i++ )
    {
        A[i] = m[i];
    }
}

Matrix::Matrix( vector3D vet )
{
    A = new float[4];
    size_i = 4;
    size_j = 1;
    order = 4;
    index(0,0) = vet.x;
    index(1,0) = vet.y;
    index(2,0) = vet.z;
    index(3,0) = 1;
}

Matrix Matrix::identity(int n)
{
    Matrix ident(n);

    for(int i=0; i<n; i++)
    for(int j=0; j<n; j++)
        if( i==j )
            ident.index(i,j) = 1;
        else
            ident.index(i,j) = 0;

    return ident;
}

Matrix Matrix::translationMatrix(vector3D translation)
{
    Matrix m = identity(4);
    m.index(0,3) = translation.x;
    m.index(1,3) = translation.y;
    m.index(2,3) = translation.z;
    return m;
}
//Cria uma matriz de escala
Matrix Matrix::scaleMatrix(vector3D scale)
{
    Matrix m = identity(4);
    m.index(0,0) = scale.x;
    m.index(1,1) = scale.y;
    m.index(2,2) = scale.z;
    return m;
}
//Cria uma matriz de rotação
Matrix Matrix::rotationMatrix( float ang, vector3D axis )
{
    quaternion q( ang, axis );
    return Matrix(q.getMatrix(), 4);
}
Matrix rotationMatrix( quaternion q )
{
    return Matrix(q.getMatrix(), 4);
}

void Matrix::create( int n )
{
    order = n;
    delete A;
    A = new float[n*n];
    for( int i=0;i<n;i++ )
        for( int j=0;j<n;j++ )
        {
            cout << "A"<< i << j << ": ";
            cin >> index(i,j);
        }
}


void Matrix::create()
{
    for( int i=0;i<size_i;i++ )
        for( int j=0;j<size_j;j++ )
        {
            cout << "A"<< i << j << ": ";
            cin >> index(i,j);
        }
}

float &Matrix::index(int i, int j)
{
    return A[j*order+i];
}

float Matrix::index(int i, int j) const
{
    return A[j*order+i];
}

void Matrix::ver()
{
    for( int i=0;i<size_i;i++ )
    {
        for( int j=0;j<size_j;j++ )
        {
            if( modulo(index(i,j)) < 0.00001 )
                cout<< " A"<< i << j << " = " << 0;
            else
                cout<< " A"<< i << j << " = " << index(i,j);
        }
        cout<<"\n";
    }
    cout << "\n";
}

Matrix Matrix::operator+( Matrix m )
{
    if( size_i == m.size_i && size_j == m.size_j )
    {
        Matrix novo(size_i, size_j);
        for(int i=0; i<size_i; i++)
        {
            for(int j=0; j<size_j; j++)
            {
                novo.index(i,j) = index(i,j) + m.index(i,j);
            }
        }
        return novo;
    }
    return Matrix();
}

Matrix Matrix::operator- ( Matrix m )
{
    if( size_i == m.size_i && size_j == m.size_j )
    {
        Matrix novo(size_i, size_j);
        for(int i=0; i<size_i; i++)
        {
            for(int j=0; j<size_j; j++)
            {
                novo.index(i,j) = index(i,j) - m.index(i,j);
            }
        }
        return novo;
    }
    return Matrix();
}

Matrix operator*( Matrix m1, Matrix m2  )
{
    if( m1.size_j == m2.size_i )
    {
        float soma = 0;
        Matrix novo(m1.size_i, m2.size_j);
        for( int i = 0 ; i < m2.size_j ; i++ ){
            for( int j = 0 ; j < m1.size_i ; j++ ){
                soma = 0;
                for( int k = 0 ; k < m1.size_j ; k++ ){
                    soma = soma + m1.index(j,k)*m2.index(k,i);
                }
                novo.index(j,i) = soma;
            }
        }
        return novo;
    }
    return Matrix();
}

vector3D operator*( Matrix m, vector3D vet )
{
    if( m.size_i != 4 || m.size_j != 4 )
        return vector3D();

    vector3D res;
    float soma;

    soma = 0;
    soma += m.index(0,0)*vet.x;
    soma += m.index(0,1)*vet.y;
    soma += m.index(0,2)*vet.z;
    soma += m.index(0,3);
    res.x = soma;

    soma = 0;
    soma += m.index(1,0)*vet.x;
    soma += m.index(1,1)*vet.y;
    soma += m.index(1,2)*vet.z;
    soma += m.index(1,3);
    res.y = soma;

    soma = 0;
    soma += m.index(2,0)*vet.x;
    soma += m.index(2,1)*vet.y;
    soma += m.index(2,2)*vet.z;
    soma += m.index(2,3);
    res.z = soma;

    return res;
}

void Matrix::switchLines(int i, int j)
{
    if( i>=0 && i<order && j>=0 && j<order )
    {
        float aux;
        for(int k=0;k<order;k++)
        {
            aux = index(i,k);
            index(i,k) = index(j,k);
            index(j,k) = aux;
        }
    }
}

Matrix Matrix::operator* ( float a )
{
    Matrix novo(size_i, size_j);
    for(int i=0; i<size_i; i++)
    {
        for(int j=0; j<size_j; j++)
        {
            novo.index(i,j) = index(i,j)*a;
        }
    }
    return novo;
}

void Matrix::operator= ( Matrix m )
{
    A = new float[ m.order*m.order ];
    order = m.order;
    size_i = m.size_i;
    size_j = m.size_j;

    for(int i=0; i<size_i*size_i; i++)
    {
        A[i] = m.A[i];
    }
}

Matrix Matrix::transpose()
{
    Matrix m(size_j, size_i);

    for(int i=0;i<size_i; i++)
    {

        for(int j=0; j<size_j; j++)
        {
            m.index(j,i) = index(i,j);
        }
    }
    return m;
}

Matrix Matrix::inverse()
{
    Matrix sol = Matrix::identity(order);
    Matrix inv(order);
    MathUt::GaussJordan(*this, sol, inv);
    return inv;
}

//Matrix Matrix::inverse(){
//
//    int n = order;
//    float c[n];
//    Matrix d(order);
//    Matrix *s = fl(*this, c);
//    for (int i=0; i<n; ++i)
//      for (int j=0; j<n; ++j)
//        d.index(i,j) = -s[n-1].index(i,j)/c[0];
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
//    for (int i=0; i<n; ++i) s[0].index(i,i) = 1;
//    for (int k=1; k<n; ++k) {
//      s[k] =a*s[k-1];
//      c[n-k] = -tr(a*s[k-1])/k;
//      for (int i=0; i<n; ++i)
//      s[k].index(i,i) += c[n-k];
//    }
//      c[0] = -tr(a*s[n-1])/n;
//    return s;
//}
//
//// Method to calculate the trace of a matrix.
//float Matrix::tr(Matrix a) {
//    int n = a.order;
//    float sum = 0;
//    for (int i=0; i<n; ++i) sum += a.index(i,i);
//
//    return sum;
//}

float *Matrix::getMatrixGL()
{
    return A;
}

Matrix::~Matrix()
{
    if( A != NULL )
        delete [] A;
}

float Matrix::modulo( float num )
{
    if(num < 0) return -num;
    else return num;
}
