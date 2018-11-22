#version 130

uniform float LOD;

smooth in vec3 N;
smooth in vec3 V;

flat in vec3 light_pos;

out vec4 frag_color;

void main (void) {
   vec3 L = normalize(light_pos - V);
   vec3 E = normalize(-V); 
   vec3 R = normalize(-reflect(L, N));
 
   //calculate Ambient Term:
   vec3 Iamb = vec3(1.0, 1.0, 1.0);

   //calculate Diffuse Term:
   vec3 Idiff = vec3(0.8, 0.8, 1.0) * vec3(max(dot(N, L), 0.0));
   
   // calculate Specular Term:
   vec3 Ispec = vec3(0.1, 0.1, 1.0) * pow(max(dot(R, E), 0.0), 2.2);
   Ispec = clamp(Ispec, 0.0, 1.0);

   // write Total Color:
   vec4 col;
   if (LOD == 0) {col = vec4(0.9, 0.9, 0.2, 1);}
   else if (LOD == 1) {col = vec4(0.9, 0.2, 0.2, 1);}
   else if (LOD == 2) {col = vec4(0.2, 0.9, 0.9, 1);}
   else {col = vec4(0.2, 0.9, 0.2, 1);}

   frag_color = col * vec4(Iamb * 0.4 + Idiff * 0.6 + Ispec * 0.2, 1.0);
}
