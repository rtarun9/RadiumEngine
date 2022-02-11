struct PSInput
{
    float4 position : SV_Position;
    float3 texCoord : POSITION;
};

Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

const float2 invAtan = float2(0.1591, 0.3183);

float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan(v.z / v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;

    return uv;
}

float4 PsMain(PSInput input) : SV_Target
{
    return tex.Sample(samplerState, SampleSphericalMap(normalize(input.texCoord))) * 0.8f;
}