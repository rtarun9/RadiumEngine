struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

cbuffer BloomPreFilter : register(b0)
{
    float threshold;
    float3 padding;
}

Texture2D renderTexture : register(t0);
SamplerState textureSampler : register(s0);

float4 PsMain(PSInput input) : SV_Target
{
    float3 color = renderTexture.Sample(textureSampler, input.texCoord).xyz;
    //color = pow(color, GAMMA_CORRECTION);

    float brightness = dot(float3(0.2126, 0.7152, 0.0722), color);
    if (brightness > threshold)
    {
        return float4(color.xyz, 1.0f);
    }

    return float4(0.0f, 0.0f, 0.0f, 1.0f);

}