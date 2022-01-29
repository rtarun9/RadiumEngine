cbuffer LightData : register(b0)
{
    // lightPosition is float4 due to padding reasons.
    float ambientStrength;
    float3 lightColor;

    float4 lightDirection;
};

struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
};

float4 CalculateAmbientLight(PSInput input)
{
    float3 ambient = ambientStrength * lightColor * input.color;
    return float4(ambient, 1.0f);
}

float4 CalculateDiffuseLight(float3 normal, PSInput input)
{
    float3 lightDir = normalize(-lightDirection).rgb;
    float diffuse = max(dot(lightDir, normal), 0.0f);
    
    float4 result = float4(diffuse * lightColor * input.color, 1.0f);
    return result;
}


float4 PsMain(PSInput input) : SV_Target
{
    float4 lightCalculationResult = CalculateAmbientLight(input) + CalculateDiffuseLight(normalize(input.normal), input);

    return lightCalculationResult;
}