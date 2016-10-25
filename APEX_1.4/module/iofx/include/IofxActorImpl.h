/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_ACTOR_IMPL_H__
#define __IOFX_ACTOR_IMPL_H__

#include "Apex.h"
#include "IofxActor.h"
#include "InstancedObjectSimulationIntl.h"
#include "ResourceProviderIntl.h"
#include "ApexActor.h"
#include "IofxAssetImpl.h"
#include "IofxManagerIntl.h"
#include "IofxManager.h"
#include "ModifierImpl.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
class Modifier;
}
namespace iofx
{

class IofxScene;
class RenderVolumeImpl;

struct ObjectRange
{
	ObjectRange() : startIndex(0), objectCount(0) {}

	uint32_t			startIndex;	    //!< start index for this IOFX actor
	uint32_t			objectCount;	//!< count of object instances this frame
};


class IofxRenderableImpl;

class IofxActorImpl :
	public IofxActor,
	public ApexResourceInterface,
	public ApexActor,
	public ApexResource,
	public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	IofxActorImpl(ResID, IofxScene*, IofxManager&);
	~IofxActorImpl();

	// Actor methods
	void				setPhysXScene(PxScene*)	{ }
	PxScene* 			getPhysXScene() const
	{
		return NULL;
	}
	Asset* getOwner() const
	{
		READ_ZONE();
		return NULL;
	}
	Actor*        getActor()
	{
		return this;
	}
	const char* 		getIosAssetName() const
	{
		READ_ZONE();
		return mMgr.mIosAssetName.c_str();
	}
	IofxRenderable*	acquireRenderableReference();

	void				getLodRange(float& min, float& max, bool& intOnly) const;
	float		getActiveLod() const;
	void				forceLod(float lod);
	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		WRITE_ZONE();
		ApexActor::setEnableDebugVisualization(state);
	}

	// ApexResourceInterface methods
	void				release();
	void			    destroy();

	uint32_t	    getListIndex() const
	{
		return m_listIndex;
	}
	void	            setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	virtual float		getObjectRadius() const
	{
		READ_ZONE();
		return mMgr.getObjectRadius();
	}
	virtual uint32_t		getObjectCount() const
	{
		READ_ZONE();
		return mResultRange.objectCount;
	}
	virtual uint32_t		getVisibleCount() const
	{
		READ_ZONE();
		return mResultVisibleCount;
	}
	virtual physx::PxBounds3 getBounds() const
	{
		READ_ZONE();
		return mResultBounds;
	}

	bool						prepareRenderables(IosObjectBaseData* obj);


	ResID						mRenderResID;
	IofxScene*					mIofxScene;
	IofxManager&                mMgr;
	RenderVolumeImpl*           mRenderVolume;

	/* Set by IOFX Manager at creation time, immutable */
	uint32_t					mActorID;

	PxBounds3                   mResultBounds;
	ObjectRange					mResultRange;
	uint32_t					mResultVisibleCount;

	physx::Array<IofxRenderableImpl*> mRenderableArray;

	uint32_t					mSemantics;
	IofxRenderableImpl*		mActiveRenderable;

	bool						mDistanceSortingEnabled;

	friend class IofxManager;
	friend class IofxManagerGPU;
};

}
} // namespace nvidia

#endif // __IOFX_ACTOR_IMPL_H__
