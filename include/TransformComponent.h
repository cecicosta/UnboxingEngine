#pragma once
#include "Composite.h"

#include "Matrix.h"
#include "Quaternion.h"
#include "UVector.h"

namespace unboxing_engine {

class TransformComponent : public IComponent {
public:
    TransformComponent();
    ~TransformComponent() override = default;

    [[nodiscard]] TransformComponent* GetParent() const;
    [[nodiscard]] const Vector3f& GetPosition() const;
    [[nodiscard]] const Vector3f& GetScale() const;
    [[nodiscard]] const Quaternion& GetRotation() const;
    [[nodiscard]] const Matrix4f&GetTransformation() const;

    void SetParent(TransformComponent*);
    void SetPosition(const Vector3f&);
    void SetScale(const Vector3f&);
    void SetRotation(const Quaternion&);
    void SetTransformation(const Matrix4f&);

private:
    TransformComponent* m_parent;
    std::vector<TransformComponent*> m_children;
    Vector3f m_position;
    Vector3f m_scale;
    Quaternion m_rotation;
    Matrix4f m_transformation;
};

}// namespace unboxing_engine