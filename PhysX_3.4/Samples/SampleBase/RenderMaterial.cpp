// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "SamplePreprocessor.h"
#include "RenderMaterial.h"
#include "RenderTexture.h"
#include "RendererMaterialDesc.h"
#include "Renderer.h"
#include "RendererMaterial.h"
#include "RendererMaterialInstance.h"
#include "RendererMemoryMacros.h"

using namespace physx;
using namespace SampleRenderer;

const char* defaultMaterialLitVertexShader = "vertex/staticmesh.cg";
const char* defaultMaterialTexturedUnlitFragmentShader = "fragment/sample_diffuse_and_texture.cg";
const char* defaultMaterialTexturedLitFragmentShader = "fragment/sample_diffuse_and_texture.cg";
const char* defaultMaterialFragmentShader = "fragment/sample_diffuse_no_texture.cg";

RenderMaterial::RenderMaterial(Renderer& renderer, const PxVec3& diffuseColor, PxReal opacity, bool doubleSided, PxU32 id, RenderTexture* texture, bool lit, bool flat, bool instanced) :
	mRenderMaterial			(NULL),
	mRenderMaterialInstance	(NULL),
	mID						(id),
	mDoubleSided			(doubleSided),
	mOwnsRendererMaterial	(true)
{
	RendererMaterialDesc matDesc;
	if(lit)
		matDesc.type			= RendererMaterial::TYPE_LIT;
	else 
		matDesc.type			= RendererMaterial::TYPE_UNLIT;
	matDesc.alphaTestFunc		= RendererMaterial::ALPHA_TEST_ALWAYS;
	matDesc.alphaTestRef		= 0.0f;
	if(opacity==1.0f)
	{
		matDesc.blending		= false;
		matDesc.srcBlendFunc	= RendererMaterial::BLEND_ONE;
		matDesc.dstBlendFunc	= RendererMaterial::BLEND_ONE;
	}
	else
	{
		matDesc.type			= RendererMaterial::TYPE_UNLIT;
		matDesc.blending		= true;
//		matDesc.srcBlendFunc	= RendererMaterial::BLEND_ONE;
//		matDesc.dstBlendFunc	= RendererMaterial::BLEND_ONE;
		matDesc.srcBlendFunc	= RendererMaterial::BLEND_SRC_ALPHA;
		matDesc.dstBlendFunc	= RendererMaterial::BLEND_ONE_MINUS_SRC_ALPHA;
	}

	if(instanced)
	{
		matDesc.instanced = true;
	}
	else
	{
		matDesc.instanced = false;
	}

	matDesc.geometryShaderPath	= NULL;
	
	matDesc.vertexShaderPath	= defaultMaterialLitVertexShader;
		
	if(texture)
	{
		if(lit)
		{
			matDesc.fragmentShaderPath	= defaultMaterialTexturedLitFragmentShader;
		}
		else
		{
			matDesc.fragmentShaderPath	= defaultMaterialTexturedUnlitFragmentShader;
		}
	}
	else
	{
		matDesc.fragmentShaderPath	= defaultMaterialFragmentShader;
	}
	PX_ASSERT(matDesc.isValid());

	mRenderMaterial = renderer.createMaterial(matDesc);
	mRenderMaterialInstance = new RendererMaterialInstance(*mRenderMaterial);

	setDiffuseColor(PxVec4(diffuseColor.x, diffuseColor.y, diffuseColor.z, opacity));
	setShadeMode(flat);

	update(renderer);

	if(texture)
	{
		const RendererMaterial::Variable* var = mRenderMaterialInstance->findVariable("diffuseTexture", RendererMaterial::VARIABLE_SAMPLER2D);
		//PX_ASSERT(var);
		if(var)
			mRenderMaterialInstance->writeData(*var, &texture->mTexture);
	}
}

RenderMaterial::RenderMaterial(Renderer& renderer, RendererMaterial* mat, RendererMaterialInstance* matInstance, PxU32 id) :
	mRenderMaterial			(mat),
	mRenderMaterialInstance	(matInstance),
	mID						(id),
	mDoubleSided			(false),
	mOwnsRendererMaterial	(false)
{
	update(renderer);
}

void RenderMaterial::update(SampleRenderer::Renderer& renderer)
{
	const RendererMaterial::Variable* var = mRenderMaterialInstance->findVariable("windowWidth", RendererMaterial::VARIABLE_FLOAT);
	if(var)
	{
		PxU32 tmpWindowWidth, tmpWindowHeight;
		renderer.getWindowSize(tmpWindowWidth, tmpWindowHeight);

		const PxReal windowWidth = PxReal(tmpWindowWidth);
		mRenderMaterialInstance->writeData(*var, &windowWidth);
	}

}

void RenderMaterial::setParticleSize(const PxReal particleSize)
{
	const RendererMaterial::Variable* var = mRenderMaterialInstance->findVariable("particleSize", RendererMaterial::VARIABLE_FLOAT);
	if(var)
	{
		mRenderMaterialInstance->writeData(*var, &particleSize);
	}	
}

void RenderMaterial::setDiffuseColor(const PxVec4& color)
{
	const RendererMaterial::Variable* var = mRenderMaterialInstance->findVariable("diffuseColor", RendererMaterial::VARIABLE_FLOAT4);
	if(var)
	{
		const PxReal data[] = { color.x, color.y, color.z, color.w };
		mRenderMaterialInstance->writeData(*var, data);
	}
}

void RenderMaterial::setShadeMode(bool flat)
{
	const RendererMaterial::Variable* var = mRenderMaterialInstance->findVariable("shadeMode", RendererMaterial::VARIABLE_FLOAT);
	if(var)
	{
		float shadeMode = flat?1.0f:0.0f;
		mRenderMaterialInstance->writeData(*var, &shadeMode);
	}
}

RenderMaterial::~RenderMaterial()
{
	if(mOwnsRendererMaterial)
	{
		DELETESINGLE(mRenderMaterialInstance);
		SAFE_RELEASE(mRenderMaterial);
	}
}
