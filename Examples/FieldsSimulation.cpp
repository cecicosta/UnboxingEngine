#pragma once
#include "FieldsSimulationShaders.h"
#include "SceneCompositeLibrary/BasicSceneCompositeTypes.h"
#include "ShaderLibrary.h"
#include "UnboxingEngine.h"
#include "algorithms/CollisionPrimitives.h"

constexpr float E0 = 8.854e-12; // F/m [Farads per meter] 1C charge per 1V potential difference, every 1 metter

unboxing_engine::systems::STextureInspectionOptions options {1e-20};


using namespace unboxing_engine;



/**
 * Calculates the energy differential by distance as the definition of force.
 * F = dE/dx
 */
float EnergyDifferential(const float dx) {

}
float Force(const float delta_x) {

}

// We want initially simulate a time independent system of a single hydrogen atom in the ground state
// I. Follow the definition for the nucleus potential
// II. We do not intend to make time evolution, so render the probability space of the particles
// III. For this, we will need to render the electron on its probability space
// IV. The radius of a particle must be dynamic. The geometry might be centered on its peak of probability,
// but it extends up until its current state allows. The region shift might occur, as wave packet translation.
// The influence of external fields must be passed as parameter for the particle shader.

/**
 * This class models fields which affects particles across space
 */
struct ESpaceField {
    std::function<float(float radius, float azimuth, float bearing)> field;
    Vector3f position;
    // Distance from the scenter from which the field calculation is valid (planet surface, black hole event horizon, nucleus radius).
    float horizon;
};


class CMouseTrackingSphere : public CustomShaderComposite, public UListener<core_events::IMouseInputEvent> {
public:
    CMouseTrackingSphere(CCore& engine, float radius) : CustomShaderComposite(*primitive_utils::Sphere(radius)), mEngine(engine){
    }

    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        if (!cursor.isButtonPressed) {
            return;
        }
        float currentZ = GetPosition().z;
        Vector3f outPoint, outDirection;
        auto camera = mEngine.GetCamera();
        camera.CastRayFromScreen(cursor.buttonPressedX, cursor.buttonPressedY, outPoint, outDirection);
        float x = outPoint.x + outDirection.x*(camera.GetPosition().z - currentZ);
        float y = outPoint.y + outDirection.y*(camera.GetPosition().z - currentZ);
        SetPosition(Vector3f{x, y, currentZ});
    }
private:
    CCore& mEngine;
};


class SimpleTextureComposite : public CSceneComposite {
    public:
    SimpleTextureComposite() {
        auto render = std::make_unique<RenderTextureComponent>();
        render->SetPolygonMode(EPolygonMode::Fill);
        AddComponent<IRenderComponent>(std::move(render));
    }

    void SetTexture(systems::STextureHandle* texture) {
        if (const auto render = GetComponent<RenderTextureComponent>()) {
            render->SetTexture(texture);
        }
    }
};

class RenderToTexture : public CSceneComposite {
public:
    explicit RenderToTexture(CCore& engine, const CMeshBuffer& mesh) : mEngine(engine), mMesh(mesh) {
        auto render = std::make_unique<RenderToTextureComponent>(mMesh);
        render->SetPolygonMode(EPolygonMode::Fill);
        AddComponent<IRenderComponent>(std::move(render));
        mEngine.RegisterSceneElement(*this);
    }

    ~RenderToTexture() override {
        mEngine.UnregisterSceneElement(*this);
    }

    [[nodiscard]] RenderToTextureComponent& GetRenderComponent() const {
        if (auto render = dynamic_cast<RenderToTextureComponent*>(GetComponent<IRenderComponent>())) {
            return *render;
        }
        assert(false);
    }

    void SetMaterial(const SMaterial &material) {
        auto render = GetComponent<IRenderComponent>();
        render->SetMaterial(material);
    }

    void SetShader (const char* vertex, const char* fragment) {
        if (auto render = dynamic_cast<RenderToTextureComponent*>(GetComponent<IRenderComponent>())) {
            render->SetVertexShader(vertex);
            render->SetFragmentShader(fragment);
        } else {
            assert(false);
        }
    }

    void SetTexture(const std::string& name, systems::STextureHandle* texture) {
        if (const auto render = GetComponent<IRenderComponent>()) {
            dynamic_cast<RenderToTextureComponent*>(render)->SetSrcTexture(name, texture);
        }
    }

    [[nodiscard]] systems::STextureHandle* GetTexture() const {
        if(const auto render = GetComponent<IRenderComponent>()) {
            return dynamic_cast<RenderToTextureComponent*>(render)->GetDstTexture();
        }
        return nullptr;
    }


    void SetClearRenderTarget (bool clear) {
        if (auto render = dynamic_cast<RenderToTextureComponent*>(GetComponent<IRenderComponent>())) {
            render->SetRenderTargetClearEnabled(clear);
        } else {
            assert(false);
        }
    }

    void SetRenderTargetBlendMode(systems::ERenderTargetBlendMode blendMode) {
        if (auto render = dynamic_cast<RenderToTextureComponent*>(GetComponent<IRenderComponent>())) {
            render->SetRenderTargetBlendMode(blendMode);
        } else {
            assert(false);
        }
    }
private:
    CCore& mEngine;
    CMeshBuffer mMesh;
};


class RenderTexture : public CSceneComposite {
public:
    explicit RenderTexture(CCore& engine, const CMeshBuffer& mesh) : mEngine(engine), mMesh(mesh) {
        auto render = std::make_unique<RenderTextureComponent>();
        render->SetMaterial(yellowMaterial());
        render->SetPolygonMode(EPolygonMode::Fill);
        AddComponent<IRenderComponent>(std::move(render));
        mEngine.RegisterSceneElement(*this);
    }

    ~RenderTexture() override {
        mEngine.UnregisterSceneElement(*this);
    }

    [[nodiscard]] RenderTextureComponent& GetRenderComponent() const {
        if (auto render = dynamic_cast<RenderTextureComponent*>(GetComponent<IRenderComponent>())) {
            return *render;
        }
        assert(false);
    }

    void SetMaterial(const SMaterial &material) {
        auto render = GetComponent<IRenderComponent>();
        render->SetMaterial(material);
    }

    void SetShader (const char* vertex, const char* fragment) {
        if (auto render = dynamic_cast<RenderTextureComponent*>(GetComponent<IRenderComponent>())) {
            render->SetVertexShader(vertex);
            render->SetFragmentShader(fragment);
        } else {
            assert(false);
        }
    }

    [[nodiscard]] systems::STextureHandle* GetTexture() const {
        if(const auto render = GetComponent<RenderTextureComponent>()) {
            return nullptr; //render->GetDstTexture();
        }
        return nullptr;
    }

    void SetTexture(const std::string& name, systems::STextureHandle* texture) {
        if (const auto render = GetComponent<IRenderComponent>()) {
            dynamic_cast<RenderTextureComponent*>(render)->SetTexture(name, texture);
        }
    }


private:
    CCore& mEngine;
    CMeshBuffer mMesh;
};


class OnMousePressDetector : public CSceneComposite, public UListener<core_events::IMouseInputEvent> {
public:
    OnMousePressDetector(CCore& engine, RenderToTexture* first, RenderToTexture* second ) : mFirst(first), mSecond(second), mEngine(engine){
        mEngine.RegisterSceneElement(*this);
    }
    ~OnMousePressDetector() override {
        mEngine.UnregisterSceneElement(*this);
    }

    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        if (!cursor.isButtonPressed) {
            return;
        }
        mFirst->SetTexture("u_electron", mSecond->GetTexture());
        mSecond->SetTexture("u_electron", mFirst->GetTexture());
        mFirst->SetClearRenderTarget(false);
        mSecond->SetClearRenderTarget(false);

    }
private:
    CCore& mEngine;
    RenderToTexture* mFirst;
    RenderToTexture* mSecond;
};

int main(int argc, char *argv[]) {
    unboxing_engine::CCore engine(640, 480, 32);
    Camera camera(720, 720, 60, 1, 300);
    camera.SetPerspectiveProjection();
    camera.SetPosition({0, 0, 100});
    engine.SetCamera(camera);
    engine.Start();

    float radius = 40;

    {

        /***********
         * NUCLEUS *
         ***********/
        RenderToTexture nucleus1st(engine,*primitive_utils::Sphere(radius));
        nucleus1st.SetShader(transform_vertex, volumetric_scanning_frag);

        RenderToTexture nucleus2nd(engine, *primitive_utils::Quad());
        nucleus2nd.SetMaterial(whiteMaterial());
        nucleus2nd.SetTexture("u_texture", nucleus1st.GetTexture());
        nucleus2nd.SetShader(quad_render_vertex, nucleus_field_tex_render_frag);

        /************
         * ELECTRON *
         ************/
        RenderToTexture electron1st(engine, *primitive_utils::Sphere(radius/4));
        electron1st.SetPosition({0, 20, 0});
        electron1st.SetShader(transform_vertex, volumetric_scanning_frag);

        RenderToTexture electron2nd(engine, *primitive_utils::Quad());
        electron2nd.SetTexture("u_texture", electron1st.GetTexture());
        electron2nd.SetMaterial(whiteMaterial());
        electron2nd.SetShader(quad_render_vertex, nucleus_field_tex_render_frag);

        engine.StepRender();

        const auto* texture = electron2nd.GetTexture();
        const auto& debug = engine.GetRenderDebug();
        debug.PrintTextureStatistics(*texture, "field scale 1", options);


        /**
         * PING-PONG FIELD FLOW
         */

        // The following creates a ping-pong texture feedback loop for update the electron field.
        // Both objects use overwrite blend to replace the fragment values at each iteration by the new calculated
        // value from the change rate of the local gradient.
        RenderToTexture ping(engine, *primitive_utils::Quad());
        ping.SetRenderTargetBlendMode(systems::ERenderTargetBlendMode::Overwrite);
        ping.SetTexture("u_texture", nucleus2nd.GetTexture());
        ping.SetTexture("u_electron", electron2nd.GetTexture());
        ping.SetMaterial(redMaterial());
        ping.SetShader(quad_render_vertex, fields_divergence_frag);

        RenderToTexture pong(engine, *primitive_utils::Quad());
        pong.SetRenderTargetBlendMode(systems::ERenderTargetBlendMode::Overwrite);
        pong.SetTexture("u_texture", nucleus2nd.GetTexture());
        pong.SetTexture("u_electron", ping.GetTexture());
        pong.SetMaterial(redMaterial());
        pong.SetShader(quad_render_vertex, fields_divergence_frag);

        OnMousePressDetector onClickStartPingPongTextureFeedback(engine, &ping, &pong);

        RenderToTexture flow(engine, *primitive_utils::Quad());
        flow.SetTexture("u_electron", pong.GetTexture());
        flow.SetMaterial(blueMaterial());
        flow.SetShader(quad_render_vertex, vizualize_flow_fragment_shader);
        flow.GetRenderComponent().SetRenderColorScale(1e16);

/*
        RenderTexture nucleus(engine, *primitive_utils::Quad());
        nucleus.SetTexture("u_texture", nucleus2nd.GetTexture());
        nucleus.SetScale(Vector3f(20, 20, 20));
        nucleus.SetPosition(Vector3f(-20, 0, 0));
        nucleus.SetMaterial(whiteMaterial());
        nucleus.SetShader(vizualize_texture_fragment, signed_texture_debug_fragment_shader_source);
        //nucleus.GetRenderComponent().SetRenderColorScale(1.5e15);
*/
        RenderTexture electron(engine, *primitive_utils::Quad());
        electron.SetTexture("u_texture", electron2nd.GetTexture());
        electron.SetScale(Vector3f(20, 20, 20));
        electron.SetPosition(Vector3f(-20, 0, 0));
        electron.SetMaterial(whiteMaterial());
        electron.SetShader(vizualize_texture_fragment, r_as_alpha);
        //electron.GetRenderComponent().SetRenderColorScale(1.5e15);

        RenderTexture rightSide(engine, *primitive_utils::Quad());
        rightSide.SetTexture("u_texture", flow.GetTexture());
        rightSide.SetScale(Vector3f(20, 20, 20));
        rightSide.SetPosition(Vector3f(20, 0, 0));
        rightSide.SetMaterial(whiteMaterial());
        rightSide.SetShader(vizualize_texture_fragment, signed_texture_debug_fragment_shader_source);

        engine.Run();
    }
    engine.Release();
    return 0;
}
