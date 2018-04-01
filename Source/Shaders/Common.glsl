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

vec3 intersectRayTri(vec3 rayOrigin, vec3 rayDirection, vec3 v0, vec3 v1, vec3 v2)
{
	vec3 e0 = v1 - v0;
	vec3 e1 = v2 - v0;
	vec3 s1 = cross(rayDirection, e1);
	float  invd = 1.0 / (dot(s1, e0));
	vec3 d = rayOrigin - v0;
	float  b1 = dot(d, s1) * invd;
	vec3 s2 = cross(d, e0);
	float  b2 = dot(rayDirection, s2) * invd;
	float temp = dot(e1, s2) * invd;

	return vec3(1.0 - b1 - b2, b1, b2);
}
