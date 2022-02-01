cbuffer PerFrame : register(b0)
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
}

cbuffer PerObject : register(b1)
{
    matrix modelMatrix;
    matrix inverseTransposedModelMatrix;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_Position;
    float depth : DEPTH;
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(lightProjectionMatrix, mul(lightViewMatrix, modelMatrix));
	
    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    psInput.depth = psInput.position.z;
    return psInput;
}
