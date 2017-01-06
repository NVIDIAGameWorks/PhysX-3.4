/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "ApexRenderResourceManager.h"
#include "ApexRenderResources.h"


bool ApexRenderResourceManager::getInstanceLayoutData(uint32_t particleCount,
	uint32_t particleSemanticsBitmap,
	nvidia::apex::UserRenderInstanceBufferDesc* bufferDesc)
{
	PX_UNUSED(particleSemanticsBitmap);
	using namespace nvidia::apex;
	RenderDataFormat::Enum positionFormat = RenderInstanceLayoutElement::getSemanticFormat(RenderInstanceLayoutElement::POSITION_FLOAT3);
	RenderDataFormat::Enum rotationFormat = RenderInstanceLayoutElement::getSemanticFormat(RenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3);
	const uint32_t positionElementSize = RenderDataFormat::getFormatDataSize(positionFormat);
	const uint32_t rotationElementSize = RenderDataFormat::getFormatDataSize(rotationFormat);
	bufferDesc->semanticOffsets[RenderInstanceLayoutElement::POSITION_FLOAT3] = 0;
	bufferDesc->semanticOffsets[RenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3] = positionElementSize;
	uint32_t strideInBytes = positionElementSize + rotationElementSize;;
	bufferDesc->stride = strideInBytes;
	bufferDesc->maxInstances = particleCount;
	return true;
}

bool ApexRenderResourceManager::getSpriteLayoutData(uint32_t spriteCount,
	uint32_t spriteSemanticsBitmap,
	nvidia::apex::UserRenderSpriteBufferDesc* bufferDesc)
{
	PX_UNUSED(spriteSemanticsBitmap);
	RenderDataFormat::Enum positionFormat = RenderSpriteLayoutElement::getSemanticFormat(RenderSpriteLayoutElement::POSITION_FLOAT3);
	RenderDataFormat::Enum colorFormat = RenderSpriteLayoutElement::getSemanticFormat(RenderSpriteLayoutElement::COLOR_BGRA8);
	RenderDataFormat::Enum scaleFormat = RenderSpriteLayoutElement::getSemanticFormat(RenderSpriteLayoutElement::SCALE_FLOAT2);
	const uint32_t positionElementSize = RenderDataFormat::getFormatDataSize(positionFormat);
	const uint32_t colorElementSize = RenderDataFormat::getFormatDataSize(colorFormat);
	const uint32_t scaleElementSize = RenderDataFormat::getFormatDataSize(scaleFormat);
	bufferDesc->semanticOffsets[RenderSpriteLayoutElement::POSITION_FLOAT3] = 0;
	bufferDesc->semanticOffsets[RenderSpriteLayoutElement::COLOR_BGRA8] = positionElementSize;
	bufferDesc->semanticOffsets[RenderSpriteLayoutElement::SCALE_FLOAT2] = positionElementSize + colorElementSize;
	uint32_t strideInBytes = positionElementSize + colorElementSize + scaleElementSize;
	bufferDesc->stride = strideInBytes;
	bufferDesc->maxSprites = spriteCount;
	bufferDesc->textureCount = 0;
	return true;
}