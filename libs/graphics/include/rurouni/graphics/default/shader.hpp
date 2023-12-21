#ifndef RR_LIB_GRAPHICS_DEFAULT_SHADER_H
#define RR_LIB_GRAPHICS_DEFAULT_SHADER_H

// rurouni
#include "rurouni/graphics/vertex_buffer.hpp"

namespace rr::graphics {

static const std::string DEFAULT_POST_FX_SHADER_VERTEX_SRC = R"(
#version 330 core
layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoords;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(a_Position.x, a_Position.y, 0.0, 1.0);
    TexCoords = a_TexCoords;
}
)";

static const std::string DEFAULT_POST_FX_SHADER_FRAGMENT_SRC = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    FragColor = texture(screenTexture, TexCoords);
}
)";

static const std::string DEFAULT_QUAD_SHADER_VERTEX_SRC = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in uint a_EntityId;

uniform mat4 u_ViewProjection;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;
out flat uint v_EntityId;

void main()
{
  v_Color = a_Color;
  v_TexCoord = a_TexCoord;
  v_TexIndex = a_TexIndex;
  v_EntityId = a_EntityId;
  gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";

static const std::string DEFAULT_QUAD_SHADER_FRAGMENT_SRC = R"(
#version 430 core

layout(location = 0) out vec4 color;
layout(location = 1) out uint entityId;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;
in flat uint v_EntityId;

uniform sampler2D u_Textures[32];

void main()
{
   color = texture(u_Textures[int(v_TexIndex)], v_TexCoord) * v_Color;
  if(color.a < 0.1)
       discard;

  entityId = v_EntityId;
}
)";

static const std::string DEFAULT_TEXT_SHADER_VERTEX_SRC = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;

out vec2 v_TexCoord;

void main()
{
  v_TexCoord = a_TexCoord;
  gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";

static const std::string DEFAULT_TEXT_SHADER_FRAGMENT_SRC = R"(
#version 430 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
  color = texture(u_Texture, v_TexCoord);
  if(color.a < 0.1)
       discard;
}
)";

static const std::string DEFAULT_MSDF_SHADER_VERTEX_SRC = R"(
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;

out vec2 v_TexCoord;
out vec4 v_Color;

void main()
{
	v_TexCoord = a_TexCoord;
    v_Color = a_Color;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";

static const std::string DEFAULT_MSDF_SHADER_FRAGMENT_SRC = R"(
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_Texture;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

// source: https://medium.com/@calebfaith/implementing-msdf-font-in-opengl-ea09a9ab7e00
void main() {
    vec3 msd = texture(u_Texture, v_TexCoord).rgb;
    ivec2 sz = textureSize(u_Texture, 0).xy;
    float dx = dFdx(v_TexCoord.x) * sz.x; 
    float dy = dFdy(v_TexCoord.y) * sz.y;
    float toPixels = 8.0 * inversesqrt(dx * dx + dy * dy);
    float sigDist = median(msd.r, msd.g, msd.b);
    float w = fwidth(sigDist);
    float opacity = smoothstep(0.5 - w, 0.5 + w, sigDist);

    o_Color = vec4(v_Color.rgb, opacity);
}
)";

static const std::string DEFAULT_SDF_SHADER_VERTEX_SRC = R"(
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;

out vec2 v_TexCoord;
out vec4 v_Color;

void main()
{
	v_TexCoord = a_TexCoord;
    v_Color = a_Color;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";

static const std::string DEFAULT_SDF_SHADER_FRAGMENT_SRC = R"(
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_Texture;

void main() {
    float c = texture(u_Texture, v_TexCoord).r;

    if (c > 0.5)
        o_Color = vec4(1, 1, 1, 1) * v_Color;
    else 
        o_Color = vec4(0, 0, 0, 0);
}
)";

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_DEFAULT_SHADER_H
