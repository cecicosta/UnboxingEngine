#pragma once


static const char *volumetric_scanning_frag = R"(
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



/**
 * Fragment shader for volumetric rendering of the line of sight of a radial potential region
 *
 * We need the min distance of the line-cross-section from the center, given by r_min = sqrt( (line_cross_section/2)^2 + r^2 ).
 * Next, we integrate the potential contribution for each point across the line-cross-section, until the minimum distance, which is halfway
 * from the sphere exit, and multiply by 2.
 */
static auto field_tex_render_frag = R"(
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

    if(volumetric_opacity < 0.00001) {
        discard;
    }

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
    float V = 2 * k * 1/r_min_pm; //asinh(radius_pm/r_min_pm);

    // No normalize the potential decay to values between 0-1, we need a maximum potential sum contribution.
    // Using Bohr-radius as the radius reference for a maximum potential
    float V_max = 2 * k * 1/pm; //asinh(radius_pm/pm); // In practice r is an effective infinite, in comparison to a0,

    float normalizedDecay = V;///V_max;

    o_color = vec4(vec3(v_color.r, v_color.g, v_color.b), normalizedDecay);
}
)";



static const char *combine_fields_tex_frag = R"(
#version 150 core

uniform sampler2D u_texture;
uniform sampler2D u_electron;
uniform float u_visualization_scale;
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
void main() {
    vec4 nucleusTex = texture(u_texture, v_uv);
    vec4 electronTex = texture(u_electron, v_uv);

    vec2 texel = 1.0 / vec2(textureSize(u_texture, 0));

    float left  = texture(u_texture, v_uv - vec2(texel.x, 0.0)).a;
    float right = texture(u_texture, v_uv + vec2(texel.x, 0.0)).a;
    float down  = texture(u_texture, v_uv - vec2(0.0, texel.y)).a;
    float up    = texture(u_texture, v_uv + vec2(0.0, texel.y)).a;

    float upLeft =
        texture(u_texture, v_uv + vec2(-texel.x, texel.y)).a;

    float upRight =
        texture(u_texture, v_uv + vec2(texel.x, texel.y)).a;

    float downLeft =
        texture(u_texture, v_uv + vec2(-texel.x, -texel.y)).a;

    float downRight =
        texture(u_texture, v_uv + vec2(texel.x, -texel.y)).a;

    vec2 nucleusGradient = vec2(
        (right - left) / (2.0 * texel.x),
        (up - down) / (2.0 * texel.y)
    );

    float nCenter = nucleusTex.a;

    float nAxial =
        left + right + up + down;

    float nDiagonal =
        upLeft + upRight + downLeft + downRight;

    float nLaplacian =
        (
            4.0 * nAxial +
            nDiagonal -
            20.0 * nCenter
        ) / 6.0;




    texel = 1.0 / vec2(textureSize(u_electron, 0));

    left  = texture(u_electron, v_uv - vec2(texel.x, 0.0)).a;
    right = texture(u_electron, v_uv + vec2(texel.x, 0.0)).a;
    down  = texture(u_electron, v_uv - vec2(0.0, texel.y)).a;
    up    = texture(u_electron, v_uv + vec2(0.0, texel.y)).a;

    upLeft =
        texture(u_electron, v_uv + vec2(-texel.x, texel.y)).a;

    upRight =
        texture(u_electron, v_uv + vec2(texel.x, texel.y)).a;

    downLeft =
        texture(u_electron, v_uv + vec2(-texel.x, -texel.y)).a;

    downRight =
        texture(u_electron, v_uv + vec2(texel.x, -texel.y)).a;

    vec2 electronGradient = vec2(
        (right - left) / (2.0 * texel.x),
        (up - down) / (2.0 * texel.y)
    );

    float center = electronTex.a;

    float axial =
        left + right + up + down;

    float diagonal =
        upLeft + upRight + downLeft + downRight;

    float laplacian =
        (
            4.0 * axial +
            diagonal -
            20.0 * center
        ) / 6.0;

    float diffusionStep = 0.3;

    float finalSample =
        center + diffusionStep * (laplacian - nLaplacian);

    o_color = vec4(v_color.rgb, finalSample);
}
)";
