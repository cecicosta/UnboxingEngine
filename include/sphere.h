#ifndef SPHERE_H
#define SPHERE_H

#include "Vector3d.h"
#include "region.h"
#include "texture.h"

class sphere: public region
{
    public:
    float radius;

    public:
    sphere();
    sphere(float radius);
    sphere(Vector3d position, float radius);

    void setRadius( float radius );
    float getRadius();

    Vector3d getNormal(Vector3d point);

    //Checa se um ponto está dentro da esfera
    virtual bool isInside(Vector3d point );

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a esfera.
    virtual bool intersectionWithRay(Vector3d p1, Vector3d p2, Vector3d &intersection );
    bool intersectionWithLine(Vector3d p1, Vector3d p2, Vector3d &intersection );

    virtual void draw(float pos[], float scale[], Texture* tex){}

};

#endif
