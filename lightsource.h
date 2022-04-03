#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H
#include <GL/gl.h>
#include "vector3D.h"

#define SPOT  0
#define POINT 1

class lightSource
{
public:
    //Parametros opengl para iluminação
    GLfloat lightAmb[4];
    GLfloat lightDif[4];
    GLfloat lightSpe[4];
    GLfloat lightPos[4];
    vector3D position;
    vector3D direction;
    double  exponent;
    double angulo;
    int sourceNum;
    bool enable;


public:
    lightSource( int sourceNum, vector3D position );
    lightSource( int sourceNum, vector3D position, GLfloat dif[], GLfloat spec[], GLfloat amb[]);
    lightSource( int sourceNum, vector3D position, vector3D direction, GLfloat exponent, GLfloat ang, GLfloat dif[], GLfloat spec[], GLfloat amb[] );
    void setLightAmb(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setLightDif(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setLightSpec(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setLightPos( vector3D position );
    void setLightDir( vector3D direction );
    void setEnable(bool enable);
    vector3D getPosition(){ return position; }
    bool isEnable();

};

#endif // LIGHTSOURCE_H
