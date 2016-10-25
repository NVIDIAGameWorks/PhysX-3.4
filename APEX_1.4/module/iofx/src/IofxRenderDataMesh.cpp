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



bool IofxSharedRenderDataMeshImpl::createRenderLayout(IofxScene& iofxScene, uint32_t semantics, uint32_t maxObjectCount, RenderLayout_t& renderLayout)
{
	bool bIsUserDefinedLayout = false;
	IofxRenderCallback* renderCallback = iofxScene.getIofxRenderCallback();
	if (renderCallback)
	{
		if (renderCallback->getIofxMeshRenderLayout(renderLayout, maxObjectCount, semantics, mInteropFlags))
		{
			bIsUserDefinedLayout = renderLayout.isValid();
		}
	}
	if (!bIsUserDefinedLayout)
	{
		//make default layout for given set of semantics
		uint32_t offset = 0;
		{
			IofxMeshRenderLayoutElement::Enum element = IofxMeshRenderLayoutElement::POSITION_FLOAT3;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxMeshRenderLayoutElement::getFormat(element));
		}
		{
			IofxMeshRenderLayoutElement::Enum element = IofxMeshRenderLayoutElement::ROTATION_SCALE_FLOAT3x3;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxMeshRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::COLOR))
		{
			IofxMeshRenderLayoutElement::Enum element = IofxMeshRenderLayoutElement::COLOR_FLOAT4;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxMeshRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::DENSITY))
		{
			IofxMeshRenderLayoutElement::Enum element = IofxMeshRenderLayoutElement::DENSITY_FLOAT1;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxMeshRenderLayoutElement::getFormat(element));
		}
		if (semantics & (1 << IofxRenderSemantic::USER_DATA))
		{
			IofxMeshRenderLayoutElement::Enum element = IofxMeshRenderLayoutElement::USER_DATA_UINT1;
			renderLayout.offsets[element] = offset;
			offset += RenderDataFormat::getFormatDataSize(IofxMeshRenderLayoutElement::getFormat(element));
		}
		renderLayout.stride = offset;

		renderLayout.bufferDesc.setDefaults();
		renderLayout.bufferDesc.size = maxObjectCount * renderLayout.stride;
		renderLayout.bufferDesc.interopFlags = mInteropFlags;
	}
	PX_ASSERT((renderLayout.stride & 0x03) == 0);
	return bIsUserDefinedLayout;
}

void IofxSharedRenderDataMeshImpl::freeAllRenderStorage()
{
	if (mSharedRenderData.meshRenderBuffer != NULL)
	{
		mSharedRenderData.meshRenderBuffer->release();
		mSharedRenderData.meshRenderBuffer = NULL;
	}
}

bool IofxSharedRenderDataMeshImpl::allocAllRenderStorage(const RenderLayout_t& newRenderLayout, IofxRenderCallback* iofxRenderCallback)
{
	mSharedRenderData.meshRenderBuffer = iofxRenderCallback->createRenderBuffer(newRenderLayout.bufferDesc);
	return (mSharedRenderData.meshRenderBuffer != NULL);
}

}
} // namespace nvidia
