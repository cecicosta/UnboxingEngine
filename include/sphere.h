#ifndef SPHERE_H
#define SPHERE_H

#include "Vector3Df.h"
#include "region.h"
#include "texture.h"

class sphere: public region
{
    public:
    float radius;

    public:
    sphere();
    sphere(float radius);
    sphere(Vector3Df position, float radius);

    void setRadius( float radius );
    float getRadius();

    Vector3Df getNormal(Vector3Df point);

    //Checa se um ponto está dentro da esfera
    virtual bool isInside(Vector3Df point );

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a esfera.
    virtual bool intersectionWithRay(Vector3Df p1, Vector3Df p2, Vector3Df &intersection );
    bool intersectionWithLine(Vector3Df p1, Vector3Df p2, Vector3Df &intersection );

    virtual void draw(float pos[], float scale[], Texture* tex){}

};

#endif
