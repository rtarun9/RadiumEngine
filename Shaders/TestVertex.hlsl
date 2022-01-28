cbuffer PerFrame : register(b0)
{
    matrix viewProjectionMatrix;
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
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(viewProjectionMatrix, modelMatrix);
	
    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    psInput.texCoord = input.texCoord;
    psInput.normal = mul(inverseTransposedModelMatrix, float4(input.position, 1.0f));
    
    return psInput;
}
