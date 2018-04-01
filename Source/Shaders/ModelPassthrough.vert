#version 450

#include "Common.glsl"

layout (location = 0) in vec3 a_pos0;
layout (location = 1) in vec3 a_nor0;
layout (location = 2) in vec2 a_tex0;

layout (location = 0) out vec2 v_tex0;
layout (location = 1) out vec3 v_viewVector;
layout (location = 2) out vec3 v_worldPos;

void main()
{
	vec3 worldPos = (vec4(a_pos0, 1) * g_matWorld).xyz;
	gl_Position = vec4(worldPos, 1) * g_matViewProj;
	v_tex0 = a_tex0;
	v_viewVector = worldPos - g_cameraPos.xyz;
	v_worldPos = worldPos;
}
