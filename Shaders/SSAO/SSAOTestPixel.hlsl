Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D position : register(t2);
Texture2D ssaoTexture : register(t3);

cbuffer CameraData : register(b0)
{
    float3 cameraPosition;
    float padding;
};

cbuffer LightData : register(b1)
{
    float ambientStrength;
    float3 lightColor;

    float lightStrength;
    float3 lightDirection;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

SamplerState textureSampler : register(s0);
SamplerState clampTextureSampler : register(s1);

float4 PsMain(PSInput input) : SV_Target
{
    float3 pixelPositionValue = position.Sample(textureSampler, input.texCoord).xyz;
    float3 albedoValue = albedo.Sample(textureSampler, input.texCoord).xyz;
    float3 normalValue = normal.Sample(textureSampler, input.texCoord).xyz;

    float3 viewDir = normalize(-pixelPositionValue);

    float3 lightDir = normalize(-lightDirection);
    
    float3 diffuse = (max(dot(normalValue, lightDir), 0.0f) * albedoValue * lightColor * lightStrength);
    float3 ambient = ssaoTexture.Sample(textureSampler, input.texCoord).x * albedoValue * 0.2f;
   // only for comparing SSAO on vs Off.
    if (input.texCoord.x > 0.5f)
    {
        ambient = albedoValue * 0.2f;
    }

    return float4(ambient + diffuse, 1.0f);
}