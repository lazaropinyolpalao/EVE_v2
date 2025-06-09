cbuffer ObjectLightInteraction : register(b0){
    float3 viewPos;
    bool receiveShadows;
    bool needs_light;
    float3 padding;
};

cbuffer SpotLight : register(b1){
    float3 position;
    float pad1;
    float3 direction;
    float pad2;
    float3 diffuse;
    float pad3;
    float3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear_light;
    float quadratic;
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
        float3 norm = normalize(input.normal);
        float3 viewDir = normalize(viewPos - input.position.xyz);

        float dist = length(position - input.position.xyz);

        // -- DIFFUSE --
        float3 lightDir = normalize(position - input.position.xyz);
        float diff = max(dot(norm, lightDir), 0.0f);
        // -- SPECULAR --
        float3 reflectDir = reflect(-lightDir, norm);
        float spec = max(dot(viewDir, reflectDir), 0.0f);
        // -- ATTENUATION --

        float attenuation = 1.0 / (constant + linear_light * dist + quadratic * (dist * dist));
        // -- SOFT EDGES --
        float theta = dot(lightDir, normalize(-direction));
        float epsilon = cutOff - outerCutOff;
        float intensity = clamp((theta - outerCutOff) / epsilon, 0.0f, 1.0f);

        // -- RESULT --
        float3 diffuse = (diffuse * diff * (attenuation * intensity));
        float3 specular = (specular * spec * (attenuation * intensity));
        
        float4 t = ((diffuse + specular).xyz, 0.0f);
        float3 result = t.xyz * Text.Sample(TextSampler, input.uv).rgb;
        //result = Text.Sample(TextSampler, input.uv).rgb;
        output.color = float4(result.xyz, 1.0f);

    }
    else
    {
        output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);

    }
    
    return output;
}