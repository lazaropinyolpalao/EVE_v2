Texture2D Text : register(t0);
SamplerState TextSam : register(s0);

struct PSInput {
  float4 position: SV_POSITION;
  float3 color: COLOR;
  float3 normal: NORMAL;
  float2 uv: UV;
};

struct PSOutput {
  float4 color: SV_Target0;
};

PSOutput Main(PSInput input) {
  PSOutput output;

  output.color = Text.Sample(TextSam, input.uv);

  return output;
}