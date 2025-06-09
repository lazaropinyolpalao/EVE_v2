#version 330 core

struct Pointlight {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float range;
};

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform samplerCube pointlight_depthmap;
uniform float point_far_plane;

uniform vec3 viewPos;
uniform Pointlight pointlight;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
	vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
	vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
	vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
	vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 FragPos, vec3 lightPos) {

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

	// Retrieve data from gbuffer
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
	float Specular = texture(gAlbedoSpec, TexCoords).a;

	if(Normal != vec3(0.0)) {

		// -- DIFFUSE
		vec3 lightDir = normalize(pointlight.position - FragPos);
		float diff = max(dot(Normal, lightDir), 0.0);

		// -- SPECULAR --
		vec3 viewDir = normalize(viewPos - FragPos);		
		vec3 reflectDir = reflect(-lightDir, Normal);
		float spec = max(dot(viewDir, reflectDir), 0.0);
		
		// -- ATENUATION --
		float dist = length(pointlight.position - FragPos);
		float attenuation = 1.0 / (pointlight.constant + pointlight.linear * dist + pointlight.quadratic * (dist * dist));

		// -- RESULT --
		vec3 diffuse  = pointlight.diffuse  * diff * attenuation * Albedo;
		vec3 specular = pointlight.specular * spec * attenuation * Specular;

		float shadow = ShadowCalculation(FragPos, pointlight.position);
		vec3 result = ((1.0 - shadow) * (diffuse + specular)) * Albedo;
	
		FragColor = vec4(result, 1.0);

	}
	else {
		discard;
	}

}