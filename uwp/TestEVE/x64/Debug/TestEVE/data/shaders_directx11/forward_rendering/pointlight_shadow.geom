#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 lightSpaceMatrix[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main(){
    for(int face = 0; face < 6; ++face){
        gl_Layer = face; // Tell OpenGL in which face are we rendering
        for(int i = 0; i < 3; ++i){
            FragPos = gl_in[i].gl_Position;
            gl_Position = lightSpaceMatrix[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 