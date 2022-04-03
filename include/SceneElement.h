#ifndef SCENEELEMENT_H
#define SCENEELEMENT_H

/*Classe que representa um objeto génerico. Possui ids para
a geometria e textura, variaveis de estado para posição, escala e rotação,
região de colisão e material*/

#include <vector>
#include <string>
#include "vector3D.h"
#include "quaternion.h"
#include "region.h"
#include "material.h"
#include "meshbuffer.h"
#include "matrix.h"

//Tipos referentes ao formato utilizado.
#define SIMPLE  0
#define NOSIMPLE 1
#define NOSIMPLEANIM 2

class SceneElement
{
    public:
    vector3D position;
    vector3D scale;
    sphere reg;
    int texID;
    int id;
    int meshID;
    int type;
    int nivel;
    material SEmaterial;
    Matrix transformation;
    public:
    quaternion q;


    SceneElement();
    SceneElement( sphere reg, vector3D position, int meshID, int texID, int type, int id, bool illuminated = false);

    int getID();
    void setID(int id);
    int getNivel();
    int getType();
    int getTexID();
    int getName();
    double *getEmissivity();
    vector3D getPosition();
    void setPosition(vector3D position);
    void setRotation( double ang, vector3D eixo);
    vector3D getScale();
    void setScale( vector3D scale );
    void setMaterial( material mat);
    material getMaterial();
    meshBuffer *getMesh();
    Matrix &getMatrix();
    virtual int Interation( SceneElement* );
    virtual void Update();
};



#endif
