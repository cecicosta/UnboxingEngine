#ifndef REGION_H
#define REGION_H

#include "Vector3Df.h"
#include "texture.h"

class region
{

public:
    Vector3Df position;
    Vector3Df w; // Velocidade Angular da regi�o
    Vector3Df v; // Velocidade de transla��o da regi�o
    float mass;
    float inertia;
    float COR;  // coeficiente de restitui��o

    public:

    void setVelocity(Vector3Df v );
    Vector3Df getVelocity();

    void setAngularVelocity(Vector3Df w );
    Vector3Df getAngularVelocity();

    float getMass();
    void setMass(float mass);

    float getInertia();
    float getCOR();

    //Checa se um ponto est� dentro da regi�o
    virtual bool isInside(Vector3Df point ) = 0;

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a regi�o.
    virtual bool intersectionWithRay(Vector3Df p1, Vector3Df p2, Vector3Df &intersection ) = 0;
    virtual bool intersectionWithLine(Vector3Df p1, Vector3Df p2, Vector3Df &intersection ) = 0;

    virtual void draw(float pos[], float scale[], Texture* tex) = 0;

};

#endif

