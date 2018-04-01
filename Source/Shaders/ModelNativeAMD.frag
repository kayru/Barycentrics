#version 450
#extension GL_AMD_shader_explicit_vertex_parameter : require

#include "Common.glsl"

layout (location = 0) in flat float v_IdFlat;
layout (location = 1) in __explicitInterpAMD float v_Id;

layout (location = 0) out vec4 fragColor0;

void main()
{
	int idRef = floatBitsToInt(v_IdFlat);
	int id0 = floatBitsToInt(interpolateAtVertexAMD(v_Id, 0));
	int id1 = floatBitsToInt(interpolateAtVertexAMD(v_Id, 1));
	int id2 = floatBitsToInt(interpolateAtVertexAMD(v_Id, 2));

	vec3 barycentrics;
	if (idRef == id0)
	{
		barycentrics.y = gl_BaryCoordSmoothAMD.x;
		barycentrics.z = gl_BaryCoordSmoothAMD.y;
		barycentrics.x = 1.0 - barycentrics.z - barycentrics.y;
	}
	else if (idRef == id1)
	{
		barycentrics.x = gl_BaryCoordSmoothAMD.x;
		barycentrics.y = gl_BaryCoordSmoothAMD.y;
		barycentrics.z = 1.0 - barycentrics.x - barycentrics.y;
	}
	else if (idRef == id2)
	{
		barycentrics.z = gl_BaryCoordSmoothAMD.x;
		barycentrics.x = gl_BaryCoordSmoothAMD.y;
		barycentrics.y = 1.0 - barycentrics.x - barycentrics.z;
	}
	else
	{
		barycentrics = vec3(1.0);
	}

	fragColor0.rgb = barycentrics;
	fragColor0.a = 1.0;
}
