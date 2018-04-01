#version 450

#if 0 // Reference mode

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

layout (location = 0) in vec2 v_tex0[];
layout (location = 1) in vec3 v_viewVector[];
layout (location = 2) in vec3 v_worldPos[];

layout (location = 0) out vec2 out_tex0;
layout (location = 1) out vec3 out_viewVector;
layout (location = 2) out flat vec3 out_worldPos0;
layout (location = 3) out flat vec3 out_worldPos1;
layout (location = 4) out flat vec3 out_worldPos2;

void main()
{
	for (int i=0; i<3; ++i)
	{
		gl_Position = gl_in[i].gl_Position;
		out_tex0 = v_tex0[i];
		out_viewVector = v_viewVector[i];
		out_worldPos0 = v_worldPos[0];
		out_worldPos1 = v_worldPos[1];
		out_worldPos2 = v_worldPos[2];
		EmitVertex();
	}
}

#else

#extension GL_NV_geometry_shader_passthrough : require

layout(triangles) in;

layout(passthrough) in gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0, passthrough) in vec2 in_tex0;
layout (location = 1, passthrough) in vec3 in_viewVector;
layout (location = 2) in vec3 in_worldPos[];

layout (location = 2) out vec3 v_worldPos0;
layout (location = 3) out vec3 v_worldPos1;
layout (location = 4) out vec3 v_worldPos2;

void main()
{
	v_worldPos0 = in_worldPos[0];
	v_worldPos1 = in_worldPos[1];
	v_worldPos2 = in_worldPos[2];
}

#endif
