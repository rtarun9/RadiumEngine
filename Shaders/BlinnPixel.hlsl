cbuffer LightData : register(b0)
{
    // lightPosition is float4 due to padding reasons.
    float ambientStrength;
    float3 lightColor;

    float4 lightDirection;
};

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D heightTexture : register(t3);

SamplerState textureSampler : register(s0);

struct PSInput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

float4 CalculateAmbientLight(PSInput input)
{
    float3 ambient = ambientStrength * lightColor;
    return float4(ambient, 1.0f);
}

float4 CalculateDiffuseLight(float3 normal, PSInput input)
{
    float3 lightDir = normalize(-lightDirection).rgb;
    float diffuse = max(dot(lightDir, normal), 0.0f);
    
    float4 result = float4(diffuse, diffuse, diffuse, 1.0f);
    return result;
}


float4 PsMain(PSInput input) : SV_Target
{
    const float GAMMA = 2.2f;

    float4 lightCalculationResult = CalculateAmbientLight(input) + CalculateDiffuseLight(normalize(input.normal), input);

    float4 result = lightCalculationResult * normalTexture.Sample(textureSampler, input.texCoord);
    result = pow(result, 1 / GAMMA);

    return result;
}