#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec3 aNorm;
layout (location = 3) in vec2 aUV;

out vec3 FragPos;
out vec3 Color;
out vec3 Normal;
out vec4 FragPosDirectionalLightSpace;
out vec2 UV;

uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 directionalLightMatrix;

void main() {
  FragPos = vec3(transform * vec4(aPos, 1.0));
  Color = aCol;
  UV = aUV;
  Normal = transpose(inverse(mat3(transform))) * aNorm;
  FragPosDirectionalLightSpace = directionalLightMatrix * vec4(FragPos, 1.0);

  gl_Position = projection * view * transform * vec4(aPos, 1.0);
}