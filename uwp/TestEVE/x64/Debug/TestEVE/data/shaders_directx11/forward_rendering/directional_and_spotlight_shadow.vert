#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec3 aNorm;
layout (location = 3) in vec2 aUV;

uniform mat4 lightSpaceMatrix;
uniform mat4 transform;

void main(){
    gl_Position = lightSpaceMatrix * transform * vec4(aPos, 1.0);
}