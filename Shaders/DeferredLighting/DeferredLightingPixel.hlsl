Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D position : register(t2);
Texture2D specular : register(t3);

cbuffer CameraData : register(b0)
{
	float3 cameraPosition;
	float padding;
};

struct LightData
{
    // lightPosition is float4 due to padding reasons.
	float ambientStrength;
	float3 lightColor;

	float lightStrength;
	float3 lightDirection;
};

cbuffer LightBuffer : register(b1)
{
	LightData lightData[100];
}

struct PSInput
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

SamplerState textureSampler : register(s0);
SamplerState clampTextureSampler : register(s1);

static const int POINT_LIGHT_COUNT = 100;

float4 PsMain(PSInput input) : SV_Target
{
	float3 pixelPositionValue = position.Sample(textureSampler, input.texCoord).xyz;
	float3 albedoValue = albedo.Sample(textureSampler, input.texCoord).xyz;
	float3 normalValue = normal.Sample(textureSampler, input.texCoord).xyz;
	float3 specularValue = specular.Sample(textureSampler, input.texCoord).xyz;

	float3 ambient = albedoValue * 0.1f;
	float3 viewDir = normalize(cameraPosition - pixelPositionValue);

	float3 diffuse = float3(0.0f, 0.0f, 0.0);
	for (int i = 0; i < POINT_LIGHT_COUNT; i++)
	{
        float attenuation = 5.5f / pow(length(pixelPositionValue - lightData[i].lightDirection), 1);
		float3 lightDirection = normalize(lightData[i].lightDirection - pixelPositionValue);
		diffuse += (max(dot(normalValue, lightDirection), 0.0f) * albedoValue * lightData[i].lightColor * attenuation);
	}

	return float4(diffuse + ambient, 1.0f);
}