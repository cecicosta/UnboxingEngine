#pragma once

#include "internal_components/TransformComponent.h"

#include <vector>

namespace unboxing_engine {

class SceneNode {
public:
    void SetPosition(const Vector3f& position);
    void SetScale(const Vector3f& scale);
    void SetRotation(const Quaternion& rotation);
    void SetRotation(float degrees, const Vector3f&axi);

    [[nodiscard]] Vector3f GetPosition() const;
    [[nodiscard]] Vector3f GetScale() const;
    [[nodiscard]] Quaternion GetRotation() const;
    [[nodiscard]] Matrix4f GetTransformation() const;

    void SetParent(SceneNode* parent);

    SceneNode* GetParent();
    std::vector<SceneNode*> GetChildren();

protected:
    SceneNode* m_parent = nullptr;
    std::vector<SceneNode*> m_children;
    TransformComponent m_transform;
};
}// namespace unboxing_engine
