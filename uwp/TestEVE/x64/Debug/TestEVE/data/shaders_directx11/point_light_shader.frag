#version 330 core

struct Light {
  bool active;
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
};

in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Light light;
uniform sampler2D texture1;

void main() {
  vec3 tex = texture(texture1, UV).rgb;
  vec3 result = vec3(0.0) + tex;
  if(light.active) {
    // -- AMBIENT --
    vec3 ambient = light.ambient;
    // -- DIFFUSE --
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;
    // -- SPECULAR --
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = max(dot(viewDir, reflectDir), 0.0);
    vec3 specular = light.specular * spec;
    // -- ATTENUATION --
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // -- RESULT --
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    result *= (ambient + diffuse + specular);
  }
  FragColor = vec4(result, 1.0);
}