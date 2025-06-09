#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;


uniform bool needs_light;

void main() {

	// Retrieve data from gbuffer
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
	float Specular = texture(gAlbedoSpec, TexCoords).a;

	if(Normal != vec3(0.0)) {
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		if(float(needs_light) == 0.0){
			FragColor = vec4(Albedo, 1.0);
		}
	}
	else {
		discard;
	}

}