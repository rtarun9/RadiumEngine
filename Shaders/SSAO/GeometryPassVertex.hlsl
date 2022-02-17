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

cbuffer VPData : register(b2)
{
    matrix inverseTransposeVPMatrix;
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
    float4 viewSpacePosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float3x3 TBN : TBN_MATRIX;
};

PSInput VsMain(VSInput input)
{
    // Since SSAO is done is screen space and occlusion is calculated in view space, the algorithm is implemented
    // in view space as well.
    matrix vpMatrix = mul(viewMatrix, modelMatrix);
    matrix mvpMatrix = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
    
    float3 T = normalize(mul(modelMatrix, float4(input.tangent, 0.0f)).xyz);
    float3 B = normalize(mul(modelMatrix, float4(input.bitangent, 0.0f)).xyz);
    float3 N = normalize(mul(modelMatrix, float4(input.normal, 0.0f)).xyz);

    float3x3 TBN = float3x3(T, B, N);


    PSInput psInput;
    psInput.position = mul(mvpMatrix, float4(input.position, 1.0f));
    psInput.viewSpacePosition = mul(vpMatrix, float4(input.position, 1.0f));
    psInput.normal = mul((float3x3) (inverseTransposeVPMatrix), input.normal);
    psInput.texCoord = input.texCoord;
    psInput.color = color;
    psInput.TBN = TBN;

    return psInput;
}
