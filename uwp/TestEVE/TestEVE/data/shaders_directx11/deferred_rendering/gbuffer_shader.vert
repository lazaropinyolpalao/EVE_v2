#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vec4 worldPos = transform * vec4(aPos, 1.0);
	FragPos = worldPos.xyz;
	TexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(transform)));
	Normal = normalMatrix * aNormal;

	gl_Position = projection * view * worldPos;
}