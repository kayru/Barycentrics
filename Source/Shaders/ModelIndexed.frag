#version 450

#include "Common.glsl"

layout (location = 0) in vec2 v_tex0;
layout (location = 2) in vec3 v_viewVector;

layout (location = 0) out vec4 fragColor0;

void main()
{
	if (g_useTexture)
	{
		fragColor0.rgb = texture(albedoSampler, v_tex0).rgb;
	}
	else
	{
		fragColor0.rgb = vec3(v_tex0, 0.0);
	}

	fragColor0.a = 1;
}
