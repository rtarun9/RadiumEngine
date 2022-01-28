cbuffer PerFrame : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
}

cbuffer PerObject : register(b1)
{
    matrix modelMatrix;
    matrix inverseTransposedModelMatrix;
    float3 color;
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
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
	
    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    psInput.texCoord = input.texCoord;
    psInput.normal = mul((float3x3) (inverseTransposedModelMatrix), input.normal);
    
    // Only for testing, needs to be removed soon.
    psInput.color = color;
    
    return psInput;
}
