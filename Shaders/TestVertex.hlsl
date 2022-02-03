cbuffer PerFrame : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;

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
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float4 lightTransformedPosition : LIGHT_SPACE_POSITION;
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    matrix lightSpaceTransform = mul(lightProjectionMatrix, mul(lightViewMatrix, modelMatrix));

    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    psInput.texCoord = input.texCoord;
    psInput.normal = mul((float3x3) (inverseTransposedModelMatrix), input.normal);
    psInput.lightTransformedPosition = mul(lightSpaceTransform, float4(input.position, 1.0f));

    //psInput.position = psInput.lightTransformedPosition;

    return psInput;
}
