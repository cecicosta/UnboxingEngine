#pragma once


static const char *vertex_shader_source = R"(#version 150 core
in vec3 i_position;
uniform vec4 color;
out vec4 v_color;
uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;
void main() {
    v_color = color;
    gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(i_position, 1.0);
}
)";

static const char *fragment_shader_source = R"(#version 150
in vec4 v_color;
out vec4 o_color;
void main() {
    o_color = v_color;
}
)";

static const char *signed_texture_debug_vertex_shader_source = R"(
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

static const char *signed_texture_debug_fragment_shader_source = R"(#version 150 core
uniform sampler2D u_texture;
uniform float u_visualization_scale;
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
void main() {
    vec4 texture = texture(u_texture, v_uv);
    float volumetricOpacityValue = texture.a * u_visualization_scale;

    o_color = v_color * vec4(texture.rgb, volumetricOpacityValue);
}
)";


static const char *multiple_texture_fragment_shader = R"(
#version 150 core

uniform sampler2D u_texture;
uniform sampler2D u_electron;
uniform float u_visualization_scale;
in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;
void main() {
    vec4 texture1 = texture(u_texture, v_uv);
    vec4 texture2 = texture(u_electron, v_uv);
    float value = (texture2.a) * u_visualization_scale;

    vec3 signColor = value >= 0.0
        ? v_color.rgb
        : vec3(1.0, 1.0, 1.0) - v_color.rgb;

    float opacity = clamp(abs(value), 0.0, 1.0);
    o_color = vec4(signColor, opacity);
}
)";


static const char *bake_textures_2_fragment_shader = R"(
#version 150 core

uniform sampler2D u_tex_first;
uniform sampler2D u_tex_second;
uniform float u_visualization_scale;

in vec4 v_color;
in vec2 v_uv;
out vec4 o_color;

void main() {
    vec4 first = texture(u_tex_first, v_uv);
    vec4 second = texture(u_tex_second, v_uv);

    o_color = (first - second) * u_visualization_scale;
}
)";

static const char *vizualize_texture_fragment = R"(#version 150 core
uniform vec4 color;
in vec2 i_position;
out vec2 v_uv;
out vec4 v_color;


uniform mat4 u_model_matrix;
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

void main() {
    v_uv = i_position.xy + 0.5;
    v_color = color;
//    gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(i_position, 1.0);
    gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4(i_position * 2.0, 0.0, 1.0);
}
)";

static const char *vizualize_flow_fragment_shader = R"(
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

    float finalSample = (laplacian - nLaplacian);

    if(finalSample < 0) {
        discard;
    }

    o_color = vec4(1, 1, 1, 0.7);
}
)";

static const char *transform_vertex = R"(
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


static const char *quad_render_vertex = R"(
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

