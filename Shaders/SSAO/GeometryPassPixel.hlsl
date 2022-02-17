cbuffer LightData : register(b0)
{
    // lightPosition is float4 due to padding reasons.
    float ambientStrength;
    float3 lightColor;

    float lightStrength;
    float3 lightDirection;
};

cbuffer CameraData : register(b1)
{
    float3 cameraPosition;
    float padding;
};

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 position : SV_Target2;
};

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D heightTexture : register(t3);

SamplerState textureSampler : register(s0);
SamplerState clampTextureSampler : register(s1);

struct PSInput
{
    float4 position : SV_Position;
    float4 viewSpacePosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float3x3 TBN : TBN_MATRIX;
};

PSOutput PsMain(PSInput input) : SV_Target
{
    float4 diffTexture = diffuseTexture.Sample(textureSampler, input.texCoord);
    float alpha = diffTexture.a;

    
    float3 normal = normalTexture.Sample(textureSampler, input.texCoord).xyz;
    // Convert to -1, 1 range
    normal = normal * 2.0f - float3(1.0f, 1.0f, 1.0f);
    normal = normalize(mul(normal, input.TBN));

    PSOutput output;
    output.albedo = float4(diffTexture.xyz, alpha);
    output.position = input.viewSpacePosition;
    output.normal = float4(normal, 1.0f);

    return output;
}