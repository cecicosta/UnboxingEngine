#include "SceneElement.h"

#include "TransformComponent.h"

void SceneElement::setPosition(vector3D position) {
    m_transformation = Matrix::translationMatrix(this->m_position * -1) * m_transformation;
    m_transformation = Matrix::translationMatrix(position) * m_transformation;
    this->m_position = position;
    this->reg.position = position;
}
void SceneElement::setRotation(double ang, vector3D eixo) {
    m_transformation = Matrix::translationMatrix(m_position * -1) * m_transformation;
    m_transformation = Matrix::rotationMatrix(ang, eixo) * m_transformation;
    m_transformation = Matrix::translationMatrix(m_position) * m_transformation;
    m_rotation = quaternion(ang, eixo) * m_rotation;
}
vector3D SceneElement::getScale() {
    return m_scale;
}
void SceneElement::setScale(vector3D scale) {
    m_transformation = m_transformation * Matrix::scaleMatrix(scale);
    this->m_scale = scale;
}

unboxing_engine::SceneElement::SceneElement()
    : m_transform(std::make_unique<unboxing_engine::TransformComponent>())
    , m_components(std::make_unique<unboxing_engine::Composite>()) {
}
