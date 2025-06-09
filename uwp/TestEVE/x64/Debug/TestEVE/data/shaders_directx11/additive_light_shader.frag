#version 330 core

#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define MAX_TEXTURES 16

struct DirectionalLight {
	bool visible;
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	bool visible;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float range;
};

struct SpotLight {
	bool visible;
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float cutOff;
	float outerCutOff;
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
in vec4 FragPosDirectionalLightSpace;
in vec4 FragPosSpotlightLightSpace;
in vec3 CamView;

uniform int totalDirectionalLights;
uniform int totalPointLights;
uniform int totalSpotLights;
uniform int num_textures;
uniform bool receivesShadows;
uniform bool needsLight;
uniform sampler2D texture1;
uniform sampler2D directional_depthmap;
uniform sampler2D spot_depthmap[MAX_SPOT_LIGHTS];
uniform samplerCube point_depthmap[MAX_POINT_LIGHTS];
uniform float point_far_plane;

uniform DirectionalLight directionalLight[1];
uniform PointLight pointLight[MAX_POINT_LIGHTS];
uniform SpotLight spotLight[MAX_SPOT_LIGHTS];


float ShadowCalculation(sampler2D depthText, vec3 lightDir, vec4 FragPosLightSpace) {
	// perform perspective divide
	vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(depthText, projCoords.xy).r;
	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	// calculate bias (based on depth map resolution and slope)
	vec3 normal = normalize(Normal);
	//vec3 lightDir = normalize(lightPos - FragPos);
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	// check whether current frag pos is in shadow
	//float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	// PCF
	/**/
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthText, 0);
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
			float pcfDepth = texture(depthText, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;
	/**/
	// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
	if(projCoords.z < 0.0 || projCoords.z > 1.0) { shadow = 0.0; }

//  if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) 
//  {return -1.0;}
				
	return shadow;
}

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
	 vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
	 vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
	 vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
	 vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
	 vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowPointLightCalculation(samplerCube depth_cube, vec3 fragPos, vec3 lightPos) {
		vec3 fragToLight = fragPos - lightPos;
		
		float currentDepth = length(fragToLight);
		
		float shadow = 0.0;
		float bias = 0.15;
		int samples = 20;
		float viewDistance = length(CamView - fragPos);
		float diskRadius = (1.0 + (viewDistance / point_far_plane)) / 25.0;
		for(int i = 0; i < samples; ++i)
		{
				float closestDepth = texture(depth_cube, fragToLight + gridSamplingDisk[i] * diskRadius).r;
				closestDepth *= point_far_plane;   // undo mapping [0;1]
				if(currentDepth - bias > closestDepth)
						shadow += 1.0;
		}
		shadow /= float(samples);
				
		return shadow;
}  

void main() {
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(CamView - FragPos);
	vec3 tex = vec3(texture(texture1, UV));

	if(float(needsLight) == 1.0){
		vec3 ambient = vec3(0.0);
		vec3 diffuse = vec3(0.0);
		vec3 specular = vec3(0.0);

		for(int i = 0; i < totalDirectionalLights; i++) {
			float vis = float(directionalLight[i].visible);

			// -- DIFFUSE --
			vec3 lightDir = normalize(-directionalLight[i].direction);
			float diff = max(dot(norm, lightDir), 0.0);
			// -- SPECULAR --
			vec3 reflectDir = reflect(-lightDir, norm);
			float spec = max(dot(viewDir, reflectDir), 0.0);
			// -- RESULT --

			//vec3 ambient = directionalLight[i].ambient;
			//vec3 diffuse = directionalLight[i].diffuse * diff;
			//vec3 specular = directionalLight[i].specular * spec;

			ambient  += directionalLight[i].ambient           * vis;
			diffuse  += (directionalLight[i].diffuse  * diff) * vis;
			specular += (directionalLight[i].specular * spec) * vis;

			//total = vec3(ambient + diffuse + specular);
			//result += total;
		}

		for(int i = 0; i < totalPointLights; i++) {
			float vis = float(pointLight[i].visible);
			float distance = length(pointLight[i].position - FragPos);
			if(distance > pointLight[i].range) { vis = 0.0; }
			// -- DIFFUSE --
			vec3 lightDir = normalize(pointLight[i].position - FragPos);
			float diff = max(dot(norm, lightDir), 0.0);
			// -- SPECULAR --
			vec3 reflectDir = reflect(-lightDir, norm);
			float spec = max(dot(viewDir, reflectDir), 0.0);
			// -- ATTENUATION --
			float attenuation = 1.0 / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * (distance * distance));
			// -- RESULT --
			ambient  += (pointLight[i].ambient  *        attenuation) * vis;
			diffuse  += (pointLight[i].diffuse  * diff * attenuation) * vis;
			specular += (pointLight[i].specular * spec * attenuation) * vis;
		}

		for(int i = 0; i < totalSpotLights; i++) {

			float vis = float(spotLight[i].visible);
			float distance = length(spotLight[i].position - FragPos);
			if(distance > spotLight[i].range) {vis = 0.0;}
			// -- DIFFUSE --
			vec3 lightDir = normalize(spotLight[i].position - FragPos);
			float diff = max(dot(norm, lightDir), 0.0);
			// -- SPECULAR --
			vec3 reflectDir = reflect(-lightDir, norm);
			float spec = max(dot(viewDir, reflectDir), 0.0);
			// -- ATTENUATION --
		
			float attenuation = 1.0 / (spotLight[i].constant + spotLight[i].linear * distance + spotLight[i].quadratic * (distance * distance));
			// -- SOFT EDGES --
			float theta = dot(lightDir, normalize(-spotLight[i].direction));
			float epsilon = spotLight[i].cutOff - spotLight[i].outerCutOff;
			float intensity = clamp((theta - spotLight[i].outerCutOff) / epsilon, 0.0, 1.0);
			// -- RESULT --
		
			ambient  += (spotLight[i].ambient         * (attenuation * intensity)) * vis;
			diffuse  += (spotLight[i].diffuse  * diff * (attenuation * intensity)) * vis;
			specular += (spotLight[i].specular * spec * (attenuation * intensity)) * vis;

		}

		float shadow = -1.0;
		if(float(receivesShadows) == 1.0){
			shadow = ShadowCalculation(spot_depthmap[0], normalize(spotLight[0].position - FragPos), FragPosSpotlightLightSpace) * float(spotLight[0].visible);
			shadow += ShadowCalculation(directional_depthmap, directionalLight[0].direction, FragPosDirectionalLightSpace) * float(directionalLight[0].visible);
			shadow += ShadowPointLightCalculation(point_depthmap[0], FragPos, pointLight[0].position) * float(pointLight[0].visible);
		}
		else{
			shadow = 0.0;
		}

		if(shadow == -1.0) {
			FragColor = vec4(1.0, 0.0, 1.0, 1.0);
		} 
		else {
			vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * tex;
			FragColor = vec4(result, 1.0);
		}
	}else{FragColor = vec4(tex, 1.0);}
}