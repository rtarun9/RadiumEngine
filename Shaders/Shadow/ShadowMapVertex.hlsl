cbuffer PerFrame : register(b0)
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
}

cbuffer PerObject : register(b1)
{
    matrix modelMatrix;
    matrix inverseTransposedModelMatrix;
    float4 color;
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
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(lightProjectionMatrix, mul(lightViewMatrix, modelMatrix));
	
    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    return psInput;
}
