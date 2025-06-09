#version 330 core

struct PointLight {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float range;
};

out vec4 FragColor;

in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 UV;

uniform vec3 viewPos;
uniform bool receivesShadows;
uniform bool needs_light;
uniform PointLight pointlight;
uniform samplerCube pointlight_depthmap;
uniform sampler2D texture1;
uniform float point_far_plane;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
	vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
	vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
	vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
	vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 lightPos) {

	vec3 fragToLight = FragPos - lightPos;
		
	float currentDepth = length(fragToLight);
		
	float shadow = 0.0;
	float bias = 0.15;
	int samples = 20;
	float viewDistance = length(viewPos - FragPos);
	float diskRadius = (1.0 + (viewDistance / point_far_plane)) / 25.0;

	for(int i = 0; i < samples; ++i) {
		float closestDepth = texture(pointlight_depthmap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
		closestDepth *= point_far_plane;   // undo mapping [0;1]
		if(currentDepth - bias > closestDepth) {
			shadow += 1.0;
		}
	}

	shadow /= float(samples);
				
	return shadow;
}

void main() {
	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	if(float(needs_light) == 1.0) {

		// -- DIFFUSE --
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(pointlight.position - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);

		// -- SPECULAR --
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = max(dot(viewDir, reflectDir), 0.0);

		// -- ATTENUATION --
		float dist = length(pointlight.position - FragPos);
		float attenuation = 1.0 / (pointlight.constant + pointlight.linear * dist + pointlight.quadratic * (dist * dist));

		// -- RESULT --
		vec3 diffuse  = (pointlight.diffuse  * diff * attenuation);
		vec3 specular = (pointlight.specular * spec * attenuation);

		// -- SHADOW --
		float shadow = -1.0;
		if(float(receivesShadows) == 1.0){
			shadow += ShadowCalculation(pointlight.position);
		}
		else { shadow = 0.0; }

		vec3 result = ((1.0 - shadow) * (diffuse + specular)) * vec3(texture(texture1, UV));
		FragColor = vec4(result, 1.0);
	}

}


