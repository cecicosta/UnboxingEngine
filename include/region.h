#ifndef REGION_H
#define REGION_H

#include "Vector3d.h"
#include "texture.h"

class region
{

public:
    Vector3d position;
    Vector3d w; // Velocidade Angular da regi�o
    Vector3d v; // Velocidade de transla��o da regi�o
    float mass;
    float inertia;
    float COR;  // coeficiente de restitui��o

    public:

    void setVelocity(Vector3d v );
    Vector3d getVelocity();

    void setAngularVelocity(Vector3d w );
    Vector3d getAngularVelocity();

    float getMass();
    void setMass(float mass);

    float getInertia();
    float getCOR();

    //Checa se um ponto est� dentro da regi�o
    virtual bool isInside(Vector3d point ) = 0;

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a regi�o.
    virtual bool intersectionWithRay(Vector3d p1, Vector3d p2, Vector3d &intersection ) = 0;
    virtual bool intersectionWithLine(Vector3d p1, Vector3d p2, Vector3d &intersection ) = 0;

    virtual void draw(float pos[], float scale[], Texture* tex) = 0;

};

#endif

