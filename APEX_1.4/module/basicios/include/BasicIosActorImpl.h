/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_IOS_ACTOR_IMPL_H__
#define __BASIC_IOS_ACTOR_IMPL_H__

#include "Apex.h"

#include "BasicIosActor.h"
#include "IofxActor.h"
#include "BasicIosAssetImpl.h"
#include "InstancedObjectSimulationIntl.h"
#include "BasicIosScene.h"
#include "ApexActor.h"
#include "ApexContext.h"
#include "ApexFIFO.h"
#include "BasicIosCommon.h"
#include "BasicIosCommonSrc.h"
#include "InplaceStorage.h"
#include "ApexMirroredArray.h"
#include "ApexRand.h"

#include <PxFiltering.h>

#define MAX_CONVEX_PLANES_COUNT         4096
#define MAX_CONVEX_VERTS_COUNT          4096
#define MAX_CONVEX_POLYGONS_DATA_SIZE   16384

#define MAX_TRIMESH_VERTS_COUNT         4096
#define MAX_TRIMESH_INDICES_COUNT       16384

namespace nvidia
{
namespace apex
{
	class FieldSamplerQueryIntl;
	class FieldSamplerCallbackIntl;
}
namespace iofx
{
	class RenderVolume;
}

namespace basicios
{

/* Class for managing the interactions with each emitter */
class BasicParticleInjector : public IosInjectorIntl, public ApexResourceInterface, public ApexResource
{
public:
	void	setPreferredRenderVolume(nvidia::apex::RenderVolume* volume);
	float	getLeastBenefitValue() const
	{
		return 0.0f;
	}
	bool	isBacklogged() const
	{
		return false;
	}

	void	createObjects(uint32_t count, const IosNewObject* createList);
#if APEX_CUDA_SUPPORT
	void	createObjects(ApexMirroredArray<const IosNewObject>& createArray);
#endif
	
	void	setLODWeights(float maxDistance, float distanceWeight, float speedWeight, float lifeWeight, float separationWeight, float bias);

	PxTaskID getCompletionTaskID() const;

	virtual void release();
	void destroy();

	// ApexResourceInterface methods
	void	setListIndex(ResourceList& list, uint32_t index);
	uint32_t		getListIndex() const
	{
		return m_listIndex;
	}

	virtual void		setPhysXScene(PxScene*)	{}
	virtual PxScene*	getPhysXScene() const
	{
		return NULL;
	}

	void				assignSimParticlesCount(uint32_t input)
	{
		mSimulatedParticlesCount = input;
	}

	virtual uint32_t		getSimParticlesCount() const
	{
		return mSimulatedParticlesCount;
	}
	
	virtual uint32_t getActivePaticleCount() const
	{
		return mSimulatedParticlesCount;
	}

	virtual void setObjectScale(float objectScale);

protected:
	BasicIosActorImpl* 			mIosActor;
	IofxManagerClientIntl*	mIofxClient;
	nvidia::apex::RenderVolume* mVolume;
	physx::Array<uint16_t>		mRandomActorClassIDs;
	uint32_t					mLastRandomID;
	uint16_t					mVolumeID;

	uint32_t					mInjectorID;
	uint32_t					mSimulatedParticlesCount;

	/* insertion buffer */
	ApexFIFO<IosNewObject>	mInjectedParticles;

	QDSRand					mRand;

	BasicParticleInjector(ResourceList& list, BasicIosActorImpl& actor, uint32_t injectorID);
	~BasicParticleInjector();

	void init(nvidia::apex::IofxAsset* iofxAsset);

	friend class BasicIosActorImpl;
};


class BasicIosActorImpl : public InstancedObjectSimulationIntl,
	public BasicIosActor,
	public ApexResourceInterface,
	public ApexResource,
	public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	BasicIosActorImpl(ResourceList& list, BasicIosAssetImpl& asset, BasicIosScene& scene, nvidia::apex::IofxAsset& iofxAsset, bool isDataOnDevice);
	~BasicIosActorImpl();

	// ApexInterface API
	void								release();
	void								destroy();

	// Actor API
	void								setPhysXScene(PxScene* s);
	PxScene*							getPhysXScene() const;
	virtual void						putInScene(PxScene* scene);

	Asset*						getOwner() const
	{
		return static_cast<Asset*>(mAsset);
	}

	// ApexContext
	void								getLodRange(float& min, float& max, bool& intOnly) const;
	float						getActiveLod() const;
	void								forceLod(float lod);
	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		ApexActor::setEnableDebugVisualization(state);
	}

	// ApexResourceInterface methods
	void								setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	uint32_t						getListIndex() const
	{
		return m_listIndex;
	}

	// IOSIntl
	float						getObjectRadius() const
	{
		return mAsset->getParticleRadius();
	}
	float						getObjectDensity() const
	{
		return mAsset->getRestDensity();
	}

	// BasicIosActor
	float						getParticleRadius() const
	{
		READ_ZONE();
		return mAsset->getParticleRadius();
	}
	float						getRestDensity() const
	{
		READ_ZONE();
		return mAsset->getRestDensity();
	}
	uint32_t						getParticleCount() const
	{
		READ_ZONE();
		return mParticleCount;
	}
	PX_INLINE void						setOnStartFSCallback(FieldSamplerCallbackIntl* callback)
	{
		if (mFieldSamplerQuery)
		{
			mFieldSamplerQuery->setOnStartCallback(callback);
		}
	}
	PX_INLINE void						setOnFinishIOFXCallback(IofxManagerCallbackIntl* callback)
	{
		if (mIofxMgr)
		{
			mIofxMgr->setOnFinishCallback(callback);
		}
	}

	const PxVec3* 				getRecentPositions(uint32_t& count, uint32_t& stride) const;

	PxVec3 						getGravity() const;
	void								setGravity(PxVec3& gravity);

	void								visualize();
	virtual void						submitTasks() = 0;
	virtual void						setTaskDependencies() = 0;
	virtual void						fetchResults();

	IosInjectorIntl*						allocateInjector(IofxAsset* iofxAsset);
	void								releaseInjector(IosInjectorIntl&);

	virtual void								setDensityOrigin(const PxVec3& v) 
	{ 
		mDensityOrigin = v; 
	}


protected:
	//Member functions below have implemented in test actor
	virtual bool checkBenefit(uint32_t totalCount) {PX_UNUSED(totalCount); return true;};
	virtual bool checkHoles(uint32_t totalCount) {PX_UNUSED(totalCount); return true;};
	virtual bool checkInState(uint32_t totalCount) {PX_UNUSED(totalCount); return true;};
	virtual bool checkHistogram(uint32_t /*bound*/, uint32_t /*beg*/, uint32_t /*back*/) {return true;};

	virtual void						removeFromScene();

	void								injectNewParticles();

	void								initStorageGroups(InplaceStorage& storage);

	void								setTaskDependencies(PxTask* iosTask, bool isDataOnDevice);

	void								FillCollisionData(CollisionData& baseData, PxShape* shape);

	BasicIosAssetImpl* 						mAsset;
	BasicIosScene* 						mBasicIosScene;

	IofxManagerIntl* 						mIofxMgr;
	IosBufferDescIntl						mBufDesc;

	ResourceList						mInjectorList;

	PxVec3						mGravityVec;
	PxVec3						mUp;
	float						mGravity;
	float						mTotalElapsedTime;			//AM: People, methinks this will backfire eventually due to floating point precision loss!

	uint32_t						mMaxParticleCount;
	uint32_t						mMaxTotalParticleCount;

	uint32_t						mParticleCount;
	uint32_t						mParticleBudget;

	uint32_t						mInjectedCount;
	float						mInjectedBenefitSum;
	float						mInjectedBenefitMin;
	float						mInjectedBenefitMax;

	uint32_t						mLastActiveCount;
	float						mLastBenefitSum;
	float						mLastBenefitMin;
	float						mLastBenefitMax;

	ApexMirroredArray<float>		mLifeSpan;
	ApexMirroredArray<float>		mLifeTime;
	ApexMirroredArray<uint32_t>		mInjector;
	ApexMirroredArray<float>		mBenefit;

	ApexMirroredArray<PxPlane>	mConvexPlanes;
	ApexMirroredArray<PxVec4>	mConvexVerts;
	ApexMirroredArray<uint32_t>		mConvexPolygonsData;

	ApexMirroredArray<PxVec4>	mTrimeshVerts;
	ApexMirroredArray<uint32_t>		mTrimeshIndices;

	ApexMirroredArray<uint32_t>		mInjectorsCounters;

	ApexMirroredArray<float>		mGridDensityGrid;
	ApexMirroredArray<float>		mGridDensityGridLowPass;

	GridDensityParams					mGridDensityParams;
	// Particle Density Origin
	PxVec3 mDensityOrigin;
	PxMat44 mDensityDebugMatInv;

	// If true we are
	bool mTrackGravityChanges;

	// Only for use by the IOS Asset, the actor is unaware of this
	bool mIsMesh;

	FieldSamplerQueryIntl*				mFieldSamplerQuery;
	ApexMirroredArray<PxVec4>	mField;

	PxFilterData						mCollisionFilterData;
	Array<physx::PxOverlapHit>			mOverlapHits;

	InplaceStorageGroup					mSimulationStorageGroup;
	InplaceHandle<SimulationParams>		mSimulationParamsHandle;

	PxTask*				mInjectTask;

	FieldSamplerCallbackIntl*				mOnStartCallback;
	IofxManagerCallbackIntl*				mOnFinishCallback;

	friend class BasicIosInjectTask;

	friend class BasicIosAssetImpl;
	friend class BasicParticleInjector;
};

}
} // namespace nvidia

#endif // __BASIC_IOS_ACTOR_IMPL_H__
