struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

Texture2D testTexture : register(t0);
SamplerState textureSampler : register(s0);

float4 PsMain(PSInput input) : SV_Target
{
    // Not using texture as of now
    return float4(0.8f, 0.8f, 0.8f, 1.0f);
}