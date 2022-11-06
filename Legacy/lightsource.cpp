#include "lightsource.h"
lightSource::lightSource(int sourceNum, vector3D position)
{
    this->enable = true;
    this->sourceNum = sourceNum;
    this->angulo = 0;
    this->exponent = 0;
    this->position = position;
    this->direction = vector3D(0,0,0);

    setLightAmb( 0.0,0.0,0.0,1.0);
    setLightDif( 1.0,1.0,1.0,1.0);
    setLightSpec(1.0,1.0,1.0,1.0);

    GLfloat pos[] = { position.x, position.y, position.z, 1.0 };
    glLightfv( GL_LIGHT0+sourceNum, GL_POSITION, pos );
    glEnable(GL_LIGHT0+sourceNum);
}

lightSource::lightSource( int sourceNum, vector3D position, vector3D direction, GLfloat exponent, GLfloat ang, GLfloat dif[], GLfloat spec[], GLfloat amb[] )
{
    this->enable = true;
    this->sourceNum = sourceNum;
    this->angulo = ang;
    this->exponent = exponent;
    this->position = position;
    this->direction = direction;

    setLightAmb( amb[0], amb[1], amb[2], amb[3] );
    setLightDif( dif[0], dif[1], dif[2], dif[3] );
    setLightSpec( spec[0], spec[1], spec[2], spec[3] );

    GLfloat pos [] = { position.x, position.y, position.z, 1 } ;
    GLfloat dir[] = { direction.x, direction.y, direction.z };

    glLightfv( GL_LIGHT0+sourceNum, GL_POSITION, pos );
    glLightfv( GL_LIGHT0+sourceNum, GL_SPOT_DIRECTION, dir);
    glLightf ( GL_LIGHT0+sourceNum, GL_SPOT_EXPONENT , exponent);
    glLightf ( GL_LIGHT0+sourceNum, GL_SPOT_CUTOFF, ang);
    glEnable(GL_LIGHT0+sourceNum);
}

lightSource::lightSource( int sourceNum, vector3D position, GLfloat dif[], GLfloat spec[], GLfloat amb[] )
{
    this->enable = true;
    this->sourceNum = sourceNum;
    this->angulo = 0;
    this->exponent = 0;
    this->position = position;
    this->direction = vector3D(0,0,0);

    setLightAmb( amb[0], amb[1], amb[2], amb[3]);
    setLightDif( dif[0], dif[1], dif[2], dif[3]);
    setLightSpec( spec[0], spec[1], spec[2], spec[3]);

    GLfloat pos[] = { position.x, position.y, position.z, 1.0 };
    glLightfv( GL_LIGHT0, GL_POSITION, pos );
    glEnable(GL_LIGHT0+sourceNum);
}
void lightSource::setLightAmb(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    lightAmb[0] = r;
    lightAmb[1] = g;
    lightAmb[2] = b;
    lightAmb[3] = a;

    glLightfv(GL_LIGHT0+sourceNum, GL_AMBIENT, lightAmb );
}


void lightSource::setLightDif(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    lightDif[0] = r;
    lightDif[1] = g;
    lightDif[2] = b;
    lightDif[3] = a;

    glLightfv(GL_LIGHT0+sourceNum, GL_DIFFUSE, lightDif );
}

void lightSource::setLightSpec(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    lightSpe[0] = r;
    lightSpe[1] = g;
    lightSpe[2] = b;
    lightSpe[3] = a;

    glLightfv(GL_LIGHT0+sourceNum, GL_SPECULAR, lightSpe );
}
void lightSource::setLightPos( vector3D position )
{
    this->position = position;
}

void lightSource::setLightDir( vector3D direction )
{
    this->direction = direction;
}

void lightSource::setEnable(bool enable)
{
    this->enable = enable;
    if(enable)
    {
        glLightfv(GL_LIGHT0+sourceNum, GL_AMBIENT, lightAmb );
        glLightfv(GL_LIGHT0+sourceNum, GL_SPECULAR, lightSpe );
        glLightfv(GL_LIGHT0+sourceNum, GL_DIFFUSE, lightDif );

        GLfloat pos [] = { position.x, position.y, position.z, 1 } ;
        GLfloat dir[] = { direction.x, direction.y, direction.z };
        glLightfv( GL_LIGHT0+sourceNum, GL_POSITION, pos );
        glLightfv( GL_LIGHT0+sourceNum, GL_SPOT_DIRECTION, dir);
        glLightf ( GL_LIGHT0+sourceNum, GL_SPOT_EXPONENT , exponent);
        glLightf ( GL_LIGHT0+sourceNum, GL_SPOT_CUTOFF, angulo);

        glEnable( GL_LIGHT0+sourceNum );
    }
    else
        glDisable( GL_LIGHT0+sourceNum );
}

bool lightSource::isEnable(){ return this->enable; }
