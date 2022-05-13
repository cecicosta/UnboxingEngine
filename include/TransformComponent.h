#pragma once
#include "Composite.h"

#include "Matrix.h"
#include "Quaternion.h"
#include "Vector3Df.h"

namespace unboxing_engine {

class TransformComponent : public IComponent {
public:
    TransformComponent();
    ~TransformComponent() override = default;

private:
    Vector3Df m_position;
    Vector3Df m_scale;
    Quaternion m_rotation;
    Matrix m_transformation;
};

}// namespace unboxing_engine