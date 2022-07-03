#include "TransformComponent.h"

namespace unboxing_engine {
TransformComponent::TransformComponent() {
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

Matrix4f TransformComponent::GetTransformation() const {
    auto transformation = Matrix4f::Identity();
    transformation = Matrix4f::RotationMatrix(m_rotation) * transformation;
    transformation = Matrix4f::TranslationMatrix(m_position) * transformation;
    transformation = Matrix4f::ScaleMatrix(m_scale) * transformation;
    return transformation;
}

void TransformComponent::SetPosition(const Vector3f &position) {
    m_position = position;
}

void TransformComponent::SetScale(const Vector3f &scale) {
    m_scale = scale;
}

void TransformComponent::SetRotation(const Quaternion &rotation) {
    m_rotation = rotation;
}
void TransformComponent::Translate(const Vector3f &position) {
    m_position = m_position + position;
}
void TransformComponent::Scale(float scale) {
    m_scale = scale * m_scale;
}
void TransformComponent::Rotate(const Quaternion &rotation) {
    m_rotation =  m_rotation * rotation;
}
void TransformComponent::Rotate(float degrees, const Vector3f &axi) {
    m_rotation = m_rotation * Quaternion(degrees, axi);
}
}// namespace unboxing_engine