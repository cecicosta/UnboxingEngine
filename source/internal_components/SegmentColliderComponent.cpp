#include "internal_components/SegmentColliderComponent.h"

#include "internal_components/IRenderComponent.h"
#include "internal_components/BoundingBox2DColliderComponent.h"
#include "algorithms/CollisionAlgorithms.h"
#include "SceneComposite.h"

namespace unboxing_engine {

unboxing_engine::CSegmentColliderComponent::CSegmentColliderComponent() = default;
CSegmentColliderComponent::~CSegmentColliderComponent() = default;

bool CSegmentColliderComponent::HasCollided(const IColliderComponent &other) const {
    if (typeid(other).hash_code() == typeid(CBoxColliderComponent2D).hash_code()) {
        return HasCollided(dynamic_cast<const CBoxColliderComponent2D&>(other));
    }
    if (typeid(other).hash_code() == typeid(CSegmentColliderComponent).hash_code()) {
        return HasCollided(dynamic_cast<const CSegmentColliderComponent &>(other));
    }
    assert(false && (std::string("Collision type not suported - &other type: ") + typeid(other).name()).c_str());
    return false;
}

const CSceneComposite *CSegmentColliderComponent::GetSceneComposite() const {
    return mSceneComposite;
}

void CSegmentColliderComponent::OnAttached(CSceneComposite &sceneComposite) {
    mSceneComposite = &sceneComposite;
    if (auto render = sceneComposite.GetComponent<IRenderComponent>()) {
        mMeshBuffer = &render->GetMeshBuffer();
    }
}

void CSegmentColliderComponent::OnDetached() {
    mSceneComposite = nullptr;
}

void CSegmentColliderComponent::OnIntersects() {

}

bool CSegmentColliderComponent::HasCollided(const CBoxColliderComponent2D &other) const {
    if (!mSceneComposite || !mMeshBuffer) {
        return false;
    }

    std::vector<Vector3f> other_transformed_vertices;
    if (auto other_composite = other.GetSceneComposite(); 
        auto other_render = other_composite->GetComponent<IRenderComponent>()) {        
        other_transformed_vertices = algorithms::ApplyTransformationToVerticesArray(other_render->GetMeshBuffer().vertices, other_composite->GetTransformation());
        assert(other_transformed_vertices.size() == 4 && "Geometry invalid for CBoxColliderComponent2D");
    } else {
        return false;
    }
    
    auto this_vertices = algorithms::ApplyTransformationToVerticesArray(mMeshBuffer->vertices, mSceneComposite->GetTransformation()); 
    auto result = this_vertices.size() >= 2 ? algorithms::checkPathIntersectionWithSegment<float, 3>(other_transformed_vertices, this_vertices[0], this_vertices[1], true) 
        : unboxing_engine::algorithms::SCollisionResult<float, 3>();
    return result.vertices.size() > 0;
}

bool CSegmentColliderComponent::HasCollided(const CSegmentColliderComponent &other) const {
    if (!mSceneComposite || !mMeshBuffer) {
        return false;
    }

    std::vector<Vector3f> other_transformed_vertices;
    if (auto other_composite = other.GetSceneComposite();
        auto other_render = other_composite->GetComponent<IRenderComponent>()) {
        auto other_vertices = other_render->GetMeshBuffer().vertices;
        other_transformed_vertices = algorithms::ApplyTransformationToVerticesArray(other_vertices, other_composite->GetTransformation());
        assert(other_transformed_vertices.size() == 3 && "Geometry invalid for CSegmentColliderComponent");
    } else {
        return false;
    }

    auto this_vertices = algorithms::ApplyTransformationToVerticesArray(mMeshBuffer->vertices, mSceneComposite->GetTransformation());
    auto result = this_vertices.size() == 2 ? algorithms::checkPathIntersectionWithSegment<float, 3>(other_transformed_vertices, this_vertices[0], this_vertices[1], true)
                                            : unboxing_engine::algorithms::SCollisionResult<float, 3>();
    return result.vertices.size() > 0;
    return false;
}

}// namespace unboxing_engine