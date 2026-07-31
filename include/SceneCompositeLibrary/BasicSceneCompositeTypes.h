#pragma once

#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "UnboxingEngine.h"
#include "internal_components/RenderComponent.h"

namespace unboxing_engine {
inline SMaterial yellowMaterial() {
    SMaterial material{};
    material.materialDif[0] = 1;
    material.materialDif[1] = 1;
    material.materialDif[2] = 0;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}

inline SMaterial blueMaterial() {
    SMaterial material{};
    material.materialDif[0] = 0;
    material.materialDif[1] = 0;
    material.materialDif[2] = 1;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}


inline SMaterial greenMaterial() {
    SMaterial material{};
    material.materialDif[0] = 0;
    material.materialDif[1] = 1;
    material.materialDif[2] = 0;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}


inline SMaterial redMaterial() {
    SMaterial material{};
    material.materialDif[0] = 1;
    material.materialDif[1] = 0;
    material.materialDif[2] = 0;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}

class CSegment
    : public CSceneComposite {
public:
    CSegment(CCore &engine, const SMaterial &material)
        : mMesh(primitive_utils::Lines(mRefStart, mRefEnd))
        , mEngine(engine) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(*mMesh);
        render->SetMaterial(material);
        render->SetPolygonMode(EPolygonMode::Line);
        AddComponent<IRenderComponent>(std::move(render));
        UpdateSegment(Vector3f(), Vector3f());
    }

    ~CSegment() override = default;

    void UpdateSegment(const Vector3f &start, const Vector3f &end) {
        const Vector3f direction = (end - start).Normalized();
        float length = (end - start).Length();

        const Vector3f initialDirection = (mRefEnd - mRefStart).Normalized();

        const float cos = initialDirection.DotProduct(direction);
        const float angle = acosf(cos);
        const Vector3f axis = initialDirection.CrossProduct(direction);

        SetRotation(Quaternion(angle * (180.0f / pi), axis.Normalized()));
        SetPosition(start);
        SetScale(Vector3f(length, length, length));
        mStart = start;
        mEnd = end;
    }

    void SetMaterial(const SMaterial &material) {
        auto render = GetComponent<IRenderComponent>();
        render->SetMaterial(material);
    }

    [[nodiscard]] Vector3f GetStart() const {
        return GetTransformation() * mStart;
    }

    [[nodiscard]] Vector3f GetEnd() const {
        return GetTransformation() * mEnd;
    }

private:
    const Vector3f mRefStart{0, 0, 0};
    const Vector3f mRefEnd{0, 1, 0};
    Vector3f mStart, mEnd;
    std::unique_ptr<CMeshBuffer> mMesh;
    CCore &mEngine;
};

class CSimpleMeshWireFrame : public CSceneComposite {
public:
    explicit CSimpleMeshWireFrame(const CMeshBuffer &mesh)
        : mMesh(mesh) {
        auto render = std::make_unique<CDefaultMeshRenderComponent>(mMesh);
        render->SetMaterial(yellowMaterial());
        render->SetPolygonMode(EPolygonMode::Line);
        AddComponent<IRenderComponent>(std::move(render));
    }

    void SetMaterial(const SMaterial &material) {
        auto render = GetComponent<IRenderComponent>();
        render->SetMaterial(material);
    }

private:
    CMeshBuffer mMesh;
};
}// namespace unboxing_engine
