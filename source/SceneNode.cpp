#include "SceneNode.h"

namespace unboxing_engine {

void CSceneNode::SetPosition(const Vector3f& position) {
    m_transform.SetPosition(position);
}

void CSceneNode::SetScale(const Vector3f &scale) {
    m_transform.SetScale(scale);
}

void CSceneNode::SetRotation(const Quaternion &rotation) {
    m_transform.SetRotation(rotation);
}

void CSceneNode::SetRotation(float degrees, const Vector3f &axi) {
    m_transform.SetRotation(Quaternion(degrees, axi));
}

Vector3f CSceneNode::GetScale() const {
    return m_transform.GetScale();
}

Vector3f CSceneNode::GetPosition() const {
    return m_transform.GetPosition();
}

Quaternion CSceneNode::GetRotation() const {
    return m_transform.GetRotation();
}

Matrix4f CSceneNode::GetTransformation() const {
    return m_transform.GetTransformation();
}

void CSceneNode::SetParent(CSceneNode *parent) {
    m_parent = parent;
}

CSceneNode *CSceneNode::GetParent() {
    return m_parent;
}

std::vector<CSceneNode*> CSceneNode::GetChildren() {
    return m_children;
}
}// namespace unboxing_engine
