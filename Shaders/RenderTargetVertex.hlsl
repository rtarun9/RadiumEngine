struct VSInput
{
    float2 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

PSInput VsMain(VSInput input)
{
    PSInput psInput;
    psInput.position = float4(input.position.x, input.position.y, 0.0f, 1.0f);
    psInput.texCoord = input.texCoord;

    return psInput;
}