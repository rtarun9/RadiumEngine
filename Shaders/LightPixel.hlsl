cbuffer LightData : register(b0)
{
    // lightPosition is float4 due to padding reasons.
    float ambientStrength;
    float3 lightColor;

    float lightStrength;
    float3 lightDirection;
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
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float4 lightTransformedPosition : LIGHT_SPACE_POSITION;
    float3x3 TBN : TBN_MATRIX;
    matrix modelMatrix : MODEL_MATRIX;
    matrix shadowProjectionViewMatrix : SHADOW_MATRIX;
};



float4 PsMain(PSInput input) : SV_Target
{
    return float4(lightColor.xyz, 1.0f);
}