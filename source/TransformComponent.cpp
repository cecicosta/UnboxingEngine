#include "TransformComponent.h"

namespace unboxing_engine {
TransformComponent::TransformComponent() : m_transformation(Matrix::Identity(4)) {
}
}// namespace unboxing_engine