#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

layout (location = 0) in vec2 v_tex0[]; // unused

layout (location = 0) out vec2 v_barycentrics;
layout (location = 1) out uint v_primId;

void main()
{
	v_primId = gl_PrimitiveID;

	gl_Position = gl_in[0].gl_Position;
	v_barycentrics = vec2(1,0);
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	v_barycentrics = vec2(0,1);
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	v_barycentrics = vec2(0,0);
	EmitVertex();
}
