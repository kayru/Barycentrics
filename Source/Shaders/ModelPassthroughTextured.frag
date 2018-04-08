#version 450

#include "Common.glsl"

layout (location = 0) in vec2 v_tex0;
layout (location = 1) in vec3 v_viewVector;
layout (location = 2) in flat vec3 v_worldPos0;
layout (location = 3) in flat vec3 v_worldPos1;
layout (location = 4) in flat vec3 v_worldPos2;

layout (location = 0) out vec4 fragColor0;

void main()
{
	vec3 barycentrics = intersectRayTri(g_cameraPos.xyz,
		normalize(v_viewVector),
		v_worldPos0,
		v_worldPos1,
		v_worldPos2);

	vec2 texcoords = interpolateTexCoords(gl_PrimitiveID, barycentrics);
	fragColor0.rgb = texture(albedoSampler, texcoords).rgb;
	fragColor0.a = 1.0;
}
