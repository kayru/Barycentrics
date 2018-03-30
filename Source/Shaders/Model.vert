#version 450

#include "Common.glsl"

layout (location = 0) out vec2 v_tex0;
layout (location = 1) out vec3 v_nor0;
layout (location = 2) out vec3 v_worldPos;
layout (location = 3) out vec3 v_viewVector;
layout (location = 4) out vec3 v_worldPosFlat[3];

void main()
{
	uint index0 = g_indices[gl_VertexIndex];
	uint index1 = g_indices[gl_VertexIndex + 1];
	uint index2 = g_indices[gl_VertexIndex + 2];

	Vertex vertex0 = getVertex(index0);
	Vertex vertex1 = getVertex(index1);
	Vertex vertex2 = getVertex(index2);

	vec3 worldPos0 = (vec4(vertex0.position, 1) * g_matWorld).xyz;
	vec3 worldPos1 = (vec4(vertex1.position, 1) * g_matWorld).xyz;
	vec3 worldPos2 = (vec4(vertex2.position, 1) * g_matWorld).xyz;

	gl_Position = vec4(worldPos0, 1) * g_matViewProj;

	v_tex0 = vertex0.texcoord;
	v_nor0 = vertex0.normal;
	v_worldPos = worldPos0;
	v_viewVector = worldPos0 - g_cameraPos.xyz;

	v_worldPosFlat[0] = worldPos0;
	v_worldPosFlat[1] = worldPos1;
	v_worldPosFlat[2] = worldPos2;
}
