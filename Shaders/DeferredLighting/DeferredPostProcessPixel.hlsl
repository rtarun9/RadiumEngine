struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

cbuffer RTVBuffer : register(b0)
{
    float exposure;
    float3 padding;
}

Texture2D renderTexture : register(t0);
Texture2D bloomTexture : register(t1);
Texture2D deferred : register(t2);

SamplerState textureSampler : register(s0);

float4 PsMain(PSInput input) : SV_Target
{

    // Change this to modify the blur effect
    const float blurKernel[9] =
    {
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    };

    float3 bloomPixelColor = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 9; i++)
    {
        bloomPixelColor += bloomTexture.Sample(textureSampler, input.texCoord) * blurKernel[i];

    }
    const float GAMMA = 2.2f;
    const float3 GAMMA_CORRECTION = float3(1.0f / GAMMA, 1.0f / GAMMA, 1.0f / GAMMA);
    
    float3 hdrColor = renderTexture.Sample(textureSampler, input.texCoord).xyz + bloomPixelColor + deferred.Sample(textureSampler, input.texCoord).xyz;

    float3 mappedColor = float3(1.0f, 1.0f, 1.0f) - exp(-hdrColor * exposure);
    mappedColor = pow(mappedColor, GAMMA_CORRECTION);

    return float4(mappedColor, 1.0f);

}