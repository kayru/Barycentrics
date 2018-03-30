layout (binding = 0) uniform Global
{
	mat4 g_matView;
	mat4 g_matProj;
	mat4 g_matViewProj;
	mat4 g_matWorld;
	vec4 g_cameraPos;
};

layout (binding = 1) uniform Material
{
	vec4 g_baseColor;
};

layout (binding = 2) uniform sampler2D albedoSampler;

struct VertexPacked
{
	float pX, pY, pZ;
	float nX, nY, nZ;
	float tX, tY;
};

layout (std430, binding = 3) readonly buffer VertexBuffer
{
	VertexPacked g_vertices[];
};

layout (std430, binding = 4) readonly buffer IndexBuffer
{
	uint g_indices[];
};

struct Vertex
{
	vec3 position;
	vec3 normal;
	vec2 texcoord;
};

Vertex getVertex(uint i)
{
	VertexPacked v = g_vertices[i];

	Vertex r;
	r.position = vec3(v.pX, v.pY, v.pZ);
	r.normal = vec3(v.nX, v.nY, v.nZ);
	r.texcoord = vec2(v.tX, v.tY);

	return r;
}
