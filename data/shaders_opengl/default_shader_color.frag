#version 330 core

in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform sampler2D texture1;

void main() {
  FragColor = vec4(Color.x, Color.y, Color.z, 1.0);
}