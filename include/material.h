#ifndef MATERIAL_H
#define MATERIAL_H
#include <GL/gl.h>
class material
{
    //Capacidade de brilho do material
    GLfloat materialSpe[4];
    GLfloat materialAmb[4];
    GLfloat materialDif[4];
    GLfloat materialEmis[4];
    GLint shininess;
    bool enable;
public:
    material();
    material( GLfloat dif[], GLfloat spec[], GLfloat amb[], GLfloat emis[], float shininess = 0 );
    void Illuminated(bool enable);
    bool isIlluminated(){ return enable; }

    void setMaterialAmb(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setMaterialDif(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setMaterialSpec(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    void setMaterialEmis(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    GLfloat* getMaterialAmb() { return materialAmb; }
    GLfloat* getMaterialDif() { return materialDif; }
    GLfloat* getMaterialSpec(){ return materialSpe; }
    GLfloat* getMaterialEmis(){ return materialEmis;}
    void setShininess( int sh ){ this->shininess = sh;  }
    void activeMaterial();

};

#endif // MATERIAL_H
