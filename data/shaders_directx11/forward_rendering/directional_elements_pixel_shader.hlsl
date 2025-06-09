cbuffer ObjectLightInteraction : register(b0){
    float3 viewPos;
    bool receiveShadows;
    bool needs_light;
    float3 padding;
};

cbuffer DirectionalLightUniforms : register(b1){
    matrix directionalLightMatrix;
    float3 directionalPosition;
    float p1;
    float3 directionalDirection;
    float p2;
    float3 directionalAmbient;
    float p3;
    float3 directionalDiffuse;
    float p4;
    float3 directionalSpecular;
    float p5;
};

Texture2D Text : register(t0);
SamplerState TextSampler : register(s0);

struct PSOutput {
  float4 color : SV_Target0;
};

struct PSInput {
  float4 position : SV_POSITION;
  float3 color : COLOR;
  float3 normal : NORMAL;
  float2 uv : UV;
};

PSOutput Main(PSInput input) {

  PSOutput output;
  output.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  if(needs_light) {
    float3 norm = normalize(input.normal);
    float3 viewDir = normalize(viewPos - input.position.xyz);
    
    // DIFFUSE
    float3 lightDir = normalize(-directionalDirection);
    float diff = max(dot(lightDir, norm), 0.0f);
    float3 diffuse = directionalDiffuse * diff;

    // SECULAR
    float3 reflectDir = reflect(-lightDir, norm);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0f), 64.0f);
    float3 specular = directionalSpecular * spec;

    // RESULT
    float3 result = directionalAmbient * (diffuse + specular) * Text.Sample(TextSampler, input.uv).rgb;
    output.color = float4(result, 1.0f);
  }
  
  return output;

}