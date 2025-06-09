cbuffer PerFrame : register(b0) {
  matrix transform;
  matrix projection;
  matrix view;  
};

struct VSInput {
  float3 position : POSITION;
  float3 color : COLOR;
  float3 normal : NORMAL;
  float2 uv : UV;
};

struct VSOutput {
  float4 position : SV_POSITION;
  float3 color : COLOR;
  float3 normal : NORMAL;
  float2 uv : UV;
};

VSOutput Main(VSInput input) {
	
  VSOutput output;
	
  float4 finalPos = float4(input.position, 1.0);
  output.position = mul(transform, finalPos);
  output.position = mul(view, output.position);
  output.position = mul(projection, output.position);

  output.color = input.color;
  output.normal = input.normal;
  output.uv = input.uv;

  return output;
}
