#version 130

uniform float color;

smooth in vec3 N;
smooth in vec3 V;

flat in vec3 light_pos;

out vec4 frag_color;

void main (void) {

   // write Total Color:  
   frag_color = vec4(0.9, 0.9, 0.9, 1.0)*color;
}
