#ifndef QUAT_H
#define QUAT_H
#include "vector3D.h"

class quaternion: public vector3D
{

    float m[16];

    public:
    float w;

    quaternion();
    quaternion( vector3D v );
    quaternion( float w, float x, float y, float z );

    //Construtor para quaternion de rotação.
    //Recebe o angulo de rotação  o eixo.
    quaternion( float alpha, vector3D eixo);

    friend quaternion operator+ ( quaternion v1, quaternion v2 );

    friend quaternion operator- ( quaternion v1, quaternion v2 );

    friend quaternion operator* ( quaternion v, float escalar );

    friend quaternion operator* ( float escalar, quaternion v );

    friend quaternion operator* ( quaternion q1, quaternion q2 );

    friend quaternion operator/ ( quaternion v, float escalar );

    float modulo();

    quaternion conjugado();

    quaternion normalizado();

    quaternion inverso();

    //Retorna os angulos de rotação de euler
    vector3D Euler();

    //Matriz de rotação ( quaternion deve estar normalizado )
    float* getMatrix();


};


#endif //QUAT_H
