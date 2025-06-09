#version 330 core

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 diffuse;
	vec3 specular;
	float cutOff;
	float outerCutOff;
	float constant;
	float linear;
	float quadratic;
};

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D spotlight_depthmap;
uniform mat4 spotLightMatrix;

uniform SpotLight spotlight;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 Normal){
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(spotlight_depthmap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-spotlight.direction);
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float bias = 0.005;
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(spotlight_depthmap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(spotlight_depthmap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)shadow = 0.0;

    //if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0){return -1.0;}
        
    return shadow;
}

void main() {

	// Retrieve data from gbuffer
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
	float Specular = texture(gAlbedoSpec, TexCoords).a;

  if(Normal != vec3(0.0)) {
  
	  vec3 viewDir = normalize(viewPos - FragPos);

    // -- DIFFUSE --
	  vec3 lightDir = normalize(spotlight.position - FragPos);
	  float diff = max(dot(Normal, lightDir), 0.0);

	  // -- SPECULAR --
	  vec3 reflectDir = reflect(-lightDir, Normal);
	  float spec = max(dot(viewDir, reflectDir), 0.0);

	  // -- ATTENUATION --		
	  float dist = length(spotlight.position - FragPos);
	  float attenuation = 1.0 / (spotlight.constant + spotlight.linear * dist + spotlight.quadratic * (dist * dist));

	  // -- SOFT EDGES --
	  float theta = dot(lightDir, normalize(-spotlight.direction));
	  float epsilon = spotlight.cutOff - spotlight.outerCutOff;
	  float intensity = clamp((theta - spotlight.outerCutOff) / epsilon, 0.0, 1.0);

	  // -- RESULT --		
	  vec3 diffuse  = (spotlight.diffuse  * diff * (attenuation * intensity) * Albedo);
	  vec3 specular = (spotlight.specular * spec * (attenuation * intensity) * Specular);

    vec4 FragPosSpotLightSpace = spotLightMatrix * vec4(FragPos, 1.0);

    float shadow = ShadowCalculation(FragPosSpotLightSpace, Normal);
 
    vec3 result = ((1.0 - shadow) * (diffuse + specular)) * Albedo;
	  FragColor = vec4(result, 1.0);

  }
  else {
    discard;
  }

}