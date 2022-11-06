#ifndef SPHERE_H
#define SPHERE_H

#include "UVector.h"
#include "region.h"
#include "texture.h"

class sphere: public region
{
    public:
    float radius;

    public:
    sphere();
    sphere(float radius);
    sphere(Vector3f position, float radius);

    void setRadius( float radius );
    float getRadius();

    Vector3f getNormal(Vector3f point);

    //Checa se um ponto está dentro da esfera
    virtual bool isInside(Vector3f point );

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a esfera.
    virtual bool intersectionWithRay(Vector3f p1, Vector3f p2, Vector3f &intersection );
    bool intersectionWithLine(Vector3f p1, Vector3f p2, Vector3f &intersection );

    virtual void draw(float pos[], float scale[], Texture* tex){}

};

#endif
