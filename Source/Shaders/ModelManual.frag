#version 450

#include "Common.glsl"

layout (location = 0) in flat vec3 v_worldPos;
layout (location = 1) in vec3 v_viewVector;

layout (location = 0) out vec4 fragColor0;

void main()
{
	uint index1 = g_indices[gl_PrimitiveID*3+1];
	uint index2 = g_indices[gl_PrimitiveID*3+2];

	Vertex vertex1 = getVertex(index1);
	Vertex vertex2 = getVertex(index2);

	vec3 barycentrics = intersectRayTri(g_cameraPos.xyz,
		normalize(v_viewVector),
		v_worldPos,
		(vec4(vertex1.position, 1) * g_matWorld).xyz,
		(vec4(vertex2.position, 1) * g_matWorld).xyz);
	
	vec2 texcoords = interpolateTexCoords(gl_PrimitiveID, barycentrics);	
	vec3 textured = texture(albedoSampler, texcoords).rgb;

	fragColor0.rgb = barycentrics;
	//fragColor0.rgb = textured;
	fragColor0.a = 1;
}
