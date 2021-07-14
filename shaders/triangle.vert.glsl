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
	float x,y;
} pushConst;

void main()
{

	mat4 transform = mat4(
	vec4(1, 0, 0, 0),
	vec4(0, cos(pushConst.x), -sin(pushConst.x), 0),
	vec4(0, sin(pushConst.x), cos(pushConst.x) , 0),	
	vec4(0, 0, 0, 1)
	);


	mat4 transform1 = mat4(
	vec4(cos(pushConst.x), 0, sin(pushConst.x), 0),
	vec4(0, 1, 0, 0),
	vec4(-sin(pushConst.x), 0, cos(pushConst.x) , 0),	
	vec4(0, 0, 0, 1)
	);

	vec3 pos = vec3( v[gl_VertexIndex].vx,
	 			   -v[gl_VertexIndex].vy,
 				   -v[gl_VertexIndex].vz);

	vec2 tex = vec2(v[gl_VertexIndex].u,
					v[gl_VertexIndex].v);
	
	vec3 norm = vec3(v[gl_VertexIndex].nx, 
					 v[gl_VertexIndex].ny, 
					 v[gl_VertexIndex].nz);

	gl_Position = transform * transform1 * vec4(pos, 1.0) + vec4(cos(pushConst.x), sin(pushConst.y), 0.5,0.0);
	vColor = vec4(norm,1.0); 

}