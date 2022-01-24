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
    float3 color : COLOR;
};

struct PSInput
{
    float4 color : COLOR;
    float4 position : SV_Position;
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
	
    PSInput psInput;
    psInput.color = float4(input.color, 1.0f);
    psInput.position = mul(mvpMatrix, mul(input.position, 1.0f));
	
    return psInput;
}

float4 PsMain(PSInput input) : SV_Target
{
    return input.color;
}