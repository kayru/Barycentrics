#version 450

#include "Common.glsl"

layout (location = 0) in vec2 v_tex0;
layout (location = 1) in vec3 v_nor0;
layout (location = 2) in vec3 v_worldPos;
layout (location = 3) in vec3 v_viewVector;
layout (location = 4) flat in vec3 v_worldPosFlat[3];

layout (location = 0) out vec4 fragColor0;

struct Ray
{
	vec3 o;
	vec3 d;
};

void intersectRayTri(Ray r, vec3 v0, vec3 e0, vec3 e1, out vec3 outBarycentrics, out bool outHit)
{
	const vec3 s1 = cross(r.d, e1);
	const float invd = 1.0 / (dot(s1, e0));
	const vec3 d = r.o - v0;
	const float b1 = dot(d, s1) * invd;
	const vec3 s2 = cross(d, e0);
	const float b2 = dot(r.d, s2) * invd;
	const float temp = dot(e1, s2) * invd;

	outBarycentrics.x = b1;
	outBarycentrics.y = b2;
	outBarycentrics.z = 1.0 - b1 - b2;

	if (b1 < 0.0 || b1 > 1.0 || b2 < 0.0 || b1 + b2 > 1.0 || temp < 0)
	{
		outHit = false;
	}
	else
	{
		outHit = true;
	}
}

void main()
{
	vec4 outBaseColor = vec4(0.0);

	outBaseColor = g_baseColor * texture(albedoSampler, v_tex0);

	Ray ray;
	ray.o = g_cameraPos.xyz;
	ray.d = normalize(v_viewVector);

	vec3 v0 = v_worldPosFlat[0];
	vec3 e0 = v_worldPosFlat[1] - v0;
	vec3 e1 = v_worldPosFlat[2] - v0;

	vec3 barycentrics = vec3(0.0);
	bool hit = false;
	intersectRayTri(ray, v0, e0, e1, barycentrics, hit);

	fragColor0.rgb = barycentrics;

	fragColor0.a = 1;
}
