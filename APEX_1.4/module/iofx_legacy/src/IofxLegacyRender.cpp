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
#include "IofxLegacyRender.h"
#include "ApexActor.h"
#include "PxGpuDispatcher.h"

namespace nvidia
{
namespace apex
{
namespace legacy
{


class IofxLegacyRenderData : public UserRenderData, public UserAllocated
{
public:
	virtual void release()
	{
		if (--mRefCount == 0)
		{
			PX_DELETE(this);
		}
	}
	virtual void addRef()
	{
		++mRefCount;
	}
protected:
	IofxLegacyRenderData()
		: mRefCount(0)
	{
	}
	virtual ~IofxLegacyRenderData() {}

	uint32_t mRefCount;
};

class IofxLegacySpriteRenderData : public IofxLegacyRenderData
{
public:
	IofxLegacySpriteRenderData()
		: mSpriteBuffer(NULL)
		, mSurfaceRefCount(0)
	{
	}
	virtual ~IofxLegacySpriteRenderData()
	{
		PX_ASSERT(mSurfaceRefCount == 0 && mSpriteBuffer == NULL);
	}

	uint32_t onAllocSurface()
	{
		if (mSpriteBuffer == NULL)
		{
			UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
			mSpriteBuffer = rrm->createSpriteBuffer(mSpriteBufferDesc);
		}
		return mSurfaceRefCount++;
	}
	void onReleaseSurface(uint32_t)
	{
		if (--mSurfaceRefCount == 0)
		{
			UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
			rrm->releaseSpriteBuffer(*mSpriteBuffer);
			mSpriteBuffer = NULL;
		}
	}

	UserRenderSpriteBufferDesc mSpriteBufferDesc;
	UserRenderSpriteBuffer* mSpriteBuffer;
	uint32_t mSurfaceRefCount;
};

class IofxLegacyInstanceRenderData : public IofxLegacyRenderData
{
public:
	UserRenderInstanceBufferDesc mInstanceBufferDesc;
};


class IofxLegacySpriteRenderSurface : public UserRenderSurface, public UserAllocated
{
public:
	IofxLegacySpriteRenderSurface(const UserRenderSurfaceDesc& desc)
		: mDesc(desc)
		, mMapBuffer(NULL)
	{
		IofxLegacySpriteRenderData* spriteUserData = static_cast<IofxLegacySpriteRenderData*>(mDesc.getUserData());
		mIndex = spriteUserData->onAllocSurface();
	}

	virtual ~IofxLegacySpriteRenderSurface()
	{
		if (mMapBuffer)
		{
			physx::shdfnd::getAllocator().deallocate(mMapBuffer);
		}

		IofxLegacySpriteRenderData* spriteUserData = static_cast<IofxLegacySpriteRenderData*>(mDesc.getUserData());
		spriteUserData->onReleaseSurface(mIndex);
	}

	virtual void release()
	{
		PX_DELETE(this);
	}

	PX_INLINE UserRenderSpriteBuffer* getSpriteBuffer()
	{
		IofxLegacySpriteRenderData* spriteUserData = static_cast<IofxLegacySpriteRenderData*>(mDesc.getUserData());
		PX_ASSERT(spriteUserData->mSpriteBuffer);
		return spriteUserData->mSpriteBuffer;
	}


	virtual bool map(RenderMapType::Enum mapType, MappedInfo& info)
	{
		PX_UNUSED(mapType);
		IofxLegacySpriteRenderData* spriteUserData = static_cast<IofxLegacySpriteRenderData*>(mDesc.getUserData());
		info.rowPitch = spriteUserData->mSpriteBufferDesc.textureDescs[mIndex].pitchBytes;
		info.depthPitch = 0;
		if (mMapBuffer == NULL)
		{
			size_t size = info.rowPitch * mDesc.height;
			mMapBuffer = physx::shdfnd::getAllocator().allocate(size, "StagingBuffer", __FILE__, __LINE__);
		}
		info.pData = mMapBuffer;
		return true;
	}
	virtual void unmap()
	{
		IofxLegacySpriteRenderData* spriteUserData = static_cast<IofxLegacySpriteRenderData*>(mDesc.getUserData());
		uint32_t rowPitch = spriteUserData->mSpriteBufferDesc.textureDescs[mIndex].pitchBytes;
		//TODO: copy only needed amount!
		getSpriteBuffer()->writeTexture(mIndex, uint32_t(mDesc.width * mDesc.height), mMapBuffer, rowPitch * mDesc.height);
	}

	virtual bool getCUDAgraphicsResource(CUgraphicsResource &ret)
	{
		CUgraphicsResource handleList[UserRenderSpriteBufferDesc::MAX_SPRITE_TEXTURES];
		if (getSpriteBuffer()->getInteropTextureHandleList(handleList))
		{
			ret = handleList[mIndex];
			return true;
		}
		return false;
	}

private:
	UserRenderSurfaceDesc		mDesc;
	uint32_t					mIndex;
	void*						mMapBuffer;
};

class IofxLegacyRenderBuffer : public UserRenderBuffer, public UserAllocated
{
public:
	enum Flags
	{
		NONE = 0,
		SPRITE_BUFFER,
		INSTANCE_BUFFER
	};

	virtual void release()
	{
		PX_DELETE(this);
	}

	virtual ~IofxLegacyRenderBuffer()
	{
		if (mStagingBuffer)
		{
			physx::shdfnd::getAllocator().deallocate(mStagingBuffer);
		}
	}

	virtual void* map(RenderMapType::Enum mapType, size_t offset, size_t size)
	{
		PX_UNUSED(mapType);
		if (mStagingBuffer == NULL)
		{
			mStagingBuffer = physx::shdfnd::getAllocator().allocate(mDesc.size, "StagingBuffer", __FILE__, __LINE__);
		}
		mMapOffset = PxMin(offset, mDesc.size);
		mMapSize = PxMin(size, mDesc.size - mMapOffset);
		return static_cast<uint8_t*>(mStagingBuffer) + offset;
	}

protected:
	IofxLegacyRenderBuffer(const UserRenderBufferDesc& desc)
		: mDesc(desc)
		, mStagingBuffer(NULL)
		, mMapOffset(0)
		, mMapSize(0)
	{
	}

	UserRenderBufferDesc	mDesc;
	void*					mStagingBuffer;

	size_t					mMapOffset;
	size_t					mMapSize;
};

class IofxLegacySpriteRenderBuffer : public IofxLegacyRenderBuffer
{
public:
	IofxLegacySpriteRenderBuffer(const UserRenderBufferDesc& desc)
		: IofxLegacyRenderBuffer(desc)
	{
		const UserRenderSpriteBufferDesc& spriteBufferDesc = 
			static_cast<IofxLegacySpriteRenderData*>(desc.getUserData())->mSpriteBufferDesc;

		UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
		mSpriteBuffer = rrm->createSpriteBuffer(spriteBufferDesc);
	}
	virtual ~IofxLegacySpriteRenderBuffer()
	{
		if (mSpriteBuffer)
		{
			UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
			rrm->releaseSpriteBuffer(*mSpriteBuffer);
		}
	}
	UserRenderSpriteBuffer* getSpriteBuffer()
	{
		return mSpriteBuffer;
	}

	virtual void unmap()
	{
		if (mSpriteBuffer && mMapSize > 0)
		{
			const UserRenderSpriteBufferDesc& spriteBufferDesc = 
				static_cast<IofxLegacySpriteRenderData*>(mDesc.getUserData())->mSpriteBufferDesc;
			PX_ASSERT(spriteBufferDesc.stride > 0);
			const uint32_t firstSprite = uint32_t(mMapOffset / spriteBufferDesc.stride);
			const uint32_t numSprites = uint32_t(mMapSize / spriteBufferDesc.stride);
			PX_ASSERT(firstSprite + numSprites <= spriteBufferDesc.maxSprites);
			mSpriteBuffer->writeBuffer(static_cast<uint8_t*>(mStagingBuffer) + mMapOffset, firstSprite, numSprites);
		}
	}

	//GPU access
	virtual bool getCUDAgraphicsResource(CUgraphicsResource &ret)
	{
		if (mSpriteBuffer)
		{
			return mSpriteBuffer->getInteropResourceHandle(ret);
		}
		return false;
	}

private:
	UserRenderSpriteBuffer* mSpriteBuffer;
};

class IofxLegacyInstanceRenderBuffer : public IofxLegacyRenderBuffer
{
public:
	IofxLegacyInstanceRenderBuffer(const UserRenderBufferDesc& desc)
		: IofxLegacyRenderBuffer(desc)
	{
		const UserRenderInstanceBufferDesc& instanceBufferDesc = 
			static_cast<IofxLegacyInstanceRenderData*>(desc.getUserData())->mInstanceBufferDesc;

		UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
		mInstanceBuffer = rrm->createInstanceBuffer(instanceBufferDesc);
	}
	virtual ~IofxLegacyInstanceRenderBuffer()
	{
		if (mInstanceBuffer)
		{
			UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
			rrm->releaseInstanceBuffer(*mInstanceBuffer);
		}
	}

	UserRenderInstanceBuffer* getInstanceBuffer()
	{
		return mInstanceBuffer;
	}

	//CPU access
	virtual void unmap()
	{
		if (mInstanceBuffer && mMapSize > 0)
		{
			const UserRenderInstanceBufferDesc& instanceBufferDesc = 
				static_cast<IofxLegacyInstanceRenderData*>(mDesc.getUserData())->mInstanceBufferDesc;
			PX_ASSERT(instanceBufferDesc.stride > 0);
			const uint32_t firstInstance = uint32_t(mMapOffset / instanceBufferDesc.stride);
			const uint32_t numInstances = uint32_t(mMapSize / instanceBufferDesc.stride);
			PX_ASSERT(firstInstance + numInstances <= instanceBufferDesc.maxInstances);
			mInstanceBuffer->writeBuffer(static_cast<uint8_t*>(mStagingBuffer) + mMapOffset, firstInstance, numInstances);
		}
	}

	//GPU access
	virtual bool getCUDAgraphicsResource(CUgraphicsResource &ret)
	{
		if (mInstanceBuffer)
		{
			return mInstanceBuffer->getInteropResourceHandle(ret);
		}
		return false;
	}

private:
	UserRenderInstanceBuffer* mInstanceBuffer;
};


void IofxLegacySpriteRenderable::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	URR_SCOPE;
	PX_UNUSED(rewriteBuffers);

	const IofxSpriteRenderData* spriteRenderData = mRenderable.getSpriteRenderData();
	if (spriteRenderData == NULL)
	{
		PX_ALWAYS_ASSERT();
		return;
	}
	UserRenderSpriteBuffer* spriteBuffer = NULL;
	if (spriteRenderData->sharedRenderData->spriteRenderBuffer)
	{
		IofxLegacyRenderBuffer* legacyRenderBuffer = static_cast<IofxLegacyRenderBuffer*>(spriteRenderData->sharedRenderData->spriteRenderBuffer);
		spriteBuffer = static_cast<IofxLegacySpriteRenderBuffer*>(legacyRenderBuffer)->getSpriteBuffer();
	}
	else if (spriteRenderData->sharedRenderData->spriteRenderSurfaces[0])
	{
		IofxLegacySpriteRenderSurface* legacySpriteRenderSurface = static_cast<IofxLegacySpriteRenderSurface*>(spriteRenderData->sharedRenderData->spriteRenderSurfaces[0]);
		spriteBuffer = legacySpriteRenderSurface->getSpriteBuffer();
	}

	if (mRenderResource == NULL || mRenderResource->getSpriteBuffer() != spriteBuffer)
	{
		UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();

		if (mRenderResource != NULL)
		{
			/* The sprite buffer was re-allocated for more semantics.  We
			 * must release our old render resource and allocate a new one.
			 */
			rrm->releaseResource(*mRenderResource);
			mRenderResource = NULL;
		}

		if (spriteBuffer != NULL)
		{
			UserRenderResourceDesc rDesc;
			rDesc.spriteBuffer = spriteBuffer;
			rDesc.firstSprite = 0;
			rDesc.numSprites = spriteRenderData->sharedRenderData->maxObjectCount;
			rDesc.userRenderData = userRenderData;
			rDesc.primitives = RenderPrimitiveType::POINT_SPRITES;
			rDesc.material = spriteRenderData->renderResource;

			mRenderResource = rrm->createResource(rDesc);
		}
	}

	if (mRenderResource != NULL)
	{
		PX_ASSERT( mRenderResource->getSpriteBuffer() == spriteBuffer );

		mRenderResource->setSpriteBufferRange(spriteRenderData->startIndex, spriteRenderData->objectCount);
		mRenderResource->setSpriteVisibleCount(spriteRenderData->visibleCount);
	}
}

void IofxLegacySpriteRenderable::dispatchRenderResources(UserRenderer& renderer)
{
	if (mRenderResource != NULL)
	{
		RenderContext context;
		context.world2local = physx::PxMat44(physx::PxIdentity);
		context.local2world = physx::PxMat44(physx::PxIdentity);
		context.renderResource = mRenderResource;
		renderer.renderResource(context);
	}
}


void IofxLegacyMeshRenderable::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	URR_SCOPE;

	const IofxMeshRenderData* meshRenderData = mRenderable.getMeshRenderData();
	if (meshRenderData == NULL)
	{
		PX_ALWAYS_ASSERT();
		return;
	}

	IofxLegacyRenderBuffer* legacyRenderBuffer = static_cast<IofxLegacyRenderBuffer*>(meshRenderData->sharedRenderData->meshRenderBuffer);
	UserRenderInstanceBuffer* instanceBuffer = static_cast<IofxLegacyInstanceRenderBuffer*>(legacyRenderBuffer)->getInstanceBuffer();

	if (mRenderMeshActor == NULL)
	{
		mRenderMeshActor = loadRenderMeshActor(meshRenderData);
	}
	if (mRenderMeshActor != NULL)
	{
		if (mRenderMeshActor->getInstanceBuffer() != instanceBuffer)
		{
			mRenderMeshActor->setInstanceBuffer(instanceBuffer);
		}

		PX_ASSERT( mRenderMeshActor->getInstanceBuffer() == instanceBuffer );

		mRenderMeshActor->setInstanceBufferRange(meshRenderData->startIndex, meshRenderData->objectCount);
		mRenderMeshActor->updateRenderResources(rewriteBuffers, userRenderData);
	}
}

void IofxLegacyMeshRenderable::dispatchRenderResources(UserRenderer& renderer)
{
	PX_ASSERT(mRenderMeshActor != NULL);
	if (mRenderMeshActor != NULL)
	{
		mRenderMeshActor->dispatchRenderResources(renderer);
	}
}

RenderMeshActor* IofxLegacyMeshRenderable::loadRenderMeshActor(const IofxMeshRenderData* meshRenderData)
{
	RenderMeshActor* rmactor = NULL;
	if (meshRenderData)
	{
		RenderMeshActorDesc renderableMeshDesc;
		renderableMeshDesc.maxInstanceCount = meshRenderData->sharedRenderData->maxObjectCount;

		RenderMeshAsset* meshAsset = static_cast<RenderMeshAsset*>(meshRenderData->renderResource);
		if (meshAsset)
		{
			rmactor = meshAsset->createActor(renderableMeshDesc);
			if (rmactor)
			{
				//this call is important to prevent renderResource release in case when there are no instances!
				rmactor->setReleaseResourcesIfNothingToRender(false);
			}
		}
	}
	return rmactor;
}


bool IofxLegacyRenderCallback::getIofxSpriteRenderLayout(IofxSpriteRenderLayout& spriteRenderLayout, uint32_t spriteCount, uint32_t spriteSemanticsBitmap, RenderInteropFlags::Enum interopFlags)
{
	IofxLegacySpriteRenderData* spriteRenderData = PX_NEW(IofxLegacySpriteRenderData)();
	UserRenderSpriteBufferDesc& spriteBufferDesc = spriteRenderData->mSpriteBufferDesc;

	uint32_t legacySemanticsBitmap = 0;
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::POSITION))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::POSITION);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::COLOR))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::COLOR);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::VELOCITY))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::VELOCITY);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::SCALE))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::SCALE);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::LIFE_REMAIN))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::LIFE_REMAIN);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::DENSITY))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::DENSITY);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::SUBTEXTURE))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::SUBTEXTURE);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::ORIENTATION))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::ORIENTATION);
	}
	if (spriteSemanticsBitmap & (1 << IofxRenderSemantic::USER_DATA))
	{
		legacySemanticsBitmap |= (1 << RenderSpriteSemantic::USER_DATA);
	}

	UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
	spriteBufferDesc.registerInCUDA = (interopFlags & RenderInteropFlags::CUDA_INTEROP) != 0;
	if (spriteBufferDesc.registerInCUDA)
	{
		spriteBufferDesc.interopContext = mGpuDispatcher->getCudaContextManager();
	}
	spriteBufferDesc.hint = RenderBufferHint::DYNAMIC;
	if (!rrm->getSpriteLayoutData(spriteCount, legacySemanticsBitmap, &spriteBufferDesc))
	{
		RenderDataFormat::Enum positionFormat = RenderSpriteLayoutElement::getSemanticFormat(RenderSpriteLayoutElement::POSITION_FLOAT3);
		RenderDataFormat::Enum colorFormat = RenderSpriteLayoutElement::getSemanticFormat(RenderSpriteLayoutElement::COLOR_BGRA8);
		const uint32_t positionElementSize = RenderDataFormat::getFormatDataSize(positionFormat);
		const uint32_t colorElementSize = RenderDataFormat::getFormatDataSize(colorFormat);
		spriteBufferDesc.semanticOffsets[RenderSpriteLayoutElement::POSITION_FLOAT3] = 0;
		spriteBufferDesc.semanticOffsets[RenderSpriteLayoutElement::COLOR_BGRA8] = positionElementSize;
		spriteBufferDesc.stride = positionElementSize + colorElementSize;
		spriteBufferDesc.maxSprites = spriteCount;
		spriteBufferDesc.textureCount = 0;
	}

	if (spriteBufferDesc.textureCount > 0)
	{
		static IofxSpriteRenderLayoutSurfaceElement::Enum convertTable[RenderSpriteTextureLayout::NUM_LAYOUTS] = {
			IofxSpriteRenderLayoutSurfaceElement::MAX_COUNT,
			IofxSpriteRenderLayoutSurfaceElement::POSITION_FLOAT4,
			IofxSpriteRenderLayoutSurfaceElement::SCALE_ORIENT_SUBTEX_FLOAT4,
			IofxSpriteRenderLayoutSurfaceElement::COLOR_RGBA8,
			IofxSpriteRenderLayoutSurfaceElement::COLOR_BGRA8,
			IofxSpriteRenderLayoutSurfaceElement::COLOR_FLOAT4
		};

		spriteRenderLayout.surfaceCount = spriteBufferDesc.textureCount;
		for (uint32_t i = 0; i < spriteRenderLayout.surfaceCount; ++i)
		{
			spriteRenderLayout.surfaceElements[i] = convertTable[ spriteBufferDesc.textureDescs[i].layout ];
			spriteRenderLayout.surfaceDescs[i].width = spriteBufferDesc.textureDescs[i].width;
			spriteRenderLayout.surfaceDescs[i].height = spriteBufferDesc.textureDescs[i].height;
			spriteRenderLayout.surfaceDescs[i].format = IofxSpriteRenderLayoutSurfaceElement::getFormat( spriteRenderLayout.surfaceElements[i] );

			spriteRenderLayout.surfaceDescs[i].interopFlags = spriteBufferDesc.registerInCUDA ? 
				RenderInteropFlags::CUDA_INTEROP : RenderInteropFlags::NO_INTEROP;

			spriteRenderLayout.surfaceDescs[i].userFlags = 0;
			spriteRenderLayout.surfaceDescs[i].setUserData(spriteRenderData);
		}
	}
	else
	{
		static RenderSpriteLayoutElement::Enum convertTable[IofxSpriteRenderLayoutElement::MAX_COUNT] = {
			RenderSpriteLayoutElement::POSITION_FLOAT3,
			RenderSpriteLayoutElement::COLOR_RGBA8,
			RenderSpriteLayoutElement::COLOR_BGRA8,
			RenderSpriteLayoutElement::COLOR_FLOAT4,
			RenderSpriteLayoutElement::VELOCITY_FLOAT3,
			RenderSpriteLayoutElement::SCALE_FLOAT2,
			RenderSpriteLayoutElement::LIFE_REMAIN_FLOAT1,
			RenderSpriteLayoutElement::DENSITY_FLOAT1,
			RenderSpriteLayoutElement::SUBTEXTURE_FLOAT1,
			RenderSpriteLayoutElement::ORIENTATION_FLOAT1,
			RenderSpriteLayoutElement::USER_DATA_UINT1
		};
		for (uint32_t dstElem = 0; dstElem < IofxSpriteRenderLayoutElement::MAX_COUNT; ++dstElem)
		{
			RenderSpriteLayoutElement::Enum srcElem = convertTable[dstElem];
			if (srcElem < RenderSpriteLayoutElement::NUM_SEMANTICS)
			{
				spriteRenderLayout.offsets[dstElem] = spriteBufferDesc.semanticOffsets[srcElem];
			}
		}
		spriteRenderLayout.stride = spriteBufferDesc.stride;
		spriteRenderLayout.surfaceCount = 0;

		spriteRenderLayout.bufferDesc.size = spriteRenderLayout.stride * spriteCount;
		spriteRenderLayout.bufferDesc.interopFlags = spriteBufferDesc.registerInCUDA ? 
			RenderInteropFlags::CUDA_INTEROP : RenderInteropFlags::NO_INTEROP;

		spriteRenderLayout.bufferDesc.userFlags = IofxLegacyRenderBuffer::SPRITE_BUFFER;
		spriteRenderLayout.bufferDesc.setUserData(spriteRenderData);
	}
	return true;
}

bool IofxLegacyRenderCallback::getIofxMeshRenderLayout(IofxMeshRenderLayout& meshRenderLayout, uint32_t meshCount, uint32_t meshSemanticsBitmap, RenderInteropFlags::Enum interopFlags)
{
	IofxLegacyInstanceRenderData* instanceRenderData = PX_NEW(IofxLegacyInstanceRenderData)();
	UserRenderInstanceBufferDesc& instanceBufferDesc = instanceRenderData->mInstanceBufferDesc;

	uint32_t legacySemanticsBitmap = 0;
	if (meshSemanticsBitmap & (1 << IofxRenderSemantic::POSITION))
	{
		legacySemanticsBitmap |= (1 << RenderInstanceSemantic::POSITION);
	}
	if (meshSemanticsBitmap & ((1 << IofxRenderSemantic::ROTATION) | (1 << IofxRenderSemantic::SCALE)))
	{
		legacySemanticsBitmap |= (1 << RenderInstanceSemantic::ROTATION_SCALE);
	}
	if (meshSemanticsBitmap & ((1 << IofxRenderSemantic::VELOCITY) | (1 << IofxRenderSemantic::LIFE_REMAIN)))
	{
		legacySemanticsBitmap |= (1 << RenderInstanceSemantic::VELOCITY_LIFE);
	}
	if (meshSemanticsBitmap & (1 << IofxRenderSemantic::DENSITY))
	{
		legacySemanticsBitmap |= (1 << RenderInstanceSemantic::DENSITY);
	}
	if (meshSemanticsBitmap & (1 << IofxRenderSemantic::COLOR))
	{
		legacySemanticsBitmap |= (1 << RenderInstanceSemantic::COLOR);
	}
	if (meshSemanticsBitmap & (1 << IofxRenderSemantic::USER_DATA))
	{
		legacySemanticsBitmap |= (1 << RenderInstanceSemantic::USER_DATA);
	}
	UserRenderResourceManager* rrm = GetInternalApexSDK()->getUserRenderResourceManager();
	instanceBufferDesc.registerInCUDA = (interopFlags & RenderInteropFlags::CUDA_INTEROP) != 0;
	if (instanceBufferDesc.registerInCUDA)
	{
		instanceBufferDesc.interopContext = mGpuDispatcher->getCudaContextManager();
	}
	instanceBufferDesc.hint = RenderBufferHint::DYNAMIC;
	if (!rrm->getInstanceLayoutData(meshCount, legacySemanticsBitmap, &instanceBufferDesc))
	{
		RenderDataFormat::Enum positionFormat = RenderInstanceLayoutElement::getSemanticFormat(RenderInstanceLayoutElement::POSITION_FLOAT3);
		RenderDataFormat::Enum rotationFormat = RenderInstanceLayoutElement::getSemanticFormat(RenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3);
		RenderDataFormat::Enum velocityFormat = RenderInstanceLayoutElement::getSemanticFormat(RenderInstanceLayoutElement::VELOCITY_LIFE_FLOAT4);
		const uint32_t positionElementSize = RenderDataFormat::getFormatDataSize(positionFormat);
		const uint32_t rotationElementSize = RenderDataFormat::getFormatDataSize(rotationFormat);
		const uint32_t velocityElementSize = RenderDataFormat::getFormatDataSize(velocityFormat);
		instanceBufferDesc.semanticOffsets[RenderInstanceLayoutElement::POSITION_FLOAT3]         = 0;
		instanceBufferDesc.semanticOffsets[RenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3] = positionElementSize;
		instanceBufferDesc.semanticOffsets[RenderInstanceLayoutElement::VELOCITY_LIFE_FLOAT4]    = positionElementSize + rotationElementSize;
		instanceBufferDesc.stride = positionElementSize + rotationElementSize + velocityElementSize;
		instanceBufferDesc.maxInstances = meshCount;
	}

	{
		static RenderInstanceLayoutElement::Enum convertTable[IofxMeshRenderLayoutElement::MAX_COUNT] = {
			RenderInstanceLayoutElement::POSITION_FLOAT3,
			RenderInstanceLayoutElement::ROTATION_SCALE_FLOAT3x3,
			RenderInstanceLayoutElement::POSE_FLOAT3x4,
			RenderInstanceLayoutElement::VELOCITY_LIFE_FLOAT4,
			RenderInstanceLayoutElement::DENSITY_FLOAT1,
			RenderInstanceLayoutElement::COLOR_RGBA8,
			RenderInstanceLayoutElement::COLOR_BGRA8,
			RenderInstanceLayoutElement::COLOR_FLOAT4,
			RenderInstanceLayoutElement::USER_DATA_UINT1
		};
		for (uint32_t dstElem = 0; dstElem < IofxMeshRenderLayoutElement::MAX_COUNT; ++dstElem)
		{
			RenderInstanceLayoutElement::Enum srcElem = convertTable[dstElem];
			if (srcElem < RenderInstanceLayoutElement::NUM_SEMANTICS)
			{
				meshRenderLayout.offsets[dstElem] = instanceBufferDesc.semanticOffsets[srcElem];
			}
		}
		meshRenderLayout.stride = instanceBufferDesc.stride;

		meshRenderLayout.bufferDesc.size = meshRenderLayout.stride * meshCount;
		meshRenderLayout.bufferDesc.interopFlags = instanceBufferDesc.registerInCUDA ? 
			RenderInteropFlags::CUDA_INTEROP : RenderInteropFlags::NO_INTEROP;

		meshRenderLayout.bufferDesc.userFlags = IofxLegacyRenderBuffer::INSTANCE_BUFFER;
		meshRenderLayout.bufferDesc.setUserData(instanceRenderData);
	}
	return true;
}

UserRenderBuffer* IofxLegacyRenderCallback::createRenderBuffer(const UserRenderBufferDesc& bufferDesc)
{
	switch (bufferDesc.userFlags)
	{
	case IofxLegacyRenderBuffer::SPRITE_BUFFER:
		return PX_NEW(IofxLegacySpriteRenderBuffer)(bufferDesc);
	case IofxLegacyRenderBuffer::INSTANCE_BUFFER:
		return PX_NEW(IofxLegacyInstanceRenderBuffer)(bufferDesc);
	default:
		return NULL;
	};
}

UserRenderSurface* IofxLegacyRenderCallback::createRenderSurface(const UserRenderSurfaceDesc& bufferDesc)
{
	return PX_NEW(IofxLegacySpriteRenderSurface)(bufferDesc);
}


}
}
} // namespace nvidia
