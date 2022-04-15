#include "material.h"

material::material()
{
    setMaterialAmb( 0.2, 0.2, 0.2, 1.0 );
    setMaterialDif( 0.8, 0.8, 0.8, 1.0 );
    setMaterialSpec( 0.0, 0.0, 0.0, 1.0 );
    setMaterialEmis( 0.2, 0.2, 0.2, 1.0 );
}

material::material(float dif[], float spec[], float amb[], float emis[], float shininess)
{
    setMaterialAmb( amb[0], amb[1], amb[2], amb[3] );
    setMaterialDif( dif[0], dif[1], dif[2], dif[3] );
    setMaterialSpec( spec[0], spec[1], spec[2], spec[3] );
    setMaterialEmis( emis[0], emis[1], emis[2], emis[3] );
    this->shininess = shininess;
}

void material::Illuminated(bool enable)
{
    this->enable = enable;
}

void material::setMaterialAmb(float r, float g, float b, float a)
{
    materialAmb[0] = r;
    materialAmb[1] = g;
    materialAmb[2] = b;
    materialAmb[3] = a;
}

void material::setMaterialDif(float r, float g, float b, float a)
{
    materialDif[0] = r;
    materialDif[1] = g;
    materialDif[2] = b;
    materialDif[3] = a;
}

void material::setMaterialSpec(float r, float g, float b, float a)
{
    materialSpe[0] = r;
    materialSpe[1] = g;
    materialSpe[2] = b;
    materialSpe[3] = a;
}

void material::setMaterialEmis(float r, float g, float b, float a)
{
    materialEmis[0] = r;
    materialEmis[1] = g;
    materialEmis[2] = b;
    materialEmis[3] = a;
}
//
//void material::activeMaterial()
//{
//    if( enable )
//    {
//        glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, materialAmb);
//        glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, materialDif);
//        glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, materialSpe);
//        glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, materialEmis);
//        glMaterialf  (GL_FRONT_AND_BACK, GL_SHININESS, shininess );
//    }
//    else
//    {
//        float amb[] = {0.0,0.0,0.0,1.0};
//        float dif[] = {0.0,0.0,0.0,1.0};
//        float spe[] = {0.0,0.0,0.0,1.0};
//        float emi[] = {1.0,1.0,1.0,1.0};
//
//        glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, amb);
//        glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
//        glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, spe);
//        glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, emi);
//        glMaterialf  (GL_FRONT_AND_BACK, GL_SHININESS, 0 );
//    }
//}
