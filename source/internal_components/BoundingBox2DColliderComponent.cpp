#include "internal_components/BoundingBox2DColliderComponent.h"

#include "SceneComposite.h"
#include "algorithms/CollisionAlgorithms.h"
#include "internal_components/IRenderComponent.h"
#include "internal_components/SegmentColliderComponent.h"

#include <assert.h>

namespace unboxing_engine {

void CBoxColliderComponent2D::OnIntersects() {
}

bool CBoxColliderComponent2D::HasCollided(const IColliderComponent& other) const {
    if (!mSceneComposite || !mMeshBuffer) {
        return false;
    }

    if (typeid(other).hash_code() == typeid(CBoxColliderComponent2D).hash_code() && (typeid(other).hash_code() == typeid(CSegmentColliderComponent).hash_code())) {
        assert(false && (std::string("Collision type not suported - &other type: ") + typeid(other).name()).c_str());
        return false;
    }

    std::vector<Vector3f> other_transformed_vertices;
    if (auto other_composite = other.GetSceneComposite();
        auto other_render = other_composite->GetComponent<IRenderComponent>()) {
        auto other_vertices = other_render->GetMeshBuffer().vertices;
        other_transformed_vertices = algorithms::ApplyTransformationToVerticesArray(other_vertices, other_composite->GetTransformation());
    } else {
        return false;
    }

    auto this_vertices = algorithms::ApplyTransformationToVerticesArray(mMeshBuffer->vertices, mSceneComposite->GetTransformation());
    auto result = unboxing_engine::algorithms::SCollisionResult<float, 3>();
    auto previous = this_vertices[3];
    for (int i = 0; i < this_vertices.size(); i++) {
        result = algorithms::checkPathIntersectionWithSegment<float, 3>(other_transformed_vertices, previous, this_vertices[i], true);
        if (result.vertices.size() > 0) {
            return true;
        }
        previous = this_vertices[i];
    }
    return false;
}

void CBoxColliderComponent2D::OnAttached(CSceneComposite& sceneComposite) {
    mSceneComposite = &sceneComposite;
    if (auto render = sceneComposite.GetComponent<IRenderComponent>()) {
        mMeshBuffer = &render->GetMeshBuffer();
    }
}

void CBoxColliderComponent2D::OnDetached() {
}

}// namespace unboxing_engine
