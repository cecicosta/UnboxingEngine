#include "internal_components/RenderComponent.h"

#include "Camera.h"
#include "MeshBuffer.h"
#include "SceneComposite.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace unboxing_engine::systems {

struct SShaderHandle {};
struct SRenderBufferHandle {};
struct STextureHandle {};
struct SRenderTarget {};

}

namespace {

class FakeRenderSystem final : public unboxing_engine::systems::IRenderSystem {
public:
    enum class EAction {
        CreateTexture,
        CreateRenderTarget,
        Render,
        EraseRenderBuffer
    };

    FakeRenderSystem()
        : camera(64, 32, 70.0f, 1.0f, 100.0f) {}

    bool Initialize() override { return true; }

    unboxing_engine::systems::SShaderHandle *CompileShader(
        const char *, const char *) const override {
        return &customShader;
    }

    unboxing_engine::systems::SRenderBufferHandle *WriteRenderBufferData(
        const unboxing_engine::CMeshBuffer &) override {
        ++writeRenderBufferCount;
        return &renderBuffers[writeRenderBufferCount - 1];
    }

    void EraseRenderBufferData(
        const unboxing_engine::systems::SRenderBufferHandle &handle) override {
        erasedBuffers.push_back(&handle);
        actions.push_back(EAction::EraseRenderBuffer);
    }

    const unboxing_engine::Camera &GetCamera() const override {
        return camera;
    }

    const unboxing_engine::systems::SShaderHandle *GetDefaultShader() const override {
        return &defaultShader;
    }

    const unboxing_engine::systems::SShaderHandle *GetTexturePresentationShader() const override {
        return &presentationShader;
    }

    void SetCamera(const unboxing_engine::Camera &newCamera) override {
        camera = newCamera;
    }

    void Render(const unboxing_engine::systems::SRenderContextHandle &context) override {
        renderContexts.push_back(&context);
        actions.push_back(EAction::Render);
    }

    unboxing_engine::systems::STextureHandle *CreateTexture(
        uint32_t width,
        uint32_t height,
        unboxing_engine::systems::ETextureFormat format) override {
        actions.push_back(EAction::CreateTexture);
        textureWidth = width;
        textureHeight = height;
        textureFormat = format;
        return createTextureResult ? &texture : nullptr;
    }

    unboxing_engine::systems::SRenderTarget *CreateTextureRenderTarget(
        unboxing_engine::systems::STextureHandle *textureHandle,
        unboxing_engine::systems::ERenderTargetKind kind) override {
        actions.push_back(EAction::CreateRenderTarget);
        targetTexture = textureHandle;
        targetKind = kind;
        return createRenderTargetResult ? &renderTarget : nullptr;
    }

    void OnPreRender() override {}
    void OnPostRender() override {}

    unboxing_engine::Camera camera;
    unboxing_engine::systems::SShaderHandle defaultShader;
    mutable unboxing_engine::systems::SShaderHandle customShader;
    unboxing_engine::systems::SShaderHandle presentationShader;
    unboxing_engine::systems::STextureHandle texture;
    unboxing_engine::systems::SRenderTarget renderTarget;
    unboxing_engine::systems::SRenderBufferHandle renderBuffers[2];

    bool createTextureResult = true;
    bool createRenderTargetResult = true;
    uint32_t textureWidth = 0;
    uint32_t textureHeight = 0;
    unboxing_engine::systems::ETextureFormat textureFormat =
        unboxing_engine::systems::ETextureFormat::RGBA8U;
    unboxing_engine::systems::STextureHandle *targetTexture = nullptr;
    unboxing_engine::systems::ERenderTargetKind targetKind =
        unboxing_engine::systems::ERenderTargetKind::DefaultFramebuffer;
    unsigned int writeRenderBufferCount = 0;
    std::vector<EAction> actions;
    std::vector<const unboxing_engine::systems::SRenderContextHandle *> renderContexts;
    std::vector<const unboxing_engine::systems::SRenderBufferHandle *> erasedBuffers;
};

unboxing_engine::CMeshBuffer MakeTestMesh() {
    unboxing_engine::CMeshBuffer mesh;
    mesh.nvertices = 3;
    mesh.nfaces = 1;
    mesh.vertices = {0.0f, 0.0f, 0.0f,
                     1.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f};
    mesh.triangles = {0, 1, 2};
    return mesh;
}

TEST(RenderToTextureComponentTest, CreatesTargetBeforeRendering) {
    FakeRenderSystem renderSystem;
    unboxing_engine::CSceneComposite scene;
    auto mesh = MakeTestMesh();
    auto component = std::make_unique<unboxing_engine::RenderToTextureComponent>(mesh);
    auto *componentPtr = component.get();
    scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));

    componentPtr->OnInitialize(renderSystem);

    ASSERT_EQ(renderSystem.textureWidth, 64u);
    ASSERT_EQ(renderSystem.textureHeight, 32u);
    EXPECT_EQ(renderSystem.textureFormat, unboxing_engine::systems::ETextureFormat::RGBA32F);
    EXPECT_EQ(renderSystem.targetTexture, &renderSystem.texture);
    EXPECT_EQ(renderSystem.targetKind,
              unboxing_engine::systems::ERenderTargetKind::FloatingPointAccumulation);
    ASSERT_EQ(renderSystem.renderContexts.size(), 0u);

    componentPtr->OnRender();

    ASSERT_EQ(renderSystem.renderContexts.size(), 1u);
    EXPECT_EQ(renderSystem.renderContexts[0]->renderTarget, &renderSystem.renderTarget);
    EXPECT_EQ(renderSystem.renderContexts[0]->textureHandle, nullptr);
}

TEST(RenderTextureComponentTest, RendersAssignedTexture) {
    FakeRenderSystem renderSystem;
    unboxing_engine::CSceneComposite scene;
    auto component = std::make_unique<unboxing_engine::RenderTextureComponent>();
    auto *componentPtr = component.get();
    scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));

    componentPtr->SetTexture(&renderSystem.texture);
    componentPtr->OnInitialize(renderSystem);
    componentPtr->OnRender();

    ASSERT_EQ(renderSystem.renderContexts.size(), 1u);
    EXPECT_EQ(renderSystem.renderContexts[0]->renderTarget, nullptr);
    EXPECT_EQ(renderSystem.renderContexts[0]->textureHandle, &renderSystem.texture);
    EXPECT_EQ(renderSystem.renderContexts[0]->shaderHandle, &renderSystem.presentationShader);
}

TEST(RenderToTextureComponentTest, ReleasesMeshAndPresentationBuffers) {
    FakeRenderSystem renderSystem;
    auto mesh = MakeTestMesh();

    {
        unboxing_engine::CSceneComposite scene;
        auto component = std::make_unique<unboxing_engine::RenderToTextureComponent>(mesh);
        auto *componentPtr = component.get();
        scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));
        componentPtr->OnInitialize(renderSystem);
    }

    ASSERT_EQ(renderSystem.erasedBuffers.size(), 2u);
    EXPECT_EQ(renderSystem.erasedBuffers[0], &renderSystem.renderBuffers[1]);
    EXPECT_EQ(renderSystem.erasedBuffers[1], &renderSystem.renderBuffers[0]);
}

TEST(RenderToTextureComponentTest, DoesNotRenderWhenTargetCreationFails) {
    FakeRenderSystem renderSystem;
    renderSystem.createRenderTargetResult = false;
    auto mesh = MakeTestMesh();
    unboxing_engine::CSceneComposite scene;
    auto component = std::make_unique<unboxing_engine::RenderToTextureComponent>(mesh);
    auto *componentPtr = component.get();
    scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));

    componentPtr->OnInitialize(renderSystem);
    componentPtr->OnRender();

    EXPECT_TRUE(renderSystem.renderContexts.empty());
}


}
