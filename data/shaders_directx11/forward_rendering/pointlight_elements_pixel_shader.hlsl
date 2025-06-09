cbuffer ObjectLightInteraction : register(b0){
    float3 viewPos;
    bool receiveShadows;
    bool needs_light;
    float3 padding;
};

cbuffer PointLightUniforms : register(b1){
    float3 position;
    int pad1;
    float3 diffuse;
    int pad2;
    float3 specular;
    float constant;
	float linear_light;
	float quadratic;
	float range;
	int pad3;
};


Texture2D Text : register(t0);
SamplerState TextSampler : register(s0);

struct PSOutput{
    float4 color : SV_Target0;
};

struct PSInput{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : UV;
};

PSOutput Main(PSInput input){
	
    PSOutput output;
	output.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (needs_light){
		
		// -- DIFFUSE --
        float3 norm = normalize(input.normal);
        float3 lightDir = normalize(position - input.position.xyz);
        float diff = max(dot(norm, lightDir), 0.0);

		// -- SPECULAR --
        float3 viewDir = normalize(viewPos - (float3) (input.position));
        float3 reflectDir = reflect(-lightDir, norm);
		float spec = max(dot(viewDir, reflectDir), 0.0);

		// -- ATTENUATION --
        float dist = length(position - (float3) (input.position));
        float attenuation = 1.0 / (constant + linear_light * dist + quadratic * (dist * dist));

		// -- RESULT --
        float3 diffuse = (diffuse * diff * attenuation);
        float3 specular = (specular * spec * attenuation);

        float4 t = ((diffuse + specular).xyz, 0.0f);
        float3 result = t.xyz * Text.Sample(TextSampler, input.uv).rgb;
        output.color = float4(result, 1.0);
    }

    return output;
}


