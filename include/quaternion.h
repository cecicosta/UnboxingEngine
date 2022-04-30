#ifndef QUAT_H
#define QUAT_H
#include "Vector3d.h"

class quaternion: public Vector3d {

    float m[16];

    public:
    float w;

    quaternion();
    quaternion(Vector3d v );
    quaternion( float w, float x, float y, float z );

    //Construtor para quaternion de rotação.
    //Recebe o angulo de rotação  o eixo.
    quaternion( float alpha, Vector3d eixo);

    friend quaternion operator+ ( quaternion v1, quaternion v2 );

    friend quaternion operator- ( quaternion v1, quaternion v2 );

    friend quaternion operator* ( quaternion v, float escalar );

    friend quaternion operator* ( float escalar, quaternion v );

    friend quaternion operator* ( quaternion q1, quaternion q2 );

    friend quaternion operator/ ( quaternion v, float escalar );

    float Length();

    quaternion conjugado();

    quaternion normalizado();

    quaternion inverso();

    //Retorna os angulos de rotação de euler
    Vector3d Euler();

    //Matriz de rotação ( quaternion deve estar Normalized )
    float* getMatrix();


};


#endif //QUAT_H
