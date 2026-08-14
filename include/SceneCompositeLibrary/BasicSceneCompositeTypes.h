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

inline SMaterial someMaterial() {
    SMaterial material{};
    material.materialDif[0] = 0.5f;
    material.materialDif[1] = 0.74f;
    material.materialDif[2] = 0.2f;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}

inline SMaterial otherMaterial() {
    SMaterial material{};
    material.materialDif[0] = 0.74f;
    material.materialDif[1] = 0.5f;
    material.materialDif[2] = 0.2f;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}

inline SMaterial anotherMaterial() {
    SMaterial material{};
    material.materialDif[0] = 0.5f;
    material.materialDif[1] = 0.2f;
    material.materialDif[2] = 0.74f;
    material.materialDif[3] = 1.0f;
    material.enable = true;
    return material;
}


inline SMaterial whiteMaterial() {
    SMaterial material{};
    material.materialDif[0] = 1;
    material.materialDif[1] = 1;
    material.materialDif[2] = 1;
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



static const char *customVertexShader = R"(
#version 150 core

in vec3 i_position;
uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
uniform vec4 color;

out vec3 vWorldPosition;
out vec3 vViewPosition;
out vec3 vLocalPosition;
out vec4 v_color;
out float vRadius;
void main()
{
    vec4 worldPosition = u_model_matrix * vec4(i_position, 1.0);
    vec4 viewPosition = u_view_matrix * worldPosition;

    vLocalPosition = i_position;
    vWorldPosition = worldPosition.xyz;
    vViewPosition = viewPosition.xyz;
    v_color = color;

    vRadius = length(i_position);
    gl_Position = u_projection_matrix * viewPosition;
}
)";

static const char *customFragmentShader = R"(
#version 150 core

in vec3 vWorldPosition;
in vec3 vViewPosition;
in vec3 vLocalPosition;
in vec4 v_color;
in float vRadius;

uniform vec3 u_camera_world_position;
uniform float u_camera_far;

out vec4 FragColor;

void main()
{
    float maximumDistance = max(u_camera_far, 0.000001);
    vec3 ray = u_camera_world_position - vWorldPosition;
    float distanceFromCamera = length(ray);

    float signValue = gl_FrontFacing ? -1.0 : 1.0;

    FragColor = vec4(
        signValue * ray,
        signValue * distanceFromCamera * vRadius //Encodes the radius value r*(dNear - dFar)
    );
}
)";



class CustomShaderComposite : public CSceneComposite {
public:
    explicit CustomShaderComposite(const CMeshBuffer &mesh)
        : mMesh(mesh) {
        auto render = std::make_unique<RenderToTextureComponent>(mMesh);
        render->SetMaterial(yellowMaterial());
        render->SetPolygonMode(EPolygonMode::Fill);

        render->SetVertexShader(customVertexShader);
        render->SetFragmentShader(customFragmentShader);

        AddComponent<IRenderComponent>(std::move(render));
    }

    void SetMaterial(const SMaterial &material) {
        auto render = GetComponent<IRenderComponent>();
        render->SetMaterial(material);
    }

private:
    CMeshBuffer mMesh;
};




static const char *electron_fragment_shader = R"(
#version 330 core

in vec3 vLocalPosition; // Position relative to electron's reference
in vec4 v_color;

vec3 vNucleusCenter(0, 0, 0);

// Elementary charge
float e = 1.602e-19; // [C]

// Permittivity of free space
float E0 = 8.854e-12; // F/m [Farads per meter] 1C charge per 1V potential difference, every 1 metter

// Atomic unit of length
float a0 = 5.29177210544e-11; //[m] Borh radius (4\pi*\empsilon_0*\hbar^2) / (e^2 * m_e);

out vec4 FragColor;

void main()
{
    float PI = 3.141592;

    // We cannot simply use the 3D vector length as all the points of the sphere sits on ints surface
    float r = length(vLocalPosition.xy)*(a0);

    float V0 = (e*e)/(4*PI*E0*a0);
    float V = (e*e)/(4*PI*E0*r);

    // Get the inverse of the decay, since we want fade the color
    float decay = 1 - V/V0;

    // There will always be at least 2 fragments of the sphere at the same projected position.
    // As the points get near the radius, there will be proportionally more points compressed together
    // Their alpha will be summed up on the z buffer. We either must account for that, or use solid colors
    FragColor = vec4(
        v_color.rgb - v_color.rgb*decay,
        v_color.a
    );
}
)";


constexpr float PI = 3.141592653589793238462643383279;
constexpr float kEpsilon = 1e-10;

// Permittivity of free space
constexpr float vacuum_permittivity = 8.854e-12; // F/m [Farads per meter] 1C charge per 1V potential difference, every 1 metter
// Elementary charge
constexpr float elementary_charge = 1.602e-19; // [C]

/**
 * Calculates the value of the potential at a given radius (r [m]) from the charge (z [C]).
 * param r - Radius of the shell, in meters, for which to calculate the radial potential.
 * param z - Elementary charge the particle or group of particles, in Coulombs, generating the field possesses.
 */
inline float CoulombRadialPotential(const float r, const int z) {
    return (static_cast<float>(z)*pow(elementary_charge, 2.0f))/(4*PI*vacuum_permittivity*r);
}

inline float KineticEnergy(const float m, const Vector3f& v) {
    return 0.5f * v.DotProduct(v) * m; // 1/2 mv^2
}





}// namespace unboxing_engine
