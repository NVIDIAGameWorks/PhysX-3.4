#include "common_buffers.hlsl"
#include "lighting.hlsl"

SamplerState defaultSampler : register(s0);
Texture2D diffuseTexture : register(t0);
Texture2D bonesTexture : register(t1);

struct VS_INPUT
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	int3 boneIndices : TEXCOORD5;
	float3 boneWeights : TEXCOORD6;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 worldPos : POSITION0;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL0;
};

float4x4 getBoneMatrix(int boneIndex)
{
	float4x4 boneMatrix;
	boneMatrix[0] = bonesTexture.Load(int3(0, boneIndex, 0));
	boneMatrix[1] = bonesTexture.Load(int3(1, boneIndex, 0));
	boneMatrix[2] = bonesTexture.Load(int3(2, boneIndex, 0));
	boneMatrix[3] = bonesTexture.Load(int3(3, boneIndex, 0));
	return boneMatrix;
}

float4x4 accumulate_skin(float4 boneIndices0, float4 boneWeights0)
{
	float4x4 result = boneWeights0.x * getBoneMatrix(boneIndices0.x);
	result = result + boneWeights0.y * getBoneMatrix(boneIndices0.y);
	result = result + boneWeights0.z * getBoneMatrix(boneIndices0.z);
	result = result + boneWeights0.w * getBoneMatrix(boneIndices0.w);
	return result;
}


VS_OUTPUT VS(VS_INPUT iV)
{
	VS_OUTPUT oV;

	//float4x4 boneMatrix = accumulate_skin(iV.boneIndices, iV.boneWeights);
	float4x4 boneMatrix = accumulate_skin(float4(iV.boneIndices, 0), float4(iV.boneWeights, 0));

	float3 skinnedPos = mul(float4(iV.position, 1.0f), boneMatrix);
	float4 worldSpacePos = mul(float4(skinnedPos, 1.0f), model);
	float4 eyeSpacePos = mul(worldSpacePos, view);
	oV.position = mul(eyeSpacePos, projection);

	oV.worldPos = worldSpacePos;

	// normals
	float3 localNormal = mul(float4(skinnedPos, 0.0f), boneMatrix);
	float3 worldNormal = mul(float4(localNormal, 0.0f), model);
	oV.normal = worldNormal;

	oV.uv = iV.uv;

	return oV;
}

float4 PS(VS_OUTPUT iV) : SV_Target0
{
	float4 textureColor = diffuseTexture.Sample(defaultSampler, iV.uv);
	return CalcPixelLight(textureColor, iV.worldPos, iV.normal);
}