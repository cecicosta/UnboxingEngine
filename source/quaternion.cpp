#include "quaternion.h"
#include <math.h>

    quaternion::quaternion() {}

    quaternion::quaternion( vector3D v )
    {
        w = 0;
        x = v.x;
        y = v.y;
        z = v.z;
    }

    quaternion::quaternion( float w, float x, float y, float z )
    {
        this->w = w;
        this->x = x;
        this->y = y;
        this->z = z;
    }

    //Construtor para quaternion de rotação
    quaternion::quaternion( float alpha, vector3D eixo)
    {
        float alpha_rad = 3.1415*alpha/180.0;
        float cosseno = cos( alpha_rad/2 );
        float seno = sin( alpha_rad/2 );

        eixo = eixo.normalizado();
        eixo = eixo*seno;

        w = cosseno;
        x = eixo.x;
        y = eixo.y;
        z = eixo.z;
    }

    quaternion operator+ ( quaternion v1, quaternion v2 )
    {
        quaternion novo;
        novo.w = v1.w + v2.w;
        novo.x = v1.x + v2.x;
        novo.y = v1.y + v2.y;
        novo.z = v1.z + v2.z;
        return novo;
    }

    quaternion operator- ( quaternion v1, quaternion v2 )
    {
        quaternion novo;
        novo.w = v1.w - v2.w;
        novo.x = v1.x - v2.x;
        novo.y = v1.y - v2.y;
        novo.z = v1.z - v2.z;
        return novo;
    }

    quaternion operator* ( quaternion v, float escalar )
    {
        quaternion novo;
        novo.w = v.w*escalar;
        novo.x = v.x*escalar;
        novo.y = v.y*escalar;
        novo.z = v.z*escalar;

        return novo;
    }

    quaternion operator* ( float escalar, quaternion v )
    {
        quaternion novo;
        novo.w = v.w*escalar;
        novo.x = v.x*escalar;
        novo.y = v.y*escalar;
        novo.z = v.z*escalar;

        return novo;
    }

    quaternion operator* ( quaternion q1, quaternion q2 )
    {
        vector3D v1( q1.x,q1.y,q1.z );
        vector3D v2( q2.x,q2.y,q2.z );
        float w = q1.w*q2.w - v1.escalar(v2);
        vector3D v = q1.w*v2 + q2.w*v1 + v1.vetorial(v2);

        return quaternion( w, v.x, v.y, v.z );
    }

    quaternion operator/ ( quaternion v, float escalar )
    {
        return v*(1/escalar);
    }

    float quaternion::modulo()
    {
        return sqrt( w*w + x*x + y*y + z*z );
    }

    quaternion quaternion::conjugado()
    {
        return quaternion(w, -x, -y, -z);
    }

    quaternion quaternion::normalizado()
    {
        float norma = sqrt( w*w + x*x + y*y + z*z );
        return quaternion( w/norma, x/norma, y/norma, z/norma );
    }

    quaternion quaternion::inverso()
    {
        float pInt = w*w + x*x + y*y + z*z;

        if( pInt == 0 ) return quaternion(0,0,0,0);

        return quaternion( w/pInt, -x/pInt, -y/pInt, -z/pInt );
    }

    //Retorna o angulo de rotação em um determinado eixo
    vector3D quaternion::Euler()
    {
        float PI = 3.1415;
        vector3D ang;
        float test = x*y + z*w;
                if (test > 0.499) { // singularity at north pole
                        ang.y = 2 * atan2(x,w);
                        ang.z = PI/2;
                        ang.x = 0;
                        return ang;
                }
                if (test < -0.499) { // singularity at south pole
                        ang.y = -2 * atan2(x,w);
                        ang.z = -PI/2;
                        ang.x = 0;
                        return ang;
                }
                float sqx = x*x;
                float sqy = y*y;
                float sqz = z*z;
                ang.y = atan2(2*y*w-2*x*z , 1 - 2*sqy - 2*sqz);
                ang.z = asin(2*test);
                ang.x = atan2(2*x*w-2*y*z , 1 - 2*sqx - 2*sqz);
                return ang;
    }

    //Matriz de rotação ( quaternion deve estár normalizado )
    float* quaternion::getMatrix()
    {
        float x2 = x * x;
        float y2 = y * y;
        float z2 = z * z;
        float xy = x * y;
        float xz = x * z;
        float yz = y * z;
        float wx = w * x;
        float wy = w * y;
        float wz = w * z;

        m[0] = 1.0f - 2.0f * (y2 + z2);
        m[1] = 2.0f * (xy - wz);
        m[2] = 2.0f * (xz + wy);
        m[3] = 0.0f;
        m[4] = 2.0f * (xy + wz);
        m[5] = 1.0f - 2.0f * (x2 + z2);
        m[6] = 2.0f * (yz - wx);
        m[7] = 0.0f;
        m[8] = 2.0f * (xz - wy);
        m[9] = 2.0f * (yz + wx);
        m[10] = 1.0f - 2.0f * (x2 + y2);
        m[11] = 0.0f;
        m[12] = 0.0f;
        m[13] = 0.0f;
        m[14] = 0.0f;
        m[15] = 1.0f;
        return m;
    }
