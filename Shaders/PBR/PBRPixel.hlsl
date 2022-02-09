cbuffer LightData : register(b0)
{
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

cbuffer Material : register(b2)
{
    float metallicValue;
    float3 padding2;
};

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D heightTexture : register(t3);

Texture2D depthTexture : register(t4);

Texture2D albedoMap : register(t5);
Texture2D metallicMap : register(t6);
Texture2D roughnessMap : register(t7);
Texture2D aoMap : register(t8);
Texture2D normalMap : register(t9);
Texture2D emmisiveMap : register(t10);

TextureCube environmentMap : register(t11);

SamplerState textureSampler : register(s0);
SamplerState clampTextureSampler : register(s1);

// This values needs to be same as set in Lights.hpp -> SHADOW_MAP_DIMENSION
static const int DEPTH_TEXTURE_DIMENSION = 2048;

struct PSInput
{
    float4 position : SV_Position;
    float3 worldSpacePosition : WORLD_SPACE_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float4 lightTransformedPosition : LIGHT_SPACE_POSITION;
    float3x3 TBN : TBN_MATRIX;
    matrix modelMatrix : MODEL_MATRIX;
    matrix shadowProjectionViewMatrix : SHADOW_MATRIX;
};

// Lower the viewing angle, better the reflection is (more pronounced effect).
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (float3(1.0f, 1.0f, 1.0f) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

// Normal distribution function using the GGx / Trowbridge-reitz model.
// alpha is roughness * roughness.
// This function describes how microfacets are distributing according to roughness.
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSquare = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0);
    float NdotHSquare = NdotH * NdotH;
	
    float numerator = alphaSquare;
    float denominator = (NdotHSquare * (alphaSquare - 1.0) + 1.0);
    denominator = 3.14159 * denominator * denominator;
	denominator = max(denominator, 0.000001f);

    return numerator / denominator;
}

// Geometry shadowing function using Schilck GGX Model.
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness;
    float k = (r) / 2.0;

    float numerator = max(NdotV, 0.0f);
    float denominator = NdotV * (1.0 - k) + k;
	denominator = max(denominator, 0.000001f);

    return numerator / denominator;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


float4 PsMain(PSInput input) : SV_Target
{
    float4 diffTexture = diffuseTexture.Sample(textureSampler, input.texCoord);
    float alpha = diffTexture.a;
    
    // Doesnt work as expected, should be fine as long as clear color is black.
    clip(alpha < 0.1f ? -1 : 1);

    float3 albedo = pow(albedoMap.Sample(textureSampler, input.texCoord).xyz, 2.2);

    float metallic = metallicValue;
    float roughness = roughnessMap.Sample(textureSampler, input.texCoord).x;
    float ao = aoMap.Sample(textureSampler, input.texCoord).x;

    float3 N = normalMap.Sample(textureSampler, input.texCoord).xyz;
    // Convert to -1, 1 range
  //  N = N * 2.0f - float3(1.0f, 1.0f, 1.0f);
    N = normalize(mul(input.TBN, N));

    float3 emmisive = emmisiveMap.Sample(textureSampler, input.texCoord).xyz;


    float3 V = normalize(cameraPosition - input.worldSpacePosition);

    // Calculate radiance for directional light and BRDF and light incident angle.
    float3 LO = float3(0.0f, 0.0f, 0.0f);

    float3 L = normalize(lightDirection - input.worldSpacePosition);
    float3 H = normalize(V + L);

    float3 radiance = lightColor * lightStrength;

    float3 F0 = float3(0.04, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, float3(metallic, metallic, metallic));
    float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    // calculate color based on rendering equation.

    // KD + KS = 1 according to energy conservation.
    float3 KS = FresnelSchlick(max(dot(H, V), 0.0f), F0);
    float3 KD = float3(1.0f, 1.0f, 1.0f) - KS;
    KD  = KD * (1.0f - metallicValue);

    float3 lambertComponent = albedo / 3.14159;

    // Calculate Cook - Torrance BRDF for the specular light part.
    float3 numerator = NDF * G * F;
    float denominator = max(4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f), 0.00001f);
    float3 specular = numerator / denominator;

    float3 BRDF = lambertComponent * KD + specular;

    float NDOTL = max(dot(N, L), 0.0f);
    float3 outgoingLight = emmisive + BRDF * lightColor  * lightStrength * NDOTL;

    // Sample from environment map
    float3 diffuse = environmentMap.Sample(textureSampler, N).rgb * albedo * KD * lightStrength;

    // In the rendering equation emissive light does not come under the integral part, hence directly adding it to outgoing color.
    float4 result = float4(outgoingLight + diffuse, 1.0f);
    result.a = alpha;

    return result;
}