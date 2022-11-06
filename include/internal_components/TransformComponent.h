#pragma once
#include "IComposite.h"

#include "IComponent.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "UVector.h"

namespace unboxing_engine {

class TransformComponent : public IComponent {
public:
    TransformComponent();
    ~TransformComponent() override = default;

    [[nodiscard]] const Vector3f& GetPosition() const;
    [[nodiscard]] const Vector3f& GetScale() const;
    [[nodiscard]] const Quaternion& GetRotation() const;
    [[nodiscard]] Matrix4f GetTransformation() const;

    void SetPosition(const Vector3f&);
    void SetScale(const Vector3f&);
    void SetRotation(const Quaternion&);

    void Translate(const Vector3f& position);
    void Scale(float scale);
    void Rotate(const Quaternion& rotation);
    void Rotate(float degrees, const Vector3f& axi);

private:
    Vector3f m_position;
    Vector3f m_scale;
    Quaternion m_rotation;
};

}// namespace unboxing_engine