#include "common_buffers.hlsl"

static const float att_c = 1.0f;
static const float att_l = 0.014f;
static const float att_q = 0.0007f;


float CalcAttenuation(float distance)
{
	return 1 / (att_c + att_l * distance + att_q * distance * distance);
};


float4 CalcLight(float4 textureColor, float3 lightDir, float3 viewDir, float3 normal, float3 lightColor, float specPower, float specIntensity, float attenuation)
{
	normal = normalize(normal);

	// diffuse
	float3 dirToLight = normalize(-lightDir);
	float diffuseFactor = max(dot(normal, dirToLight), 0.0);
	float4 diffuse = float4(lightColor, 1) * textureColor * diffuseFactor * attenuation;

	// specular (Blinn-Phong)
	float3 halfwayDir = normalize(dirToLight + viewDir);
	float specFactor = pow(max(dot(viewDir, halfwayDir), 0.0), specPower);
	float4 spec = float4(lightColor, 1) * specFactor * attenuation * specIntensity;
	
	return diffuse + spec;
};

float4 CalcPixelLight(float4 diffuseColor, float4 worldPos, float3 normal)
{
	float3 viewDir = normalize(viewPos - worldPos);

	// ambient
	float4 ambient = float4(ambientColor, 1) * diffuseColor;

	// dir light
	float4 dirLight = CalcLight(diffuseColor, dirLightDir, viewDir, normal, dirLightColor, specularPower, specularIntensity, 1);

	// point light
	float3 pointLightDir = worldPos - pointLightPos;
	float distance = length(pointLightDir);
    float attenuation = CalcAttenuation(distance);
	float4 pointLight = CalcLight(diffuseColor, pointLightDir, viewDir, normal, pointLightColor, specularPower, specularIntensity, attenuation);
	
	return ambient + dirLight + pointLight;
};