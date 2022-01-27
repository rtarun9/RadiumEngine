cbuffer PerApplication : register(b0)
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register(b1)
{
    matrix viewMatrix;
}

cbuffer PerObject : register(b2)
{
    matrix modelMatrix;
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
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
	
    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    psInput.texCoord = input.texCoord;
    
    return psInput;
}
