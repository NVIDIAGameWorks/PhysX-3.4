/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_RENDER_DATA_H__
#define __IOFX_RENDER_DATA_H__

#include "ApexUsingNamespace.h"
#include "PsUserAllocated.h"
#include "IofxRenderable.h"
#include "IofxRenderCallback.h"
#include "UserRenderCallback.h"
#include "ApexRenderMeshAsset.h"
#include "UserOpaqueMesh.h"
#include "RenderAPIIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCuda.h"
#endif

namespace nvidia
{
using namespace apex;

namespace iofx
{

class IofxActorImpl;
class IofxScene;


class IofxSharedRenderDataBase : public RenderEntityIntl
{
public:
	virtual ~IofxSharedRenderDataBase() {}

	virtual void updateSemantics(IofxScene& scene, uint32_t semantics, uint32_t maxObjectCount, uint32_t activeObjectCount) = 0;

	virtual size_t getRenderBufferSize() const = 0;

#if APEX_CUDA_SUPPORT
	virtual CUdeviceptr getBufferMappedCudaPtr() const = 0;
#endif
	virtual void* getBufferMappedPtr() const = 0;

	PX_INLINE bool checkSemantics(uint32_t semantics) const
	{
		return (semantics & mSemantics) == semantics;
	}

	PX_INLINE uint32_t getSemantics() const
	{
		return mSemantics;
	}

	PX_INLINE uint32_t getInstanceID() const
	{
		return mInstanceID;
	}

	PX_INLINE IofxSharedRenderDataBase* incrementReferenceCount()
	{
		return static_cast<IofxSharedRenderDataBase*>(RenderEntityIntl::incrementReferenceCount());
	}

protected:
	IofxSharedRenderDataBase()
		: mInstanceID(uint32_t(-1))
		, mSemantics(0)
	{
	}

	uint32_t	mInstanceID;
	uint32_t	mSemantics;

private:
	IofxSharedRenderDataBase& operator=(const IofxSharedRenderDataBase&);
};

template <typename Impl, typename SRDT>
class IofxSharedRenderDataImpl : public RenderEntityIntlImpl<Impl, IofxSharedRenderDataBase>
{
public:
	virtual void updateSemantics(IofxScene& iofxScene, uint32_t newSemantics, uint32_t maxObjectCount, uint32_t activeObjectCount)
	{
		mSharedRenderData.maxObjectCount = maxObjectCount;
		mSharedRenderData.activeObjectCount = activeObjectCount;

		if (newSemantics == mSemantics)
		{
			return;
		}

		Impl::RenderLayout_t newRenderLayout;
		bool bIsUserDefinedLayout = false;
		if (newSemantics != 0)
		{
			bIsUserDefinedLayout = getImpl()->createRenderLayout(iofxScene, newSemantics, maxObjectCount, newRenderLayout);
			if (getImpl()->getRenderLayout().isTheSameAs(newRenderLayout))
			{
				mSemantics = newSemantics;
				return;
			}
		}

		IofxSharedRenderDataImpl::free();

		if (newSemantics != 0)
		{
			bool result = false;
			if (bIsUserDefinedLayout)
			{
				//only create render buffer through UserRenderCallback if layout was user defined through IofxRenderCallback!
				IofxRenderCallback* iofxRenderCallback = iofxScene.getIofxRenderCallback();
				if (iofxRenderCallback != NULL)
				{
					result = getImpl()->allocAllRenderStorage(newRenderLayout, iofxRenderCallback);
				}
			}
			if (!result)
			{
				IofxRenderCallbackImpl internalIofxRenderCallback;
				result = getImpl()->allocAllRenderStorage(newRenderLayout, &internalIofxRenderCallback);
			}
			if (result)
			{
				getImpl()->getRenderLayout() = newRenderLayout;
				mSemantics = newSemantics;
			}
		}
	}

	virtual void free()
	{
		RenderEntityIntlImpl<Impl, IofxSharedRenderDataBase>::free();

		mSemantics = 0;
		getImpl()->getRenderLayout().setDefaults();
	}

	virtual void* getBufferMappedPtr() const
	{
		return getImpl()->getRenderBufferState().getMappedPtr();
	}
#if APEX_CUDA_SUPPORT
	virtual CUdeviceptr getBufferMappedCudaPtr() const
	{
		return getImpl()->getRenderBufferState().getMappedCudaPtr();
	}
#endif

	virtual size_t getRenderBufferSize() const
	{
		return getImpl()->getRenderBufferDesc().size;
	}

	typedef SRDT SharedRenderData_t;

	PX_INLINE const SharedRenderData_t& getSharedRenderData() const
	{
		return mSharedRenderData;
	}

protected:
	IofxSharedRenderDataImpl(uint32_t instance, RenderInteropFlags::Enum interopFlags)
		: RenderEntityIntlImpl<Impl, IofxSharedRenderDataBase>(interopFlags)
	{
		mInstanceID = instance;
		mSharedRenderData.maxObjectCount = 0;
		mSharedRenderData.activeObjectCount = 0;
	}

	SharedRenderData_t	mSharedRenderData;
};

class IofxSharedRenderDataMeshImpl : public IofxSharedRenderDataImpl<IofxSharedRenderDataMeshImpl, IofxMeshSharedRenderData>
{
public:
	typedef IofxMeshRenderLayout RenderLayout_t;

	IofxSharedRenderDataMeshImpl(uint32_t instance, RenderInteropFlags::Enum interopFlags)
		: IofxSharedRenderDataImpl<IofxSharedRenderDataMeshImpl, IofxMeshSharedRenderData>(instance, interopFlags)
	{
		mSharedRenderData.meshRenderBuffer = NULL;
	}

	PX_INLINE const UserRenderBufferDesc& getRenderBufferDesc() const
	{
		return mSharedRenderData.meshRenderLayout.bufferDesc;
	}
	PX_INLINE const RenderBufferStateIntl& getRenderBufferState() const
	{
		return mMeshBufferState;
	}
	PX_INLINE RenderLayout_t& getRenderLayout()
	{
		return mSharedRenderData.meshRenderLayout;
	}
	PX_INLINE const RenderLayout_t& getRenderLayout() const
	{
		return mSharedRenderData.meshRenderLayout;
	}
	//
	PX_INLINE uint32_t getRenderStorageCount() const
	{
		return (mSharedRenderData.meshRenderBuffer != NULL) ? 1u : 0u;
	}
	PX_INLINE RenderStorageStateIntl& getRenderStorage(uint32_t index, UserRenderStorage* &renderStorage, RenderInteropFlags::Enum &interopFlags)
	{
		PX_UNUSED(index);
		PX_ASSERT(index == 0);
		renderStorage = mSharedRenderData.meshRenderBuffer;
		interopFlags = mSharedRenderData.meshRenderLayout.bufferDesc.interopFlags;
		return mMeshBufferState;
	}
	PX_INLINE void onMapRenderStorage(uint32_t index)
	{
		PX_UNUSED(index);
		PX_ASSERT(index == 0);
		mMeshBufferState.setMapRange(0, mSharedRenderData.activeObjectCount * mSharedRenderData.meshRenderLayout.stride);
	}
	//

	bool createRenderLayout(IofxScene& iofxScene, uint32_t semantics, uint32_t maxObjectCount, RenderLayout_t& newRenderLayout);
	bool allocAllRenderStorage(const RenderLayout_t& newRenderLayout, IofxRenderCallback* callback);
	void freeAllRenderStorage();

private:
	IofxSharedRenderDataMeshImpl& operator=(const IofxSharedRenderDataMeshImpl&);

	RenderBufferStateIntl mMeshBufferState;
};

class IofxSharedRenderDataSpriteImpl : public IofxSharedRenderDataImpl<IofxSharedRenderDataSpriteImpl, IofxSpriteSharedRenderData>
{
public:
	typedef IofxSpriteRenderLayout RenderLayout_t;

	IofxSharedRenderDataSpriteImpl(uint32_t instance, RenderInteropFlags::Enum interopFlags)
		: IofxSharedRenderDataImpl<IofxSharedRenderDataSpriteImpl, IofxSpriteSharedRenderData>(instance, interopFlags)
	{
		mSharedRenderData.spriteRenderBuffer = NULL;
		for (uint32_t i = 0; i < IofxSpriteRenderLayout::MAX_SURFACE_COUNT; ++i)
		{
			mSharedRenderData.spriteRenderSurfaces[i] = NULL;
		}		
	}

	PX_INLINE const UserRenderBufferDesc& getRenderBufferDesc() const
	{
		return mSharedRenderData.spriteRenderLayout.bufferDesc;
	}
	PX_INLINE const RenderBufferStateIntl& getRenderBufferState() const
	{
		return mSpriteBufferState;
	}
	PX_INLINE RenderLayout_t& getRenderLayout()
	{
		return mSharedRenderData.spriteRenderLayout;
	}
	PX_INLINE const RenderLayout_t& getRenderLayout() const
	{
		return mSharedRenderData.spriteRenderLayout;
	}

	//
	PX_INLINE uint32_t getRenderStorageCount() const
	{
		if (mSharedRenderData.spriteRenderLayout.surfaceCount > 0)
		{
			return mSharedRenderData.spriteRenderLayout.surfaceCount;
		}
		else
		{
			return (mSharedRenderData.spriteRenderBuffer != NULL) ? 1u : 0u;
		}
	}
	PX_INLINE RenderStorageStateIntl& getRenderStorage(uint32_t index, UserRenderStorage* &renderStorage, RenderInteropFlags::Enum &interopFlags)
	{
		if (mSharedRenderData.spriteRenderLayout.surfaceCount > 0)
		{
			PX_ASSERT(index < mSharedRenderData.spriteRenderLayout.surfaceCount);
			renderStorage = mSharedRenderData.spriteRenderSurfaces[index];
			interopFlags = mSharedRenderData.spriteRenderLayout.surfaceDescs[index].interopFlags;
			return mSpriteSurfaceStateArray[index];
		}
		else
		{
			PX_ASSERT(index == 0);
			renderStorage = mSharedRenderData.spriteRenderBuffer;
			interopFlags = mSharedRenderData.spriteRenderLayout.bufferDesc.interopFlags;
			return mSpriteBufferState;
		}
	}
	PX_INLINE void onMapRenderStorage(uint32_t index)
	{
		if (mSharedRenderData.spriteRenderLayout.surfaceCount == 0)
		{
			PX_UNUSED(index);
			PX_ASSERT(index == 0);
			mSpriteBufferState.setMapRange(0, mSharedRenderData.activeObjectCount * mSharedRenderData.spriteRenderLayout.stride);
		}
	}
	//

	bool createRenderLayout(IofxScene& iofxScene, uint32_t semantics, uint32_t maxObjectCount, RenderLayout_t& newRenderLayout);
	bool allocAllRenderStorage(const RenderLayout_t& newRenderLayout, IofxRenderCallback* callback);
	void freeAllRenderStorage();

#if APEX_CUDA_SUPPORT

	PX_INLINE CUarray getSurfaceMappedCudaArray(uint32_t index) const
	{
		PX_ASSERT(index < IofxSpriteRenderLayout::MAX_SURFACE_COUNT);
		return mSpriteSurfaceStateArray[index].getMappedCudaArray();
	}
#endif

	PX_INLINE bool getSurfaceMappedInfo(uint32_t index, UserRenderSurface::MappedInfo& mappedInfo) const
	{
		PX_ASSERT(index < IofxSpriteRenderLayout::MAX_SURFACE_COUNT);
		mappedInfo = mSpriteSurfaceStateArray[index].getMappedInfo();
		return mSpriteSurfaceStateArray[index].isMapped();
	}

private:
	IofxSharedRenderDataSpriteImpl& operator=(const IofxSharedRenderDataSpriteImpl&);

	RenderBufferStateIntl		mSpriteBufferState;
	RenderSurfaceStateIntl	mSpriteSurfaceStateArray[IofxSpriteRenderLayout::MAX_SURFACE_COUNT];
};


class IofxRenderableImpl : public IofxRenderable, public UserAllocated, public ApexRWLockable
{
public:
	// IofxRenderable methods
	virtual const physx::PxBounds3& getBounds() const
	{
		return mRenderBounds;
	}

	virtual void				update(IofxSharedRenderDataBase* sharedRenderData, const physx::PxBounds3& bounds, uint32_t startIndex, uint32_t objectCount, uint32_t visibleCount) = 0;

	virtual void				release()
	{
		bool triggerDelete = false;
		mRefCountLock.lock();
		if (mRefCount > 0)
		{
			triggerDelete = !(--mRefCount);
		}
		mRefCountLock.unlock();
		if (triggerDelete)
		{
			if (mRenderCallback != NULL)
			{
				mRenderCallback->onReleasingIofxRenderable(*this);
			}
			delete this;
		}
	}

	// Returns this if successful, NULL otherwise
	IofxRenderable*			incrementReferenceCount()
	{
		IofxRenderable* returnValue = NULL;
		mRefCountLock.lock();
		if (mRefCount > 0)
		{
			++mRefCount;
			returnValue = this;
		}
		mRefCountLock.unlock();
		return returnValue;
	}

	PX_INLINE int32_t			getReferenceCount() const
	{
		return mRefCount;
	}

	virtual ~IofxRenderableImpl()
	{
		WRITE_ZONE();

		PX_ASSERT(mRefCount == 0);
		if (mSharedRenderData != NULL)
		{
			mSharedRenderData->release();
		}
		if (mRenderResID != INVALID_RESOURCE_ID)
		{
			//this call decrements resource refCount
			GetInternalApexSDK()->getInternalResourceProvider()->releaseResource(mRenderResID);
		}
	}

protected:
	IofxRenderableImpl(IofxScene* iofxScene)
		: mRefCount(1)	// Ref count initialized to 1, assuming that whoever calls this constructor will store a reference
		, mRenderCallback(iofxScene->getIofxRenderCallback())
		, mSharedRenderData(NULL)
		, mRenderResID(INVALID_RESOURCE_ID)
	{
		mRenderBounds.setEmpty();
	}

	void initRenderResource(ResID renderResID, IofxCommonRenderData& renderData)
	{
		mRenderResID = renderResID;
		if (mRenderResID != INVALID_RESOURCE_ID)
		{
			ResourceProviderIntl* nrp = GetInternalApexSDK()->getInternalResourceProvider();
			renderData.renderResourceNameSpace = nrp->getResourceNameSpace(mRenderResID);
			renderData.renderResourceName = nrp->getResourceName(mRenderResID);
			renderData.renderResource = nrp->getResource(mRenderResID);
			//this call increments resource refCount
			nrp->setResource(renderData.renderResourceNameSpace, renderData.renderResourceName, renderData.renderResource, true, true);
		}
		else
		{
			renderData.renderResourceNameSpace = renderData.renderResourceName = "";
			renderData.renderResource = NULL;
		}
	}

	void update(IofxSharedRenderDataBase* sharedRenderData, const physx::PxBounds3& bounds)
	{
		if (mSharedRenderData != sharedRenderData)
		{
			if (mSharedRenderData != NULL)
			{
				mSharedRenderData->release();
			}
			mSharedRenderData = sharedRenderData->incrementReferenceCount();
		}
		mRenderBounds = bounds;
		if (mRenderCallback != NULL)
		{
			mRenderCallback->onUpdatedIofxRenderable(*this);
		}
	}

	volatile int32_t		mRefCount;
	AtomicLock				mRefCountLock;

	IofxRenderCallback*	mRenderCallback;
	IofxSharedRenderDataBase*	mSharedRenderData;

	physx::PxBounds3		mRenderBounds;

	ResID					mRenderResID;

	friend class IofxActorImpl;
};

class IofxMeshRenderable : public IofxRenderableImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	IofxMeshRenderable(IofxScene* iofxScene, ResID renderResID)
		: IofxRenderableImpl(iofxScene)
	{
		mMeshRenderData.startIndex = mMeshRenderData.objectCount = 0;
		mMeshRenderData.sharedRenderData = NULL;

		initRenderResource(renderResID, mMeshRenderData);
		ApexRenderMeshAsset* rma = static_cast<ApexRenderMeshAsset*>(static_cast<Asset*>(mMeshRenderData.renderResource));
		if (rma != NULL)
		{
			UserOpaqueMesh* opaqueMesh = rma->getOpaqueMesh();
			if (opaqueMesh != NULL)
			{
				//this call increments resource refCount
				GetInternalApexSDK()->getInternalResourceProvider()->setResource(APEX_OPAQUE_MESH_NAME_SPACE, rma->getName(), opaqueMesh, true, true);
			}
		}

		if (mRenderCallback != NULL)
		{
			mRenderCallback->onCreatedIofxRenderable(*this);
		}
	}

	virtual ~IofxMeshRenderable()
	{
		ApexRenderMeshAsset* rma = static_cast<ApexRenderMeshAsset*>(static_cast<Asset*>(mMeshRenderData.renderResource));
		if (rma != NULL)
		{
			UserOpaqueMesh* opaqueMesh = rma->getOpaqueMesh();
			if (opaqueMesh != NULL)
			{
				//this call decrements resource refCount
				GetInternalApexSDK()->getNamedResourceProvider()->releaseResource(APEX_OPAQUE_MESH_NAME_SPACE, rma->getName());
			}
		}
	}

	// IofxRenderableImpl methods
	virtual void				update(IofxSharedRenderDataBase* sharedRenderData, const physx::PxBounds3& bounds, uint32_t startIndex, uint32_t objectCount, uint32_t visibleCount)
	{
		mMeshRenderData.startIndex = startIndex;
		mMeshRenderData.objectCount = objectCount;
		PX_UNUSED(visibleCount);
		mMeshRenderData.sharedRenderData = &(static_cast<IofxSharedRenderDataMeshImpl*>(sharedRenderData)->getSharedRenderData());
		IofxRenderableImpl::update(sharedRenderData, bounds);
	}

	virtual Type							getType() const
	{
		return IofxRenderable::MESH;
	}

	virtual const IofxSpriteRenderData*	getSpriteRenderData() const
	{
		return NULL;
	}

	virtual const IofxMeshRenderData*		getMeshRenderData() const
	{
		return &mMeshRenderData;
	}

private:
	IofxMeshRenderData		mMeshRenderData;
};

class IofxSpriteRenderable : public IofxRenderableImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	IofxSpriteRenderable(IofxScene* iofxScene, ResID renderResID)
		: IofxRenderableImpl(iofxScene)
	{
		mSpriteRenderData.startIndex = mSpriteRenderData.objectCount = 0;
		mSpriteRenderData.visibleCount = 0;
		mSpriteRenderData.sharedRenderData = NULL;

		initRenderResource(renderResID, mSpriteRenderData);

		if (mRenderCallback != NULL)
		{
			mRenderCallback->onCreatedIofxRenderable(*this);
		}
	}

	virtual void				update(IofxSharedRenderDataBase* sharedRenderData, const physx::PxBounds3& bounds, uint32_t startIndex, uint32_t objectCount, uint32_t visibleCount)
	{
		mSpriteRenderData.startIndex = startIndex;
		mSpriteRenderData.objectCount = objectCount;
		mSpriteRenderData.visibleCount = visibleCount;
		mSpriteRenderData.sharedRenderData = &(static_cast<IofxSharedRenderDataSpriteImpl*>(sharedRenderData)->getSharedRenderData());
		IofxRenderableImpl::update(sharedRenderData, bounds);
	}

	virtual Type							getType() const
	{
		return IofxRenderable::SPRITE;
	}

	virtual const IofxSpriteRenderData*	getSpriteRenderData() const
	{
		return &mSpriteRenderData;
	}

	virtual const IofxMeshRenderData*		getMeshRenderData() const
	{
		return NULL;
	}

private:
	IofxSpriteRenderData		mSpriteRenderData;
};


class IofxRenderBuffer : public UserRenderBuffer,  public UserAllocated
{
public:
	IofxRenderBuffer(const UserRenderBufferDesc& desc)
	{
		mData = shdfnd::getAllocator().allocate(desc.size, "", __FILE__, __LINE__);
		if (mData == NULL)
		{
			PX_ASSERT(!"IofxRenderBuffer: failed to allocate CPU memory!");
		}
	}

	virtual ~IofxRenderBuffer()
	{
		if (mData)
		{
			shdfnd::getAllocator().deallocate(mData);
			mData = NULL;
		}
	}

	virtual void release()
	{
		PX_DELETE(this);
	}

	//CPU access
	virtual void* map(RenderMapType::Enum mapType, size_t offset, size_t size)
	{
		PX_UNUSED(mapType);
		PX_UNUSED(size);
		return mData ? static_cast<uint8_t*>(mData) + offset : NULL;
	}
	virtual void unmap()
	{
		//do nothing!
	}

	//GPU access
	virtual bool getCUDAgraphicsResource(CUgraphicsResource &ret)
	{
		PX_UNUSED(ret);
		return false;
	}

private:
	void*	mData;
};


class IofxRenderSurface : public UserRenderSurface,  public UserAllocated
{
public:
	IofxRenderSurface(const UserRenderSurfaceDesc& desc)
	{
		mPitch = uint32_t(desc.width) * RenderDataFormat::getFormatDataSize(desc.format);
		mData = shdfnd::getAllocator().allocate(mPitch * desc.height, "", __FILE__, __LINE__);
		if (mData == 0)
		{
			PX_ASSERT(!"IofxRenderSurface: failed to allocate CPU memory!");
		}
	}

	virtual ~IofxRenderSurface()
	{
		if (mData)
		{
			shdfnd::getAllocator().deallocate(mData);
			mData = 0;
		}
	}

	virtual void release()
	{
		PX_DELETE(this);
	}

	//CPU access
	virtual bool map(RenderMapType::Enum mapType, MappedInfo& info)
	{
		PX_UNUSED(mapType);
		PX_ASSERT(mapType == RenderMapType::MAP_READ);
		if (mData)
		{
			info.pData= mData;
			info.rowPitch = mPitch;
			info.depthPitch = 0;
			return true;
		}
		return false;
	}
	virtual void unmap()
	{
		//do nothing!
	}

	//GPU access
	virtual bool getCUDAgraphicsResource(CUgraphicsResource &ret)
	{
		PX_UNUSED(ret);
		return false;
	}

private:
	void*		mData;
	uint32_t	mPitch;
};

class IofxRenderCallbackImpl : public IofxRenderCallback
{
public:
	virtual UserRenderBuffer* createRenderBuffer(const UserRenderBufferDesc& desc)
	{
		return PX_NEW(IofxRenderBuffer)(desc);
	}
	virtual UserRenderSurface* createRenderSurface(const UserRenderSurfaceDesc& desc)
	{
		return PX_NEW(IofxRenderSurface)(desc);
	}
};


}
} // namespace nvidia

#endif /* __IOFX_RENDER_DATA_H__ */
