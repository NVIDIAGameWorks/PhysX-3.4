/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __PARTICLE_IOS_ACTOR_IMPL_H__
#define __PARTICLE_IOS_ACTOR_IMPL_H__

#include "Apex.h"

#include "ParticleIosActor.h"
#include "iofx/IofxActor.h"
#include "ParticleIosAssetImpl.h"
#include "InstancedObjectSimulationIntl.h"
#include "ParticleIosScene.h"
#include "ApexActor.h"
#include "ApexContext.h"
#include "ApexFIFO.h"
#include "ParticleIosCommonCode.h"
#include "InplaceStorage.h"
#include "ApexMirroredArray.h"
#include "PxParticleExt.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{
class FieldSamplerQueryIntl;
class FieldSamplerCallbackIntl;
}
namespace pxparticleios
{

/* Class for managing the interactions with each emitter */
class ParticleParticleInjector : public IosInjectorIntl, public ApexResourceInterface, public ApexResource, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	void	setPreferredRenderVolume(RenderVolume* volume);
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

	void	release();
	void	destroy();

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

	virtual uint32_t getActivePaticleCount() const;

	virtual void setObjectScale(float objectScale);

protected:
	ParticleIosActorImpl* 		mIosActor;
	IofxManagerClientIntl*	mIofxClient;
	RenderVolume* 	mVolume;
	physx::Array<uint16_t>		mRandomActorClassIDs;
	uint32_t					mLastRandomID;
	uint16_t					mVolumeID;

	uint32_t					mInjectorID;
	uint32_t					mSimulatedParticlesCount;


	/* insertion buffer */
	ApexFIFO<IosNewObject>	mInjectedParticles;

	ParticleParticleInjector(ResourceList& list, ParticleIosActorImpl& actor, uint32_t injectorID);
	~ParticleParticleInjector() {}

	void init(IofxAsset* iofxAsset);

	friend class ParticleIosActorImpl;
};


class ParticleIosActorImpl : public InstancedObjectSimulationIntl,
	public ParticleIosActor,
	public ApexResourceInterface,
	public ApexResource,
	public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ParticleIosActorImpl(ResourceList& list, ParticleIosAssetImpl& asset, ParticleIosScene& scene, IofxAsset& iofxAsset, bool isDataOnDevice);
	~ParticleIosActorImpl();

	// ApexInterface API
	void								release();
	void								destroy();

	// Actor API
	void								setPhysXScene(PxScene* s);
	PxScene*							getPhysXScene() const;
	virtual void						putInScene(PxScene* scene);

	Asset*						getOwner() const
	{
		READ_ZONE();
		return (Asset*) mAsset;
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
		WRITE_ZONE();
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
		return getParticleRadius();
	}
	float						getObjectDensity() const
	{
		return 1.0f;
	} // mAsset->getRestDensity(); }

	// ParticleIosActor
	float						getParticleRadius() const
	{
		return mAsset->getParticleRadius();
	}
	//float						getRestDensity() const			{ return mAsset->getRestDensity();}
	uint32_t						getParticleCount() const
	{
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

	void								visualize();
	virtual PxTaskID				submitTasks(PxTaskManager* tm) = 0;
	virtual void						setTaskDependencies(PxTaskID taskStartAfterID, PxTaskID taskFinishBeforeID) = 0;
	virtual void						fetchResults();

	IosInjectorIntl*						allocateInjector(IofxAsset* iofxAsset);
	void								releaseInjector(IosInjectorIntl&);

	virtual void								setDensityOrigin(const PxVec3& v) 
	{ 
		mDensityOrigin = v; 
	}

protected:
	virtual void						removeFromScene();

	void								injectNewParticles();
	bool								isParticleDescValid( const ParticleIosAssetParam* desc) const;
	void								initStorageGroups(InplaceStorage& storage);

	void								setTaskDependencies(PxTaskID taskStartAfterID, PxTaskID taskFinishBeforeID, PxTask* iosTask, bool isDataOnDevice);

	ParticleIosAssetImpl* 					mAsset;
	ParticleIosScene* 					mParticleIosScene;
	bool								mIsParticleSystem;	// true:SimpleParticleSystemParams , false:FluidParticleSystemParams
	PxActor*							mParticleActor;

	IofxManagerIntl* 						mIofxMgr;
	IosBufferDescIntl						mBufDesc;

	ResourceList						mInjectorList;

	physx::Array<IofxAsset*>			mIofxAssets;
	physx::Array<uint32_t>					mIofxAssetRefs;

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
	ApexMirroredArray<uint32_t>		mInjectorsCounters;
	ApexMirroredArray<uint32_t>		mInputIdToParticleIndex;

	ApexMirroredArray<float>		mGridDensityGrid;
	ApexMirroredArray<float>		mGridDensityGridLowPass;

	// Only for use by the IOS Asset, the actor is unaware of this
	bool								mIsMesh;
	
	FieldSamplerQueryIntl*				mFieldSamplerQuery;
	ApexMirroredArray<PxVec4>	mField;

	InplaceStorageGroup					mSimulationStorageGroup;

	class InjectTask : public PxTask
	{
	public:
		InjectTask(ParticleIosActorImpl& actor) : mActor(actor) {}

		const char* getName() const
		{
			return "ParticleIosActorImpl::InjectTask";
		}
		void run()
		{
			mActor.injectNewParticles();
		}

	protected:
		ParticleIosActorImpl& mActor;

	private:
		InjectTask& operator=(const InjectTask&);
	};
	InjectTask							mInjectTask;

	// Particle Density Origin
	PxVec3 mDensityOrigin;
	PxMat44 mDensityDebugMatInv;

	GridDensityParams		mGridDensityParams;

	FieldSamplerCallbackIntl*				mOnStartCallback;
	IofxManagerCallbackIntl*				mOnFinishCallback;

	friend class ParticleIosAssetImpl;
	friend class ParticleParticleInjector;
};

}
} // namespace nvidia

#endif // __PARTICLE_IOS_ACTOR_IMPL_H__
