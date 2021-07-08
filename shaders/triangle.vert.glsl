#version 450

layout(location = 0) out vec4 vColor;

struct Vertex
{
	float vx,vy,vz;
	float u,v;
	float nx,ny,nz;
};

layout(binding = 0) readonly buffer Vertices
{
	Vertex vertices[];
};


void main()
{
	Vertex v = vertices[gl_VertexIndex];
	
	vec3 pos = vec3(v.vx, -v.vy,v.vz);
	vec2 tex = vec2(v.u,v.v);
	vec3 norm = vec3(v.nx, v.ny, v.nz);

	gl_Position = vec4(pos + vec3(0, 0, 0.5), 1.0);
	vColor = vec4(norm * 0.5 + vec3(0.5), 1.0); 
}