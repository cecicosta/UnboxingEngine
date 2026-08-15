#include "MeshPrimitivesUtils.h"
#include "SceneComposite.h"
#include "ShaderLibrary.h"
#include "UnboxingEngine.h"
#include "internal_components/RenderComponent.h"
#include "systems/IRenderDebug.h"

#include <iostream>
#include <memory>

using namespace unboxing_engine;

namespace {

constexpr const char* fieldFragmentShader = R"(#version 150 core
uniform float u_visualization_scale;
in vec2 v_uv;
out vec4 o_color;

void main() {
    float radius = length(v_uv - vec2(0.5));
    float field = max(0.0, 1.0 - radius * 2.0) * u_visualization_scale;
    o_color = vec4(field, field * 0.5, 0.0, field);
}
)";

SMaterial FieldMaterial() {
    SMaterial material{};
    material.materialDif[0] = 1.0f;
    material.materialDif[1] = 1.0f;
    material.materialDif[2] = 1.0f;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}

class CDebugFieldTarget : public CSceneComposite {
public:
    CDebugFieldTarget()
        : mMesh(primitive_utils::Quad()) {
        auto render = std::make_unique<RenderToTextureComponent>(*mMesh);
        render->SetMaterial(FieldMaterial());
        render->SetPolygonMode(EPolygonMode::Fill);
        render->SetVertexShader(signed_texture_debug_vertex_shader_source);
        render->SetFragmentShader(fieldFragmentShader);
        mRender = render.get();
        AddComponent<IRenderComponent>(std::move(render));
    }

    [[nodiscard]] systems::STextureHandle* GetTexture() const {
        return mRender->GetDstTexture();
    }

    void SetFieldScale(const float scale) {
        mRender->SetRenderColorScale(scale);
    }

private:
    std::unique_ptr<CMeshBuffer> mMesh;
    RenderToTextureComponent* mRender = nullptr;
};

} // namespace

int main() {
    CCore engine(320, 240, 32);
    engine.Start();
    if (!engine.IsRunning()) {
        engine.Release();
        return 1;
    }

    int exitCode = 0;
    {
        CDebugFieldTarget fieldTarget;
        engine.RegisterSceneElement(fieldTarget);

        systems::STextureInspectionOptions options;
        options.nonZeroEpsilon = 1e-6;

        fieldTarget.SetFieldScale(1.0f);
        engine.StepRender();

        const auto* texture = fieldTarget.GetTexture();
        const auto& debug = engine.GetRenderDebug();
        if (!texture) {
            std::cerr << "Could not create the render-target texture.\n";
            exitCode = 1;
        } else {
            debug.PrintTextureStatistics(*texture, "field scale 1", options);
            const auto previous = debug.CaptureTexture(*texture, options);

            fieldTarget.SetFieldScale(0.75f);
            engine.StepRender();
            const auto current = debug.CaptureTexture(*texture, options);

            if (!previous || !current) {
                std::cerr << "Could not capture the render-target texture.\n";
                exitCode = 1;
            } else {
                const auto difference = systems::CompareTextureSnapshots(
                    *previous,
                    *current,
                    options);
                if (!difference) {
                    std::cerr << "Could not compare the texture snapshots.\n";
                    exitCode = 1;
                } else {
                    std::cout << systems::FormatTextureDifferenceStatistics(
                        *difference,
                        "field scale 1 -> 0.75");
                }
            }
        }

        engine.UnregisterSceneElement(fieldTarget);
    }

    engine.Release();
    return exitCode;
}
