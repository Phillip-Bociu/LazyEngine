#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTex;
layout(location = 2) in vec3 vNorm;

layout(location = 0) out vec4 vColor;

void main()
{
	gl_Position = vec4(vPos + vec3(0, 0, 0.5), 1.0);
	gl_Position.y = -gl_Position.y;
	vColor = vec4(vNorm * 0.5 + vec3(0.5), 1.0); 
}