#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

out vec3 Pos;
out vec2 UV;

void main() {
  Pos = aPos;
  UV = aUV;
  gl_Position = vec4(Pos, 1.0);
}