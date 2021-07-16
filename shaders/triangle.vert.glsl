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
	Vertex v[];
};

layout( push_constant ) uniform constants
{ 
	mat4 viewMatrix;
	float x,y;
} pushConst;

void main()
{

	vec3 pos = vec3( v[gl_VertexIndex].vx + pushConst.x,
	 			   -v[gl_VertexIndex].vy,
 				   -v[gl_VertexIndex].vz - pushConst.y);

	vec2 tex = vec2(v[gl_VertexIndex].u,
					v[gl_VertexIndex].v);
	
	vec3 norm = vec3(v[gl_VertexIndex].nx, 
					 v[gl_VertexIndex].ny, 
					 v[gl_VertexIndex].nz);

	//gl_Position = pushConst.viewMatrix * transform * transform1 * vec4(pos, 1.0) + vec4(cos(pushConst.x), sin(pushConst.y), 0.5,0.0);
	gl_Position = pushConst.viewMatrix * vec4(pos, 1.0);
	vColor = vec4(norm,1.0); 

}