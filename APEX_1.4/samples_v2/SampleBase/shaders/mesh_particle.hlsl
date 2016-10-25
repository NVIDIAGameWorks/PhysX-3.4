#include "common_buffers.hlsl"
#include "lighting.hlsl"

SamplerState defaultSampler : register(s0);
Texture2D diffuseTexture : register(t0);

static const float POINT_SIZE = 1.00f;

struct VS_INPUT
{
	float3 position : POSITION0;
	float3 normal : NORMAL;
	float2 scale : TANGENT;
	float2 uv : TEXCOORD0;
	float3 instanceOffset : TEXCOORD9;
	float3 instanceRotX : TEXCOORD10;
	float3 instanceRotY : TEXCOORD11;
	float3 instanceRotZ : TEXCOORD12;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 worldPos : POSITION0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
};

VS_OUTPUT VS(VS_INPUT iV)
{
	VS_OUTPUT oV;

	float4x4 modelMatrix = mul(model, float4x4(float4(iV.instanceRotX, 0), float4(iV.instanceRotY, 0), float4(iV.instanceRotZ, 0), float4(iV.instanceOffset, 1)));
	
	float4 worldSpacePos = mul(float4(iV.position, 1.0f), modelMatrix);
	float4 eyeSpacePos = mul(worldSpacePos, view);
	oV.position = mul(eyeSpacePos, projection);

	oV.worldPos = worldSpacePos;
	
	oV.uv = iV.uv;
	
	float3 worldNormal = mul(float4(iV.normal, 0.0f), modelMatrix);
	oV.normal = worldNormal;
	
	return oV;
}

float4 PS(VS_OUTPUT iV) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(defaultSampler, iV.uv);
	return CalcPixelLight(textureColor, iV.worldPos, iV.normal);
}