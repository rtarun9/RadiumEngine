struct PSInput
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION;
};

TextureCube tex : register(t0);
SamplerState samplerState : register(s0);

float4 PsMain(PSInput input) : SV_Target
{
    return tex.Sample(samplerState, input.worldPosition.xyz);
}