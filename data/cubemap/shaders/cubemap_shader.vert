#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 FragPos;

uniform mat4 view;
uniform mat4 projection;

void main(){
  vec3 tmp = mat3(view) * aPos;
  vec4 pos = projection * vec4(tmp, 1.0f);
  gl_Position = pos.xyww;	// Forzar maxima profundidad

  FragPos = aPos;
}