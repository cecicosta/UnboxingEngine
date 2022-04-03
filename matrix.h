#ifndef MATRIX_H
#define MATRIX_H
#include "vector3D.h"
#include "quaternion.h"
class Matrix
{
    public:
    float *A;
    int order;
    int size_i;
    int size_j;
    //Construtor da classe
    Matrix();
    Matrix(const int n);
    Matrix(Matrix const &m);
    Matrix(int size_i, int size_j);
    Matrix( float m[], int n );
    Matrix( vector3D vet );
    //Destrutor
    virtual ~Matrix();
    //Metodo para definir a Matrix a partir de entradas do usuario
    void create( int n );
    void create();
    //Retorna um elemento da Matrix dada a coordenada
    float &index(int i, int j);
    //Trocar linhas
    void switchLines(int i, int j);
    //Exibe em console a Matrix
    void ver();
    //Retorna a transpose da Matrix
    Matrix transpose();
    Matrix inverse();
    //Retorna a Matrix identity de order n
    static Matrix identity(int n);
    //Cria uma matriz de translação a aprtir de um vetor
    static Matrix translationMatrix(vector3D translation);
    //Cria uma matriz de escala
    static Matrix scaleMatrix(vector3D scale);
    //Cria uma matriz de rotação
    static Matrix rotationMatrix( float ang, vector3D axis );
    static Matrix rotationMatrix( quaternion q );
    //Retorna uma matriz no formato do opengl
    float *getMatrixGL();

    Matrix operator+ ( Matrix m );

    Matrix operator- ( Matrix m );

    friend Matrix operator* ( Matrix m1, Matrix m2 );

    friend vector3D operator* ( Matrix m, vector3D vet  );

    Matrix operator* ( float a );

    void operator= ( Matrix m );


    private:
    float modulo(float num);
    Matrix *fl(Matrix a, float c[]);
    float tr(Matrix a);
};


#endif // MATRIX_H
