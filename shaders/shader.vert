#version 450


vec2 pos[3] = vec2[]
(   
    vec2(0.0,-0.5),
    vec2(0.0,0.5),
    vec2(-1.0, 0.34)
);

void main()
{
    gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);

}