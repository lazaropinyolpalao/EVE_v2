#version 330 core

out vec4 FragColor;
 
in vec3 FragPos;

// Texture sampler
uniform sampler2D background_texture;
 
// For each component of v, returns -1 if the component is < 0, else 1
vec2 sign_not_zero(vec2 v) {
  return vec2(v.x >= 0 ? 1.0 : -1.0,
              v.y >= 0 ? 1.0 : -1.0);
}

// Packs a 3-component normal to 2 channels using octahedron normals
vec2 pack_normal_octahedron(vec3 v) {
  float rcp = 1.0 / dot(abs(v), vec3(1));
  v.xy *= rcp;
  if (v.z <= 0) 
      v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
  return v.xy;
}

// Unpacking from octahedron normals, input is the output from pack_normal_octahedron
vec3 unpack_normal_octahedron(vec2 packed_nrm) {  
  vec3 v = vec3(packed_nrm.xy, 1.0 - abs(packed_nrm.x) - abs(packed_nrm.y));
  if (v.z < 0) 
      v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
  return normalize(v);
}


void main() {
  vec2 uv = pack_normal_octahedron(normalize(FragPos).xzy);
  vec2 uvc = (uv * 0.5f) + 0.5f;
  FragColor = texture(background_texture, uvc);
}