#include "SceneNode.h"

#include <algorithm>

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

Matrix4f CSceneNode::GetWorldTransformation() const {
    if (m_parent) {
        return m_parent->GetWorldTransformation() * GetTransformation();
    }
    return GetTransformation();
}

void CSceneNode::SetParent(CSceneNode *parent) {
    if (m_parent == parent) {
        return;
    }

    if (m_parent) {
        auto &siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    m_parent = parent;
    if (m_parent && std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this) == m_parent->m_children.end()) {
        m_parent->m_children.push_back(this);
    }
}

CSceneNode *CSceneNode::GetParent() {
    return m_parent;
}

const CSceneNode *CSceneNode::GetParent() const {
    return m_parent;
}

std::vector<CSceneNode*> CSceneNode::GetChildren() {
    return m_children;
}
}// namespace unboxing_engine
