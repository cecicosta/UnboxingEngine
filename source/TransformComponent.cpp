#include "TransformComponent.h"

namespace unboxing_engine {
TransformComponent::TransformComponent() : m_transformation(Matrix4f::Identity()) {
}
}// namespace unboxing_engine