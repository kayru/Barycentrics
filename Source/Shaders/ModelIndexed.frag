#version 450

#include "Common.glsl"

layout (location = 0) in vec2 v_tex0;
layout (location = 0) out vec4 fragColor0;

void main()
{
	vec3 baseColor = g_baseColor.rgb * texture(albedoSampler, v_tex0).rgb;
	fragColor0.rgb = baseColor.rgb;
	fragColor0.a = 1;
}
