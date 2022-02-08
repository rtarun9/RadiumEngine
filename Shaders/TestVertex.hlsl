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
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float4 lightTransformedPosition : LIGHT_SPACE_POSITION;
    float3x3 TBN : TBN_MATRIX;
    matrix modelMatrix : MODEL_MATRIX;
    matrix shadowProjectionViewMatrix : SHADOW_MATRIX;
};

PSInput VsMain(VSInput input)
{
    matrix mvpMatrix = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    matrix lightSpaceTransform = mul(lightProjectionMatrix, mul(lightViewMatrix, modelMatrix));

    float3 T = normalize(mul(modelMatrix, float4(input.tangent, 0.0f)).xyz);
    float3 B = normalize(mul(modelMatrix, float4(input.bitangent, 0.0f)).xyz);
    float3 N = normalize(mul(modelMatrix, float4(input.normal, 0.0f)).xyz);

    float3x3 TBN = float3x3(T, B, N);

    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    psInput.texCoord = input.texCoord;
    psInput.normal = mul((float3x3) (inverseTransposedModelMatrix), input.normal);
    psInput.lightTransformedPosition = mul(lightSpaceTransform, float4(input.position, 1.0f));
    psInput.color = color;
    // This is happens as a directional (sun) light is used, so the method of converting all vectors involved in lighting from world -> normal coordinates (by multiplying
    // with TBN inverse) cannot happen.
    psInput.TBN = TBN;
    psInput.modelMatrix = modelMatrix;
    psInput.shadowProjectionViewMatrix = lightSpaceTransform;
    //psInput.position = psInput.lightTransformedPosition;

    return psInput;
}
