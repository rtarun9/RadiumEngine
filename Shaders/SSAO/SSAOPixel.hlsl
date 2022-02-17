struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

Texture2D positionTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D noiseTexture : register(t2);

SamplerState textureSampler : register(s0);

cbuffer SSAOData : register(b0)
{
    float3 samples[64];
    matrix projectionMatrix;
};

// This is needed as noise texture has to cover / tile all over screen, but input.texCoord is in range of 0, 1.
// So, this is used to scale input.texCoords to as to tile the entire screen
static const float2 noiseScale = float2(1920.0f / 4.0f, 1080.0f / 4.0f);

float4 PsMain(PSInput input) : SV_Target
{
    float3 fragmentPosition = positionTexture.Sample(textureSampler, input.texCoord).xyz;
    float3 normal = normalTexture.Sample(textureSampler, input.texCoord).xyz;
    float3 randomRotationVector = noiseTexture.Sample(textureSampler, input.texCoord * noiseScale).xyz;

    // Construct TBN matrix to transform vector from tangent -> view space using Gramm Schimdt process
    float3 tangent = normalize(randomRotationVector - normal * dot(randomRotationVector, normal));
    float3 bitangent = cross(normal, tangent);

    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float occlusionFactor = 0.0f;
    for (int i = 0; i < 64; i++)
    {
        float3 samplePosition = mul(samples[i], TBN);

        samplePosition = fragmentPosition + samplePosition * 0.5f;

        // Transform samplePosition to screen space
        float4 offset = float4(samplePosition, 1.0f);
        offset = mul(projectionMatrix, offset);
        offset.xyz /= offset.w;
        offset.x = offset.x * 0.5f + 0.5f;
        offset.y = -offset.y * 0.5f + 0.5f;

        float sampleDepth = positionTexture.Sample(textureSampler, offset.xy).z;
        float rangeCheck = smoothstep(0.0f, 1.0f, 0.5f / abs(fragmentPosition.z - sampleDepth));

        occlusionFactor += (sampleDepth >= samplePosition.z + 0.025f ? 1.0f : 0.0f) * rangeCheck;
    }


    occlusionFactor = 1.0f - (occlusionFactor / 64.0f);
    occlusionFactor = pow(occlusionFactor, 2.0f);

    return float4(occlusionFactor, occlusionFactor, occlusionFactor, 1.0f);

}