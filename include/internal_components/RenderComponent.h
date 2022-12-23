#pragma once

#include "internal_components/IRenderComponent.h"
#include "IComponent.h"

namespace unboxing_engine {
class CMeshBuffer;

class CDefaultMeshRenderComponent : public IRenderComponent {
public:
    CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
        : mMeshBuffer(meshBuffer) {}
    ~CDefaultMeshRenderComponent() override = default;

    const CMeshBuffer &GetMeshBuffer() const override { return mMeshBuffer; }
    void Render() const override;

    const CSceneComposite *GetSceneComposite() const override { return mSceneComposite; }
    void OnAttached(CSceneComposite &sceneComposite) override { mSceneComposite = &sceneComposite; }
    void OnDetached() override { mSceneComposite = nullptr; };

private:
    const CMeshBuffer &mMeshBuffer;
    CSceneComposite *mSceneComposite;
};
}// namespace unboxing_engine