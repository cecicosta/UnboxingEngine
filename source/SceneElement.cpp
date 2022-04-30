#include "SceneElement.h"
#include "resources.h"
SceneElement::SceneElement(){}
SceneElement::SceneElement( sphere reg, vector3D position, int meshID, int texID, int type, int id, bool illuminated)
{
    this->reg = reg;
    this->m_position = position;
    this->meshID = meshID;
    this->texID = texID;
    this->type = type;
    this->nivel = nivel;
    this->id = id;
    this->m_rotation = quaternion(0, vector3D(1,0,0));
    this->m_scale = vector3D(1,1,1);
    this->SEmaterial.Illuminated(illuminated);
    this->m_transformation = Matrix::translationMatrix(position);
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
    return m_position;
}
void SceneElement::setPosition(vector3D position)
{
    m_transformation = Matrix::translationMatrix(this->m_position *-1)* m_transformation;
    m_transformation = Matrix::translationMatrix(position)* m_transformation;
    this->m_position = position;
    this->reg.position = position;
}
void SceneElement::setRotation( double ang, vector3D eixo)
{
    m_transformation = Matrix::translationMatrix(m_position *-1)* m_transformation;
    m_transformation = Matrix::rotationMatrix( ang, eixo )* m_transformation;
    m_transformation = Matrix::translationMatrix(m_position)* m_transformation;
    m_rotation = quaternion( ang, eixo )* m_rotation;
}
vector3D SceneElement::getScale()
{
    return m_scale;
}
void SceneElement::setScale( vector3D scale )
{
    m_transformation = m_transformation *Matrix::scaleMatrix(scale);
    this->m_scale = scale;
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
    return m_transformation;
}

int SceneElement::Interation( SceneElement* e)
{
    return 0;
}
void SceneElement::Update( )
{
}
