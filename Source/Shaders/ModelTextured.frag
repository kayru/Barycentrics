#version 450

#include "Common.glsl"

layout (location = 0) in vec2 v_tex0;
layout (location = 2) in vec3 v_viewVector;

layout (location = 0) out vec4 fragColor0;

void main()
{
	fragColor0 = texture(albedoSampler, v_tex0);
}
