#pragma once
#include "Matrix.h"
#include "Quaternion.h"
#include "Vector3Df.h"
#include "material.h"
#include "meshbuffer.h"
#include "region.h"
#include "sphere.h"


#include <string>
#include <vector>


class IUComponent {

};

class UComposite {

};



//Tipos referentes ao formato utilizado.
#define SIMPLE 0
#define NOSIMPLE 1
#define NOSIMPLEANIM 2

class SceneElement {
public:


    sphere reg;
    int texID;
    int meshID;

    int id;
    int type;
    int nivel;

public:
    SceneElement();
    SceneElement(sphere reg, vector3D position, int meshID, int texID, int type, int id, bool illuminated = false);

    int getID();
    void setID(int id);
    int getNivel();
    int getType();
    int getTexID();
    int getName();
    double *getEmissivity();
    vector3D getPosition();
    void setPosition(vector3D position);
    void setRotation(double ang, vector3D eixo);
    vector3D getScale();
    void setScale(vector3D scale);
    void setMaterial(material mat);
    material getMaterial();
    CMeshBuffer *getMesh();
    Matrix &getMatrix();
    virtual int Interation(SceneElement *);
    virtual void Update();
private:
    vector3D m_position;
    vector3D m_scale;
    quaternion m_rotation;
    Matrix m_transformation;
};
