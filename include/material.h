#ifndef MATERIAL_H
#define MATERIAL_H
class material
{
    //Capacidade de brilho do material
    float materialSpe[4];
    float materialAmb[4];
    float materialDif[4];
    float materialEmis[4];
    int shininess;
    bool enable;
public:
    material();
    material( float dif[], float spec[], float amb[], float emis[], float shininess = 0 );
    void Illuminated(bool enable);
    bool isIlluminated(){ return enable; }

    void setMaterialAmb(float r, float g, float b, float a);
    void setMaterialDif(float r, float g, float b, float a);
    void setMaterialSpec(float r, float g, float b, float a);
    void setMaterialEmis(float r, float g, float b, float a);
    float* getMaterialAmb() { return materialAmb; }
    float* getMaterialDif() { return materialDif; }
    float* getMaterialSpec(){ return materialSpe; }
    float* getMaterialEmis(){ return materialEmis;}
    void setShininess( int sh ){ this->shininess = sh;  }
//    void activeMaterial();

};

#endif // MATERIAL_H
