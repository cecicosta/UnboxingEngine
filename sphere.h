#ifndef SPHERE_H
#define SPHERE_H

#include "vector3D.h"
#include "region.h"
#include "texture.h"

class sphere: public region
{
    public:
    float radius;

    public:
    sphere();
    sphere(float radius);
    sphere(vector3D position, float radius);

    void setRadius( float radius );
    float getRadius();

    vector3D getNormal(vector3D point);

    //Checa se um ponto está dentro da esfera
    virtual bool isInside( vector3D point );

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a esfera.
    virtual bool intersectionWithRay( vector3D p1, vector3D p2, vector3D &intersection );
    bool intersectionWithLine( vector3D p1, vector3D p2, vector3D &intersection );

    virtual void draw(float pos[], float scale[], Texture* tex){}

};

#endif
