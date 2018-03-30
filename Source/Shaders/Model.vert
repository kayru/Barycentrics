#version 450

#include "Common.glsl"

layout (location = 0) out vec2 v_tex0;
layout (location = 1) out vec3 v_nor0;
layout (location = 2) out vec3 v_worldPos;
layout (location = 3) out vec3 v_viewVector;
layout (location = 4) out vec2 v_barycentrics;
layout (location = 5) out uint v_primId;

void main()
{
	uint index0 = g_indices[gl_VertexIndex];
	Vertex vertex0 = getVertex(index0);
	vec3 worldPos0 = (vec4(vertex0.position, 1) * g_matWorld).xyz;

	gl_Position = vec4(worldPos0, 1) * g_matViewProj;

	v_tex0 = vertex0.texcoord;
	v_nor0 = vertex0.normal;
	v_worldPos = worldPos0;
	v_viewVector = worldPos0 - g_cameraPos.xyz;

	uint id = gl_VertexIndex%3;
	switch(id)
	{
	case 0: v_barycentrics = vec2(1,0); break;
	case 1: v_barycentrics = vec2(0,1); break;
	case 2: v_barycentrics = vec2(0,0); break;
	}

	v_primId = gl_VertexIndex / 3;
}
