#ifndef REGION_H
#define REGION_H

#include "UVector.h"
#include "texture.h"

class region
{

public:
    Vector3f position;
    Vector3f w; // Velocidade Angular da regi�o
    Vector3f v; // Velocidade de transla��o da regi�o
    float mass;
    float inertia;
    float COR;  // coeficiente de restitui��o

    public:

    void setVelocity(Vector3f v );
    Vector3f getVelocity();

    void setAngularVelocity(Vector3f w );
    Vector3f getAngularVelocity();

    float getMass();
    void setMass(float mass);

    float getInertia();
    float getCOR();

    //Checa se um ponto est� dentro da regi�o
    virtual bool isInside(Vector3f point ) = 0;

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a regi�o.
    virtual bool intersectionWithRay(Vector3f p1, Vector3f p2, Vector3f &intersection ) = 0;
    virtual bool intersectionWithLine(Vector3f p1, Vector3f p2, Vector3f &intersection ) = 0;

    virtual void draw(float pos[], float scale[], Texture* tex) = 0;

};

#endif

