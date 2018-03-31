#version 450

#include "Common.glsl"

layout (location = 0) in vec2 v_barycentrics;
layout (location = 1) in flat uint v_primId;

layout (location = 0) out vec4 fragColor0;

void main()
{
	uint index0 = g_indices[v_primId*3];
	uint index1 = g_indices[v_primId*3+1];
	uint index2 = g_indices[v_primId*3+2];

	Vertex vertex0 = getVertex(index0);
	Vertex vertex1 = getVertex(index1);
	Vertex vertex2 = getVertex(index2);

	fragColor0.rgb = vec3(v_barycentrics, 1.0 - v_barycentrics.x - v_barycentrics.y);

	float b0 = v_barycentrics.x;
	float b1 = v_barycentrics.y;
	float b2 = 1.0 - b0 - b1;

	vec2 texcoord = vertex0.texcoord * b0 + vertex1.texcoord * b1 + vertex2.texcoord * b2;
	vec3 outBaseColor = g_baseColor.rgb * texture(albedoSampler, texcoord).rgb;
	
	fragColor0.rgb = mix(vec3(b0, b1, b2), outBaseColor.rgb, 0.5);
	fragColor0.a = 1;
}
