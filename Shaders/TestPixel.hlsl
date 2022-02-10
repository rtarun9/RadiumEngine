cbuffer LightData : register(b0)
{
    // lightPosition is float4 due to padding reasons.
    float ambientStrength;
    float3 lightColor;

    float lightStrength;
    float3 lightDirection;
};

cbuffer CameraData : register(b1)
{
    float3 cameraPosition;
    float padding;
};

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D heightTexture : register(t3);

Texture2D depthTexture : register(t4);

SamplerState textureSampler : register(s0);
SamplerState clampTextureSampler : register(s1);

// This values needs to be same as set in Lights.hpp -> SHADOW_MAP_DIMENSION
static const int DEPTH_TEXTURE_DIMENSION = 2048;

struct PSInput
{
    float4 position : SV_Position;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float4 lightTransformedPosition : LIGHT_SPACE_POSITION;
    float3x3 TBN : TBN_MATRIX;
    matrix modelMatrix : MODEL_MATRIX;
    matrix shadowProjectionViewMatrix : SHADOW_MATRIX;
};


float CalculateShadow(float4 position)
{
    float4 shadowPosition = position;
    shadowPosition.xyz /= shadowPosition.w;

    shadowPosition.x = 0.5f * shadowPosition.x + 0.5f;
    shadowPosition.y = -0.5f * shadowPosition.y + 0.5f;

    if (shadowPosition.z > 1.0f)
    {
        return 0.0f;
    }

    float currentDepthValue = shadowPosition.z;

    float shadow = 0.0f;
    float bias = 0.005f; // max(0.05f * (1.0f -  dot(input.normal, lightDirection.xyz)), 0.005f);
    float texelSize =0.00008f;

    static const int KERNEL_WIDTH = 3;

    for (int i = -KERNEL_WIDTH; i <= KERNEL_WIDTH; i++)
    {
        for (int j = -KERNEL_WIDTH; j <= KERNEL_WIDTH; j++)
        {
            float pcfDepth = depthTexture.Sample(clampTextureSampler, shadowPosition.xy + float2(i, j) * texelSize).r;
            shadow += currentDepthValue - bias > pcfDepth ? 0.0f : 1.0f;
        }
    }

    // Number of iterations of inner + outer loop combined.
    return shadow / pow(KERNEL_WIDTH * KERNEL_WIDTH, 2);
}

float4 CalculateAmbientLight(PSInput input)
{
    float3 ambient = ambientStrength * lightColor * input.color;
    return float4(ambient, 1.0f);
}

float4 CalculateDiffuseLight(float3 normal, PSInput input)
{
    float3 lightDir = normalize(-lightDirection).rgb;
    float diffuse = max(dot(lightDir, normal), 0.0f);
    
    float4 result = float4(diffuse, diffuse, diffuse, 1.0f);
    return result;
}


float4 PsMain(PSInput input) : SV_Target
{
    float4 diffTexture = diffuseTexture.Sample(textureSampler, input.texCoord);
    float alpha = diffTexture.a;
    
    // Doesnt work as expected, should be fine as long as clear color is black.
    clip(alpha < 0.1f ? -1 : 1);

    float shadowResult = CalculateShadow(input.lightTransformedPosition);


    float3 normal = normalTexture.Sample(clampTextureSampler, input.texCoord).xyz;
    // Convert to -1, 1 range
    normal = normal * 2.0f - float3(1.0f, 1.0f, 1.0f);
    normal = normalize(mul(normal, input.TBN));

    float4 lightCalculationResult = CalculateAmbientLight(input) + (CalculateDiffuseLight(normalize(input.normal), input) * shadowResult * lightStrength);

    float4 result = float4(lightColor, 1.0f) * lightCalculationResult * diffTexture;
    
    result.a = alpha;
    return result;
}