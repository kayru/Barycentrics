#version 450

#include "Common.glsl"

layout (location = 0) in vec3 a_pos0;
layout (location = 1) in vec2 a_tex0;

layout (location = 0) out float v_IdFlat;
layout (location = 1) out float v_Id;

void main()
{
	vec3 worldPos = (vec4(a_pos0, 1) * g_matWorld).xyz;
	gl_Position = vec4(worldPos, 1) * g_matViewProj;

	float id = intBitsToFloat(gl_VertexIndex);
	v_IdFlat = id;
	v_Id = id;
}
