struct PSInput
{
    float4 position : SV_Position;
    float depth : DEPTH;
};

struct PSOutput
{
    float4 pixelColor : SV_Target;
    float depth : SV_Depth;
};

PSOutput PsMain(PSInput input)
{
    PSOutput output;
    float linearDepthValue = 0.01f / (1.01f - input.depth);

    // Doesnt matter, no render target view definied anyways.
    output.pixelColor = (linearDepthValue, linearDepthValue, linearDepthValue, 1.0f);

    output.depth = 0.1f;

    return output;
}