#version 330 core

out vec4 FragColor;

in vec2 UV;
in vec3 FragPos;

uniform bool needs_light;
uniform sampler2D texture1;

void main() {
	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	if(float(needs_light) == 0.0){
		FragColor = vec4(vec3(texture(texture1, UV)), 1.0);
	}
}