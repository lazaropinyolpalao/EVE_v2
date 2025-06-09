#version 330 core

out vec4 FragColor;

in vec3 Pos;
in vec2 UV;

uniform sampler2D screen_texture;

void main(){
  FragColor = vec4(texture2D(screen_texture, UV).rgb, 1.0);
}