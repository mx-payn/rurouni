#ifndef RR_LIB_GRAPHICS_DEFAULT_SHADER_H
#define RR_LIB_GRAPHICS_DEFAULT_SHADER_H

// rurouni
#include "rurouni/graphics/vertex_buffer.hpp"

namespace rr::graphics {

static const std::string DEFAULT_QUAD_SHADER_VERTEX_SRC = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in int a_EntityId;

uniform mat4 u_ViewProjection;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;
out flat int v_EntityId;

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
layout(location = 1) out int entityId;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;
in flat int v_EntityId;

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
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in int a_EntityID;

uniform mat4 u_ViewProjection;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
    float TexIndex;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	Output.TexIndex = a_TexIndex;
	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
)";

static const std::string DEFAULT_TEXT_SHADER_FRAGMENT_SRC = R"(
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
    float TexIndex;
};

layout (location = 0) in VertexOutput Input;
layout (location = 3) in flat int v_EntityID;

uniform sampler2D u_Textures[5];

float screenPxRange() {
	const float pxRange = 2.0; // set to distance field's pixel range
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_Textures[int(Input.TexIndex)], 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(Input.TexCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
	vec4 texColor = Input.Color * texture(u_Textures[int(Input.TexIndex)], Input.TexCoord);

	vec3 msd = texture(u_Textures[int(Input.TexIndex)], Input.TexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (opacity == 0.0)
		discard;

	vec4 bgColor = vec4(0.0);
    o_Color = mix(bgColor, Input.Color, opacity);
	if (o_Color.a == 0.0)
		discard;

	o_EntityID = v_EntityID;
}
)";

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_DEFAULT_SHADER_H
