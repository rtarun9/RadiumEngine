struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

Texture2D currentTexture : register(t0);

// Only needed while upsampling. Will be set to null during the downsampling pass as the same shader is used for both passes.
Texture2D previousTexture : register(t1);

SamplerState textureSampler : register(s0);

float4 PsMain(PSInput input) : SV_Target
{
    return currentTexture.Sample(textureSampler, input.texCoord) + previousTexture.Sample(textureSampler, input.texCoord);
}