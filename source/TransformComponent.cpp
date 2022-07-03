#include "TransformComponent.h"

namespace unboxing_engine {
TransformComponent::TransformComponent()
    : m_parent(nullptr), m_transformation(Matrix4f::Identity()) {
}
TransformComponent *TransformComponent::GetParent() const {
    return m_parent;
}
const Vector3f &TransformComponent::GetPosition() const {
    return m_position;
}
const Vector3f &TransformComponent::GetScale() const {
    return m_scale;
}
const Quaternion &TransformComponent::GetRotation() const {
    return m_rotation;
}
const Matrix4f &TransformComponent::GetTransformation() const {
    return m_transformation;
}
void TransformComponent::SetParent(TransformComponent *parent) {
    m_parent = parent;
}
void TransformComponent::SetPosition(const Vector3f &position) {
    m_transformation = Matrix4f::TranslationMatrix(this->m_position * -1) * m_transformation;
    m_transformation = Matrix4f::TranslationMatrix(position) * m_transformation;
    this->m_position = position;
    m_position = position;
}
void TransformComponent::SetScale(const Vector3f &scale) {
    m_scale = scale;
}
void TransformComponent::SetRotation(const Quaternion &rotation) {
    m_rotation = rotation;
}
void TransformComponent::SetTransformation(const Matrix4f &transformation) {
    m_transformation = transformation;
}
}// namespace unboxing_engine