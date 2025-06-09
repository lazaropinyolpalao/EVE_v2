#version 330 core

struct Light {
  bool active;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

in vec3 FragPos;
in vec3 Color;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 viewPos;
uniform Light light;

void main() {
  vec3 tex = texture(texture1, UV).rgb;
  vec3 result = vec3(0.0) + tex;
  if(light.active) {
    // -- AMBIENT --
    vec3 ambient = light.ambient;
    // -- DIFFUSE --
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;
    // -- SPECULAR --
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = max(dot(viewDir, reflectDir), 0.0);
    vec3 specular = light.specular * spec;
    // -- RESULT --
    result *= (ambient + diffuse + specular);
  }
  FragColor = vec4(result, 1.0);
}