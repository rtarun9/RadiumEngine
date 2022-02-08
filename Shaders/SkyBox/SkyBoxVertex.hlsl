cbuffer PerFrame : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;

    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct PSInput
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION;
};

PSInput VsMain(VSInput input)
{
    PSInput psInput;


    matrix projectionView = mul(projectionMatrix, viewMatrix);
    psInput.position = mul(projectionView, float4(input.position, 0.0f)).xyww;
    psInput.worldPosition = input.position;

    return psInput;
}
