#include "common_buffers.hlsl"

struct VS_INPUT
{
	float3 position : POSITION0;
	float3 color : COLOR0;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 color : COLOR0;
};

VS_OUTPUT VS(VS_INPUT iV)
{
	VS_OUTPUT oV;

	float4 worldSpacePos = mul(float4(iV.position, 1.0f), model);
	float4 eyeSpacePos = mul(worldSpacePos, view);
	oV.position = mul(eyeSpacePos, projection);

	oV.color = iV.color;

	return oV;
}

float4 PS(VS_OUTPUT iV) : SV_Target0
{
	return float4(iV.color, 1);
}