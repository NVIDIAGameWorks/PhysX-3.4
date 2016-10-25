/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "IofxSceneCPU.h"
#include "IofxSceneGPU.h"
#include "IosObjectData.h"
#include "IofxRenderData.h"
#include "IofxActorImpl.h"

namespace nvidia
{
namespace iofx
{

bool IofxSharedRenderDataSpriteImpl::createRenderLayout(IofxScene& iofxScene, uint32_t semantics, uint32_t maxObjectCount, RenderLayout_t& renderLayout)
{
	bool bIsUserDefinedLayout = false;
	IofxRenderCallback* renderCallback = iofxScene.getIofxRenderCallback();
	if (renderCallback)
	{
		if (renderCallback->getIofxSpriteRenderLayout(renderLayout, maxObjectCount, semantics, mInteropFlags))
		{
			bIsUserDefinedLayout = renderLayout.isValid();
		}
	}
	if (!bIsUserDefinedLayout)
	{
		uint32_t offset = 0;
		if (semantics & (1 << IofxRenderSemantic::POSITION))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::POSITION_FLOAT3;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::COLOR))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::COLOR_FLOAT4;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::VELOCITY))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::VELOCITY_FLOAT3;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::SCALE))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::SCALE_FLOAT2;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::LIFE_REMAIN))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::LIFE_REMAIN_FLOAT1;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::DENSITY))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::DENSITY_FLOAT1;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::SUBTEXTURE))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::SUBTEXTURE_FLOAT1;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::ORIENTATION))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::ORIENTATION_FLOAT1;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::USER_DATA))
		{
			IofxSpriteRenderLayoutElement::Enum element = IofxSpriteRenderLayoutElement::USER_DATA_UINT1;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxSpriteRenderLayoutElement::getFormat(element));
		}
		renderLayout.stride = offset;
		renderLayout.surfaceCount = 0;

		renderLayout.bufferDesc.setDefaults();
		renderLayout.bufferDesc.size = maxObjectCount * renderLayout.stride;
		renderLayout.bufferDesc.interopFlags = mInteropFlags;
	}
	PX_ASSERT((renderLayout.stride & 0x03) == 0);
	return bIsUserDefinedLayout;
}

void IofxSharedRenderDataSpriteImpl::freeAllRenderStorage()
{
	if (mSharedRenderData.spriteRenderBuffer != NULL)
	{
		mSharedRenderData.spriteRenderBuffer->release();
		mSharedRenderData.spriteRenderBuffer = NULL;
	}

	for (uint32_t i = 0; i < IofxSpriteRenderLayout::MAX_SURFACE_COUNT; ++i)
	{
		if (mSharedRenderData.spriteRenderSurfaces[i] != NULL)
		{
			mSharedRenderData.spriteRenderSurfaces[i]->release();
			mSharedRenderData.spriteRenderSurfaces[i] = NULL;
		}
	}
}

bool IofxSharedRenderDataSpriteImpl::allocAllRenderStorage(const RenderLayout_t& newRenderLayout, IofxRenderCallback* iofxRenderCallback)
{
	bool result = false;
	if (newRenderLayout.surfaceCount > 0)
	{
		result = true;
		for (uint32_t i = 0; result && i < newRenderLayout.surfaceCount; ++i)
		{
			mSharedRenderData.spriteRenderSurfaces[i] = iofxRenderCallback->createRenderSurface(newRenderLayout.surfaceDescs[i]);
			result &= (mSharedRenderData.spriteRenderSurfaces[i] != NULL);
		}
		if (!result)
		{
			for (uint32_t i = 0; i < newRenderLayout.surfaceCount; ++i)
			{
				if (mSharedRenderData.spriteRenderSurfaces[i] != NULL)
				{
					mSharedRenderData.spriteRenderSurfaces[i]->release();
					mSharedRenderData.spriteRenderSurfaces[i] = NULL;
				}
			}
		}
	}
	else
	{
		mSharedRenderData.spriteRenderBuffer = iofxRenderCallback->createRenderBuffer(newRenderLayout.bufferDesc);
		result = (mSharedRenderData.spriteRenderBuffer != NULL);
	}
	return result;
}

}
} // namespace nvidia
