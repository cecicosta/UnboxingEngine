#ifndef QUAD_H
#define QUAD_H

#include "Vector3d.h"
#include "region.h"

class rect: public region
{
    vector3D a;
    vector3D b;
    vector3D c;
    vector3D d;

    public:
    rect();

    vector3D getNormal();

    //Checa se um ponto est� dentro da esfera
    virtual bool isInside( vector3D point );

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a esfera.
    virtual bool intersectionWithRay( vector3D p1, vector3D p2, vector3D &intersection );
    virtual bool intersectionWithLine( vector3D p1, vector3D p2, vector3D &intersection );

    virtual void draw(float pos[], float scale[], Texture* tex){}

};

#endif
