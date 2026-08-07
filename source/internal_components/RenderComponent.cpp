#include "internal_components/RenderComponent.h"

#include "Camera.h"
#include "MeshPrimitivesUtils.h"
#include "systems/IRenderSystem.h"

namespace unboxing_engine {
CDefaultMeshRenderComponent::CDefaultMeshRenderComponent(const CMeshBuffer &meshBuffer)
    : CRenderComponentBase(meshBuffer) {}

CDefaultMeshRenderComponent::~CDefaultMeshRenderComponent() {
    ReleaseRenderContext();
}

void CDefaultMeshRenderComponent::OnInitialize(systems::IRenderSystem & renderSystem) {
    mRenderSystem = &renderSystem;
    UpdateRenderContext();
}

void CDefaultMeshRenderComponent::ReleaseRenderContext() {
    if(mRenderSystem && mRenderContextHandle && mRenderContextHandle->renderBufferHandle) {
        mRenderSystem->EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
    }
    mRenderContextHandle.reset();
}

void CDefaultMeshRenderComponent::OnRender() {
    if (mIsDirty) {
        if(mRenderSystem && mRenderContextHandle && mRenderContextHandle->renderBufferHandle) {
            mRenderSystem->EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
        }
        UpdateRenderContext();
    }

    if(mRenderSystem && mRenderContextHandle) {
        mRenderSystem->Render(*mRenderContextHandle);
    }
}

void CDefaultMeshRenderComponent::UpdateRenderContext() {
    if(mRenderSystem) {
        const CMeshBuffer &meshBuffer = GetMeshBuffer();
        auto renderBufferHandle = mRenderSystem->WriteRenderBufferData(meshBuffer);
        auto shaderHandle = mRenderSystem->GetDefaultShader();
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(renderBufferHandle, shaderHandle, *mSceneComposite);
        mIsDirty = false;
    }
}

CustomShaderMeshRenderComponent::CustomShaderMeshRenderComponent(const CMeshBuffer &meshBuffer)
    : CDefaultMeshRenderComponent(meshBuffer) {
}

void CustomShaderMeshRenderComponent::SetVertexShader(const char *shader) {
    mVertexShader = shader;
}

void CustomShaderMeshRenderComponent::SetFragmentShader(const char *shader) {
    mFragmentShader = shader;
}
void CustomShaderMeshRenderComponent::OnInitialize(systems::IRenderSystem &renderSystem) {
    // TODO: See how to delete the shader later
    if (!mVertexShader.empty() && !mFragmentShader.empty()) {
        mShaderHandle = renderSystem.CompileShader(mVertexShader.c_str(), mFragmentShader.c_str());
    }
    if (!mShaderHandle) {
        mShaderHandle = renderSystem.GetDefaultShader();
    }
    CDefaultMeshRenderComponent::OnInitialize(renderSystem);
}

void CustomShaderMeshRenderComponent::UpdateRenderContext() {
    CDefaultMeshRenderComponent::UpdateRenderContext();
    if (mRenderContextHandle && mShaderHandle) {
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mShaderHandle,
                *mSceneComposite,
                nullptr,
                nullptr);
    }
}



static const char *quad_render_vertex_shader = R"(
#version 150 core

uniform vec4 color;
in vec2 i_position;
out vec2 v_uv;
out vec4 v_color;

void main() {
    v_uv = i_position + 0.5;
    v_color = color;
    gl_Position = vec4(i_position * 2.0, 0.0, 1.0);
}
)";


/**
 * Fragment shader for volumetric rendering of the line of sight of a radial potential region
 *
 * We need the min distance of the line-cross-section from the center, given by r_min = sqrt( (line_cross_section/2)^2 + r^2 ).
 * Next, we integrate the potential contribution for each point across the line-cross-section, until the minimum distance, which is halfway
 * from the sphere exit, and multiply by 2.
 */
static const char *potential_region_fragment_shader = R"(
#version 150 core

uniform sampler2D u_texture;
uniform float u_visualization_scale;
uniform float u_camera_far;

in vec4 v_color;
in vec2 v_uv;

out vec4 o_color;

// 1 picometer value in meters
float pm = 1e-12; //[m]

// Pi
float PI = 3.141592;

// Elementary charge
float e = 1.602e-19; // [C]

// Permittivity of free space
float E0 = 8.854e-12; // F/m [Farads per meter] 1C charge per 1V potential difference, every 1 metter

// Atomic unit of length
float a0 = 5.29177210544e-11; //[m] Borh radius (4\pi*\empsilon_0*\hbar^2) / (e^2 * m_e);

void main() {
    float maximumDistance = max(u_camera_far, 0.000001);

    vec4 accumulated = texture(u_texture, v_uv);
    // accumulated.rgb is the line-cross-section of the potential region obtained by the volumetric calculation fragment-shader pass
    float thickness = length(accumulated.rgb);
    float volumetric_opacity = (thickness/maximumDistance) * u_visualization_scale;
    float radius = accumulated.a / thickness;

    float sqr_half_ray = (thickness*thickness)/4;
    float sqr_radius = radius*radius;

    if(sqr_half_ray > sqr_radius) {
        o_color = v_color;
        return;
    }

    // Min distance the line-cross-section reach when crossing the sphere
    float r_min = sqrt(sqr_radius - sqr_half_ray);

    // Particle number of charges
    float Z = 1;

    // Calculate the constant factors of Coulomb's potential expression.
    float k = Z*(e*e)/(4*PI*E0);

    // We now convert the dimensions to picometer
    float radius_pm = radius*pm;
    float r_min_pm = r_min*pm;

    // By integrating the Coulomb's potential expression for point on the line-cross-section, we obtain the following expression
    float V = 2 * k * asinh(radius_pm/r_min_pm);

    // No normalize the potential decay to values between 0-1, we need a maximum potential sum contribution.
    // Using Bohr-radius as the radius reference for a maximum potential
    float V_max = 2 * k * asinh(radius_pm/pm); // In practice r is an effective infinite, in comparison to a0,

    float normalizedDecay = V/V_max;

    o_color = vec4(vec3(normalizedDecay, 0, 1-normalizedDecay), volumetric_opacity);
}
)";


RenderToTextureComponent::RenderToTextureComponent(const CMeshBuffer &meshBuffer)
: CustomShaderMeshRenderComponent(meshBuffer)
, mQuadMesh(primitive_utils::Quad()){
}

RenderToTextureComponent::~RenderToTextureComponent() {
    ReleaseRenderContext();
}

void RenderToTextureComponent::OnInitialize(systems::IRenderSystem &renderSystem) {
    mRenderSystem = &renderSystem;

    mTextureHandle = renderSystem.CreateTexture(
        renderSystem.GetCamera().mWidth,
        renderSystem.GetCamera().mHeight,
        systems::ETextureFormat::RGBA32F);
    if (!mTextureHandle) {
        return;
    }

    mRenderTarget = renderSystem.CreateTextureRenderTarget(
        mTextureHandle,
        systems::ERenderTargetKind::FloatingPointAccumulation);
    if (!mRenderTarget) {
        return;
    }

    CustomShaderMeshRenderComponent::OnInitialize(renderSystem);

    auto renderBufferHandle = mRenderSystem->WriteRenderBufferData(*mQuadMesh);
    auto shaderHandle = mRenderSystem->CompileShader(quad_render_vertex_shader, potential_region_fragment_shader);

    mQuadRenderContext = std::make_unique<systems::SRenderContextHandle>(
            renderBufferHandle,
            shaderHandle,
            *mSceneComposite,
            mTextureHandle);
}

void RenderToTextureComponent::UpdateRenderContext() {
    CustomShaderMeshRenderComponent::UpdateRenderContext();
    if (mRenderTarget && mRenderContextHandle) {
        mRenderContextHandle = std::make_unique<systems::SRenderContextHandle>(
                mRenderContextHandle->renderBufferHandle,
                mRenderContextHandle->shaderHandle,
                *mSceneComposite,
                nullptr,
                mRenderTarget);
    }
}

void RenderToTextureComponent::OnRender() {
    if (mIsDirty) {
        if(mRenderSystem && mRenderContextHandle && mRenderContextHandle->renderBufferHandle) {
            mRenderSystem->EraseRenderBufferData(*mRenderContextHandle->renderBufferHandle);
        }
        UpdateRenderContext();
    }

    if(mRenderSystem && mRenderContextHandle && mQuadRenderContext) {
        mRenderSystem->Render(*mRenderContextHandle);
        mRenderSystem->Render(*mQuadRenderContext);
    }
}

void RenderToTextureComponent::ReleaseRenderContext() {
    if (mRenderSystem && mQuadRenderContext && mQuadRenderContext->renderBufferHandle) {
        mRenderSystem->EraseRenderBufferData(*mQuadRenderContext->renderBufferHandle);
    }
    mQuadRenderContext.reset();
    CDefaultMeshRenderComponent::ReleaseRenderContext();
}


}// namespace unboxing_engine
