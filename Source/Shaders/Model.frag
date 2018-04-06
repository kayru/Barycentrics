#version 450

#include "Common.glsl"

layout (location = 0) in vec2 v_barycentrics;
layout (location = 1) in flat uint v_primId;
layout (location = 2) in vec3 v_viewVector;

layout (location = 0) out vec4 fragColor0;

void main()
{
	vec3 barycentrics = vec3(v_barycentrics.x, v_barycentrics.y, 1.0 - v_barycentrics.x - v_barycentrics.y);

	vec2 texcoords = interpolateTexCoords(v_primId, barycentrics);	
	vec3 textured = texture(albedoSampler, texcoords).rgb;

	fragColor0.rgb = barycentrics;
	//fragColor0.rgb = textured;
	fragColor0.a = 1;
}
