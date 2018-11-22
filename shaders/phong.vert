#version 130

attribute vec3 vertex;
attribute vec3 normal;

//layout (location = 0) in vec3 vertex;
//layout (location = 1) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normal_matrix;

smooth out vec3 N;
smooth out vec3 V;

flat out vec3 light_pos;


void main(void)  {     
    light_pos = (view * vec4(0, 0, 100, 1)).xyz;

    vec4 view_vertex = view * model * vec4(vertex, 1);
    V = view_vertex.xyz;
    N = normalize(normal_matrix * normal);

    gl_Position = projection * view_vertex;
}
