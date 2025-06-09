#version 330 core

struct DirectionalLight {
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D directional_depthmap;
uniform mat4 directionalLightMatrix;

uniform DirectionalLight directional;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 Norm) {
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(directional_depthmap, projCoords.xy).r; 
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // calculate bias (based on depth map resolution and slope)
  vec3 normal = normalize(Norm);
  vec3 lightDir = normalize(-directional.direction);
  //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
  float bias = 0.005;
  // check whether current frag pos is in shadow
  // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(directional_depthmap, 0);
  for(int x = -1; x <= 1; ++x)
  {
      for(int y = -1; y <= 1; ++y)
      {
          float pcfDepth = texture(directional_depthmap, projCoords.xy + vec2(x, y) * texelSize).r; 
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

	  // -- AMBIENT --
	  vec3 ambient = directional.ambient;

	  // -- DIFFUSE --
	  vec3 lightDir = normalize(-directional.direction);
	  vec3 diffuse = max(dot(Normal, lightDir), 0.0) * directional.diffuse;

	  // -- SPECULAR --
	  vec3 viewDir = normalize(viewPos - FragPos);
	  vec3 reflectDir = reflect(-lightDir, Normal);
	  vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 64.0);
	  vec3 specular = directional.specular * spec * Specular;

	  // -- RESULT --
    vec4 FragPosDirectionalLightSpace = directionalLightMatrix * vec4(FragPos, 1.0);

    float shadow = ShadowCalculation(FragPosDirectionalLightSpace, Normal);

	  vec3 result = (directional.ambient + (1.0 - shadow) * (diffuse + specular)) * Albedo;
	  FragColor = vec4(result, 1.0);

  }
  else {
    discard;
  }

}


