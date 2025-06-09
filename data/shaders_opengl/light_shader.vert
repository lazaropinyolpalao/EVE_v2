#version 330 core

#define MAX_SPOT_LIGHTS 4

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec3 aNorm;
layout (location = 3) in vec2 aUV;

out vec3 FragPos;
out vec3 Color;
out vec3 Normal;
out vec2 UV;
out vec4 FragPosDirectionalLightSpace;
out vec4 FragPosSpotlightLightSpace;
out vec3 CamView;

uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 directionalLightMatrix;
uniform mat4 spotlightLightMatrix;
uniform vec3 viewPos;

void main() {
  CamView = viewPos;
  FragPos = vec3(transform * vec4(aPos, 1.0));
  Color = aCol;
  Normal = transpose(inverse(mat3(transform))) * aNorm;
  UV = aUV;
  FragPosDirectionalLightSpace = directionalLightMatrix * vec4(FragPos, 1.0);
  FragPosSpotlightLightSpace = spotlightLightMatrix * vec4(FragPos, 1.0);

  gl_Position = projection * view * vec4(FragPos, 1.0);
}