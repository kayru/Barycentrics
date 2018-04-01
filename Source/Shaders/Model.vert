#version 450

#include "Common.glsl"

layout (location = 0) out vec2 v_barycentrics;
layout (location = 1) out uint v_primId;
layout (location = 2) out vec3 v_viewVector;

void main()
{
	uint index = g_indices[gl_VertexIndex];
	Vertex vertex = getVertex(index);
	vec3 worldPos = (vec4(vertex.position, 1) * g_matWorld).xyz;

	gl_Position = vec4(worldPos, 1) * g_matViewProj;

	uint id = gl_VertexIndex%3;
	switch(id)
	{
	case 0: v_barycentrics = vec2(1,0); break;
	case 1: v_barycentrics = vec2(0,1); break;
	case 2: v_barycentrics = vec2(0,0); break;
	}

	v_primId = gl_VertexIndex / 3;
	v_viewVector = worldPos - g_cameraPos.xyz;
}
