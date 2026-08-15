#include "internal_components/RenderComponent.h"

#include "Camera.h"
#include "MeshBuffer.h"
#include "SceneComposite.h"
#include "systems/IRenderDebug.h"

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

class FakeRenderSystem final
    : public unboxing_engine::systems::IRenderSystem
    , public unboxing_engine::systems::IRenderDebug {
public:
    enum class EAction {
        CreateTexture,
        CreateRenderTarget,
        Render,
        EraseRenderBuffer,
        EraseShader,
        EraseTexture,
        EraseRenderTarget
    };

    FakeRenderSystem()
        : camera(64, 32, 70.0f, 1.0f, 100.0f) {}

    bool Initialize() override { return true; }

    unboxing_engine::systems::SShaderHandle *CompileShader(
        const char *, const char *) const override {
        return &customShaders[compileShaderCount++];
    }

    void EraseShaderData(
        const unboxing_engine::systems::SShaderHandle &handle) override {
        erasedShaders.push_back(&handle);
        actions.push_back(EAction::EraseShader);
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

    const unboxing_engine::systems::IRenderDebug &GetRenderDebug() const override {
        return *this;
    }

    std::optional<unboxing_engine::systems::STextureStatistics> InspectTexture(
        const unboxing_engine::systems::STextureHandle &,
        const unboxing_engine::systems::STextureInspectionOptions &) const override {
        return std::nullopt;
    }

    bool PrintTextureStatistics(
        const unboxing_engine::systems::STextureHandle &,
        const std::string &,
        const unboxing_engine::systems::STextureInspectionOptions &) const override {
        return false;
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

    void EraseTextureData(
        const unboxing_engine::systems::STextureHandle &handle) override {
        erasedTextures.push_back(&handle);
        actions.push_back(EAction::EraseTexture);
    }

    unboxing_engine::systems::SRenderTarget *CreateTextureRenderTarget(
        unboxing_engine::systems::STextureHandle *textureHandle,
        unboxing_engine::systems::ERenderTargetKind kind) override {
        actions.push_back(EAction::CreateRenderTarget);
        targetTexture = textureHandle;
        targetKind = kind;
        return createRenderTargetResult ? &renderTarget : nullptr;
    }

    void SetRenderTargetClearEnabled(
        unboxing_engine::systems::SRenderTarget &target,
        const bool enabled) override {
        clearTarget = &target;
        renderTargetClearEnabled = enabled;
        ++setRenderTargetClearEnabledCount;
    }

    void EraseRenderTargetData(
        const unboxing_engine::systems::SRenderTarget &handle) override {
        erasedRenderTargets.push_back(&handle);
        actions.push_back(EAction::EraseRenderTarget);
    }

    void OnPreRender() override {}
    void OnPostRender() override {}

    unboxing_engine::Camera camera;
    unboxing_engine::systems::SShaderHandle defaultShader;
    mutable unboxing_engine::systems::SShaderHandle customShaders[4];
    unboxing_engine::systems::SShaderHandle presentationShader;
    unboxing_engine::systems::STextureHandle texture;
    unboxing_engine::systems::STextureHandle secondTexture;
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
    mutable unsigned int compileShaderCount = 0;
    unboxing_engine::systems::SRenderTarget *clearTarget = nullptr;
    bool renderTargetClearEnabled = true;
    unsigned int setRenderTargetClearEnabledCount = 0;
    std::vector<EAction> actions;
    std::vector<const unboxing_engine::systems::SRenderContextHandle *> renderContexts;
    std::vector<const unboxing_engine::systems::SRenderBufferHandle *> erasedBuffers;
    std::vector<const unboxing_engine::systems::SShaderHandle *> erasedShaders;
    std::vector<const unboxing_engine::systems::STextureHandle *> erasedTextures;
    std::vector<const unboxing_engine::systems::SRenderTarget *> erasedRenderTargets;
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
    EXPECT_TRUE(renderSystem.renderContexts[0]->textureBindings.empty());
}

TEST(RenderToTextureComponentTest, ConfiguresTargetClearingBeforeAndAfterInitialization) {
    FakeRenderSystem renderSystem;
    unboxing_engine::CSceneComposite scene;
    auto mesh = MakeTestMesh();
    auto component = std::make_unique<unboxing_engine::RenderToTextureComponent>(mesh);
    auto *componentPtr = component.get();
    scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));

    componentPtr->SetRenderTargetClearEnabled(false);
    componentPtr->OnInitialize(renderSystem);

    EXPECT_EQ(renderSystem.clearTarget, &renderSystem.renderTarget);
    EXPECT_FALSE(renderSystem.renderTargetClearEnabled);
    EXPECT_EQ(renderSystem.setRenderTargetClearEnabledCount, 1u);

    componentPtr->SetRenderTargetClearEnabled(true);

    EXPECT_TRUE(renderSystem.renderTargetClearEnabled);
    EXPECT_EQ(renderSystem.setRenderTargetClearEnabledCount, 2u);
}

TEST(RenderTextureComponentTest, RendersAssignedTextures) {
    FakeRenderSystem renderSystem;
    unboxing_engine::CSceneComposite scene;
    auto component = std::make_unique<unboxing_engine::RenderTextureComponent>();
    auto *componentPtr = component.get();
    scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));

    componentPtr->SetTexture("u_accumulation", &renderSystem.texture);
    componentPtr->SetTexture("u_noise", &renderSystem.secondTexture);
    componentPtr->OnInitialize(renderSystem);
    componentPtr->OnRender();

    ASSERT_EQ(renderSystem.renderContexts.size(), 1u);
    EXPECT_EQ(renderSystem.renderContexts[0]->renderTarget, nullptr);
    ASSERT_EQ(renderSystem.renderContexts[0]->textureBindings.size(), 2u);
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[0].uniformName, "u_accumulation");
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[0].texture, &renderSystem.texture);
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[1].uniformName, "u_noise");
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[1].texture, &renderSystem.secondTexture);
    EXPECT_EQ(renderSystem.renderContexts[0]->shaderHandle, &renderSystem.presentationShader);
}

TEST(RenderTextureComponentTest, ReplacesTextureWithSameUniformName) {
    FakeRenderSystem renderSystem;
    unboxing_engine::CSceneComposite scene;
    auto component = std::make_unique<unboxing_engine::RenderTextureComponent>();
    auto *componentPtr = component.get();
    scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));

    componentPtr->SetTexture("u_texture", &renderSystem.texture);
    componentPtr->SetTexture("u_texture", &renderSystem.secondTexture);
    componentPtr->OnInitialize(renderSystem);
    componentPtr->OnRender();

    ASSERT_EQ(renderSystem.renderContexts.size(), 1u);
    ASSERT_EQ(renderSystem.renderContexts[0]->textureBindings.size(), 1u);
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[0].uniformName, "u_texture");
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[0].texture, &renderSystem.secondTexture);
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

TEST(RenderToTextureComponentTest, ReleasesAllOwnedRenderResources) {
    FakeRenderSystem renderSystem;
    auto mesh = MakeTestMesh();

    {
        unboxing_engine::CSceneComposite scene;
        auto component = std::make_unique<unboxing_engine::RenderToTextureComponent>(mesh);
        auto *componentPtr = component.get();
        scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));
        componentPtr->SetVertexShader("vertex");
        componentPtr->SetFragmentShader("fragment");
        componentPtr->OnInitialize(renderSystem);
    }

    ASSERT_EQ(renderSystem.erasedShaders.size(), 2u);
    EXPECT_EQ(renderSystem.erasedShaders[0], &renderSystem.customShaders[1]);
    EXPECT_EQ(renderSystem.erasedShaders[1], &renderSystem.customShaders[0]);
    ASSERT_EQ(renderSystem.erasedRenderTargets.size(), 1u);
    EXPECT_EQ(renderSystem.erasedRenderTargets[0], &renderSystem.renderTarget);
    ASSERT_EQ(renderSystem.erasedTextures.size(), 1u);
    EXPECT_EQ(renderSystem.erasedTextures[0], &renderSystem.texture);
}

TEST(RenderToTextureComponentTest, ReleasesTextureWhenTargetCreationFails) {
    FakeRenderSystem renderSystem;
    renderSystem.createRenderTargetResult = false;
    auto mesh = MakeTestMesh();

    {
        unboxing_engine::CSceneComposite scene;
        auto component = std::make_unique<unboxing_engine::RenderToTextureComponent>(mesh);
        auto *componentPtr = component.get();
        scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));
        componentPtr->OnInitialize(renderSystem);
    }

    EXPECT_TRUE(renderSystem.erasedRenderTargets.empty());
    ASSERT_EQ(renderSystem.erasedTextures.size(), 1u);
    EXPECT_EQ(renderSystem.erasedTextures[0], &renderSystem.texture);
}

TEST(RenderTextureComponentTest, DoesNotReleaseBorrowedResources) {
    FakeRenderSystem renderSystem;

    {
        unboxing_engine::CSceneComposite scene;
        auto component = std::make_unique<unboxing_engine::RenderTextureComponent>();
        auto *componentPtr = component.get();
        scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));
        componentPtr->SetTexture(&renderSystem.texture);
        componentPtr->OnInitialize(renderSystem);
    }

    EXPECT_TRUE(renderSystem.erasedShaders.empty());
    EXPECT_TRUE(renderSystem.erasedTextures.empty());
    EXPECT_TRUE(renderSystem.erasedRenderTargets.empty());
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

TEST(RenderToTextureComponentTest, RendersWithAssignedSourceTextures) {
    FakeRenderSystem renderSystem;
    unboxing_engine::CSceneComposite scene;
    auto mesh = MakeTestMesh();
    auto component = std::make_unique<unboxing_engine::RenderToTextureComponent>(mesh);
    auto *componentPtr = component.get();
    scene.AddComponent<unboxing_engine::IRenderComponent>(std::move(component));

    componentPtr->SetSrcTexture("u_previous", &renderSystem.texture);
    componentPtr->SetSrcTexture("u_mask", &renderSystem.secondTexture);
    componentPtr->OnInitialize(renderSystem);
    componentPtr->OnRender();

    ASSERT_EQ(renderSystem.renderContexts.size(), 1u);
    ASSERT_EQ(renderSystem.renderContexts[0]->textureBindings.size(), 2u);
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[0].uniformName, "u_previous");
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[0].texture, &renderSystem.texture);
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[1].uniformName, "u_mask");
    EXPECT_EQ(renderSystem.renderContexts[0]->textureBindings[1].texture, &renderSystem.secondTexture);
}


}
