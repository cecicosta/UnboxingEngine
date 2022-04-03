#ifndef VECT3D_H
#define VECT3D_H

class vector3D
{
    public:
    float x;
    float y;
    float z;

    vector3D();

    vector3D( float x, float y, float z );

    explicit vector3D( float vet[] );

    vector3D( const vector3D &v );

    friend vector3D operator+ ( vector3D v1, vector3D v2 );

    friend vector3D operator- ( vector3D v1, vector3D v2 );

    friend vector3D operator* ( vector3D v, float escalar );

    friend vector3D operator* ( float escalar, vector3D v );

    friend vector3D operator/ ( vector3D v, float escalar );

    float modulo();

    vector3D normalizado();

    float escalar(vector3D vet);

    vector3D vetorial( vector3D B );

};

#endif //VECT3D_H
