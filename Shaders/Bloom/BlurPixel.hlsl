struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

Texture2D blurTexture: register(t0);

SamplerState textureSampler : register(s0);

cbuffer BloomPreFilter : register(b0)
{
    float threshold;
    float3 padding;
}
    
float4 PsMain(PSInput input) : SV_Target
{
    // Change this to modify the blur effect
    const float blurKernel[9] =
    {
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    };

    float3 color = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 9; i++)
    {
        color += blurTexture.Sample(textureSampler, input.texCoord) * blurKernel[i];
    }

    return float4(color.xyz, 1.0f);

}