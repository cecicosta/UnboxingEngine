#ifndef REGION_H
#define REGION_H

#include "vector3D.h"
#include "texture.h"

class region
{

public:
    vector3D position;
    vector3D w; // Velocidade Angular da região
    vector3D v; // Velocidade de translação da região
    float mass;
    float inertia;
    float COR;  // coeficiente de restituição

    public:

    void setVelocity( vector3D v );
    vector3D getVelocity();

    void setAngularVelocity( vector3D w );
    vector3D getAngularVelocity();

    float getMass();
    void setMass(float mass);

    float getInertia();
    float getCOR();

    //Checa se um ponto está dentro da região
    virtual bool isInside( vector3D point ) = 0;

    //Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a região.
    virtual bool intersectionWithRay( vector3D p1, vector3D p2, vector3D &intersection ) = 0;
    virtual bool intersectionWithLine( vector3D p1, vector3D p2, vector3D &intersection ) = 0;

    virtual void draw(float pos[], float scale[], Texture* tex) = 0;

};

#endif

