#include "SceneNode.h"

namespace unboxing_engine {

void SceneNode::SetPosition(const Vector3f& position) {
    m_transform.SetPosition(position);
}

void SceneNode::SetScale(const Vector3f &scale) {
    m_transform.SetScale(scale);
}

void SceneNode::SetRotation(const Quaternion &rotation) {
    m_transform.SetRotation(rotation);
}

void SceneNode::SetRotation(float degrees, const Vector3f &axi) {
    m_transform.SetRotation(Quaternion(degrees, axi));
}

Vector3f SceneNode::GetScale() const {
    return m_transform.GetScale();
}

Vector3f SceneNode::GetPosition() const {
    return m_transform.GetPosition();
}

Quaternion SceneNode::GetRotation() const {
    return m_transform.GetRotation();
}

Matrix4f SceneNode::GetTransformation() const {
    return m_transform.GetTransformation();
}

void SceneNode::SetParent(SceneNode *parent) {
    m_parent = parent;
}

SceneNode *SceneNode::GetParent() {
    return m_parent;
}

std::vector<SceneNode*> SceneNode::GetChildren() {
    return m_children;
}
}// namespace unboxing_engine
