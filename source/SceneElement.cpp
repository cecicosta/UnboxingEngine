#include "SceneElement.h"
#include "resources.h"
SceneElement::SceneElement(){}
SceneElement::SceneElement( sphere reg, vector3D position, int meshID, int texID, int type, int id, bool illuminated)
{
    this->reg = reg;
    this->position = position;
    this->meshID = meshID;
    this->texID = texID;
    this->type = type;
    this->nivel = nivel;
    this->id = id;
    this->q = quaternion(0, vector3D(1,0,0));
    this->scale = vector3D(1,1,1);
    this->SEmaterial.Illuminated(illuminated);
    this->transformation = Matrix::translationMatrix(position);
    this->reg.position = position;
}

int SceneElement::getID()
{
    return id;
}
void SceneElement::setID(int id)
{
    this->id = id;
}
int SceneElement::getNivel()
{
    return nivel;
}
int SceneElement::getType()
{
    return type;
}
int SceneElement::getTexID()
{
    return texID;
}
int SceneElement::getName()
{
    return meshID;
}
double *SceneElement::getEmissivity()
{
    double *lightEmiss = new double[3];
    return lightEmiss;
}
vector3D SceneElement::getPosition()
{
    return position;
}
void SceneElement::setPosition(vector3D position)
{
    transformation = Matrix::translationMatrix(this->position*-1)*transformation;
    transformation = Matrix::translationMatrix(position)*transformation;
    this->position = position;
    this->reg.position = position;
}
void SceneElement::setRotation( double ang, vector3D eixo)
{
    transformation = Matrix::translationMatrix(position*-1)*transformation;
    transformation = Matrix::rotationMatrix( ang, eixo )*transformation;
    transformation = Matrix::translationMatrix(position)*transformation;
    q = quaternion( ang, eixo )*q;
}
vector3D SceneElement::getScale()
{
    return scale;
}
void SceneElement::setScale( vector3D scale )
{
    transformation = transformation*Matrix::scaleMatrix(scale);
    this->scale = scale;
}
void SceneElement::setMaterial( material mat)
{
    SEmaterial = mat;
}
material SceneElement::getMaterial()
{
    return SEmaterial;
}

CMeshBuffer *SceneElement::getMesh()
{
    return (CMeshBuffer *)Resources::getResource( MODEL, meshID );
}

Matrix &SceneElement::getMatrix()
{
    return transformation;
}

int SceneElement::Interation( SceneElement* e)
{
    return 0;
}
void SceneElement::Update( )
{
}
