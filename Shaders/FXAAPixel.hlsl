struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

struct LumanositySamples
{
    float topLeft;
    float topRight;
    float middle;
    float bottomLeft;
    float bottomRight;
};

Texture2D renderTexture : register(t0);

SamplerState textureSampler : register(s0);

// This is dependent on dimentions of image : Currently being hardcoded.
static const float2 textureDimentions = float2(1.0f / 1920.0f, 1.0f / 1080.0f);

static const float MIN_VALUE = 1.0f / 128.0f;

static const float FXAA_SPAN_MAX = 8.0f;

static const float FXAA_BIAS_VALUE = 1.0f / 8.0f;

float4 PsMain(PSInput input) : SV_Target
{
    // Multiply color with this to get lumanosity of a particular sample.
    float3 lumanosity = float3(0.299f, 0.587f, 0.114f);

    LumanositySamples lumanositySample;
    lumanositySample.topLeft = dot(lumanosity, renderTexture.Sample(textureSampler, input.texCoord + (float2(-1.0f, -1.0f) * textureDimentions)).xyz);
    lumanositySample.topRight = dot(lumanosity, renderTexture.Sample(textureSampler, input.texCoord + (float2(1.0f, -1.0f) * textureDimentions)).xyz);
    lumanositySample.middle = dot(lumanosity, renderTexture.Sample(textureSampler, input.texCoord).xyz);
    lumanositySample.bottomLeft = dot(lumanosity, renderTexture.Sample(textureSampler, input.texCoord + (float2(-1.0f, 1.0f) * textureDimentions)).xyz);
    lumanositySample.bottomRight = dot(lumanosity, renderTexture.Sample(textureSampler, input.texCoord + (float2(1.0f, 1.0f) * textureDimentions)).xyz);

    float2 blurDirection = float2(0.0f, 0.0f);

    blurDirection.x = -((lumanositySample.topLeft + lumanositySample.topRight) - (lumanositySample.bottomLeft + lumanositySample.bottomRight));
    blurDirection.y = ((lumanositySample.topLeft + lumanositySample.bottomLeft) - (lumanositySample.topRight + lumanositySample.bottomRight));

    // reduce blur direction biased on the average lumanosity.
    float reduceAmount = max((lumanositySample.topLeft + lumanositySample.topRight + lumanositySample.bottomRight + lumanositySample.bottomLeft) * (0.25f * FXAA_BIAS_VALUE), MIN_VALUE);

    // Scale blurdirection so that smallest component will becomes scaled to size of one texel.
    float smallestBlurComponent = 1.0f / (min(abs(blurDirection.x), abs(blurDirection.y)) + reduceAmount);

    blurDirection = clamp(blurDirection * smallestBlurComponent, float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX));

    blurDirection = blurDirection * textureDimentions;
    
    // Taking two samples, on both sides of blurDirection (with scaling direction down to ~1/6th either side)
    float3 oneThirdBack = float4(renderTexture.Sample(textureSampler, input.texCoord + (blurDirection * float2(1.0f / 3.0f - 0.5f, 1.0f / 3.0f - 0.5f)))).xyz;
    float3 oneThirdForward = float4(renderTexture.Sample(textureSampler, input.texCoord + (blurDirection * float2(2.0f / 3.0f - 0.5f, 2.0f / 3.0f - 0.5f)))).xyz;

    float3 resultOne = 0.5f * (oneThirdBack + oneThirdForward);
    
    // Taking two samples going all the way in blur direction.
    float3 resultTwo = resultOne * 0.5f + 0.25f * ((renderTexture.Sample(textureSampler, input.texCoord + (blurDirection * float2(0.0f / 3.0f - 0.5f, 0.0f / 3.0f - 0.5f))).xyz) +
                                (renderTexture.Sample(textureSampler, input.texCoord + (blurDirection * float2(3.0f / 3.0f - 0.5f, 3.0f / 3.0f - 0.5f)))).xyz);

    float3 lumanosityResult = dot(lumanosity, resultTwo);
    
    float lumanosityMinimum = min(min(lumanositySample.topLeft, lumanositySample.topRight), min(lumanositySample.bottomLeft, lumanositySample.bottomRight));
    lumanosityMinimum = min(lumanosityMinimum, lumanositySample.middle);

    float lumanosityMaximum = max(max(lumanositySample.topLeft, lumanositySample.topRight), max(lumanositySample.bottomLeft, lumanositySample.bottomRight));
    lumanosityMaximum = max(lumanosityMaximum, lumanositySample.middle);

    float lumanosityAverage = (lumanositySample.topLeft + lumanositySample.topRight + lumanositySample.middle + lumanositySample.bottomRight + lumanositySample.bottomLeft) / 5.0f;

    float3 result = float3(0.0f, 0.0f, 0.0f);

    float lumaResult = dot(lumanosity, resultTwo);

    // Test if sample has gone too far (outside range of values in lumanositySample). If so, get value from result one which is a much tigher range.
    if (lumaResult < lumanosityMinimum || lumaResult > lumanosityMaximum)
    {
        result = float3(resultOne);
    }
    else
    {
        result = float3(resultTwo);
    }

    return float4(result.xyz, 1.0f);
}