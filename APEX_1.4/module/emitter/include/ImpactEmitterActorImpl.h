/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IMPACT_EMITTER_ACTOR_IMPL_H__
#define __IMPACT_EMITTER_ACTOR_IMPL_H__

////////////////////////////////////////////////////////////////////////////////

#include "ImpactEmitterActor.h"
#include "ImpactEmitterAssetImpl.h"
#include "EmitterScene.h"
#include "ApexActor.h"
#include "ApexFIFO.h"
#include "ApexSharedUtils.h"
#include "PsUserAllocated.h"
#include "ApexRand.h"
#include "ApexRWLockable.h"

////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterBaseEvent;

namespace NvParameterized
{
class Interface;
};

namespace nvidia
{
namespace apex
{
class InstancedObjectSimulationIntl;
}
namespace emitter
{

////////////////////////////////////////////////////////////////////////////////

struct ImpactEventTriggerParams
{
	PxVec3 hitPos;
	PxVec3 hitDir;
	PxVec3 hitNorm;

	ImpactEventTriggerParams() {}
	ImpactEventTriggerParams(const PxVec3& p, const PxVec3& d, const PxVec3& n) : hitPos(p), hitDir(d), hitNorm(n) {}
};

class ImpactEmitterBaseEvent : public UserAllocated
{
public:
	ImpactEmitterBaseEvent(ImpactEmitterActorImpl* emitterActor) : mOwner(emitterActor), mValid(false) {}
	virtual ~ImpactEmitterBaseEvent() {}
	virtual void removeActorReference(ApexActor* a) = 0;

	virtual bool isValid()
	{
		return mValid;
	}
	virtual void trigger(const ImpactEventTriggerParams& params) = 0;
	virtual void setPreferredRenderVolume(RenderVolume*) = 0;

	virtual void submitTasks(PxTaskManager*) {}
	virtual void setTaskDependencies(PxTask*) {}

protected:
	ImpactEmitterActorImpl* 	mOwner;
	bool mValid;
};

////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterParticleEvent : public ImpactEmitterBaseEvent
{
public:
	static const size_t MAX_PARTICLES = 2048;

	enum eAxisType { AXIS_INCIDENT = 0, AXIS_NORMAL, AXIS_REFLECTION };

	ImpactEmitterParticleEvent(NvParameterized::Interface* eventParamPtr, ImpactEmitterAssetImpl& asset, ImpactEmitterActorImpl* emitterActor);

	virtual ~ImpactEmitterParticleEvent();

	virtual void removeActorReference(ApexActor* a)
	{
		if (mParticleInjector == a)
		{
			mParticleInjector = NULL;
		}
	}

	virtual void trigger(const ImpactEventTriggerParams& params);

	virtual void submitTasks(PxTaskManager* tm);
	virtual void setTaskDependencies(PxTask* tickTask);
	virtual void setPreferredRenderVolume(nvidia::apex::RenderVolume* vol);

protected:
	void run();

	void trigger(const PxVec3& hitPos, const PxVec3& hitDir, const PxVec3& hitNorm);
	void initParticle(const PxVec3& pos, const PxVec3 basis[], PxVec3& outPos, PxVec3& outVel, float& outLife);

	typedef physx::Array<ImpactEventTriggerParams> TriggerQueue;
	TriggerQueue	mTriggerQueue;
	PxTask* 	mEventTask;
	friend class ParticleEventTask;

	eAxisType		mAxis;
	float	mMinAngle;
	float	mMaxAngle;
	float	mMinSpeed;
	float	mMaxSpeed;
	float	mMinLife;
	float	mMaxLife;
	uint32_t	mParticleSpawnCount;
	IosInjectorIntl* 	mParticleInjector;
};

////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterEventSet : public UserAllocated
{
public:
	struct EventSetEntry
	{
		float dly;
		ImpactEmitterBaseEvent* evnt;

		EventSetEntry() : dly(0.0f), evnt(NULL) {};
		EventSetEntry(float d, ImpactEmitterBaseEvent* e) : dly(d), evnt(e) {};
	};

	ImpactEmitterEventSet() {};
	virtual ~ImpactEmitterEventSet();

	bool AddEvent(NvParameterized::Interface* eventParamPtr, ImpactEmitterAssetImpl& asset, ImpactEmitterActorImpl* emitterActor);

	physx::Array<EventSetEntry> entries;

};

////////////////////////////////////////////////////////////////////////////////

class ImpactEmitterActorImpl : public ImpactEmitterActor, public EmitterActorBase, public ApexResourceInterface, public ApexResource, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	class QueuedImpactEvent
	{
	public:
		QueuedImpactEvent() {};
		QueuedImpactEvent(const PxVec3& p, const PxVec3& d, const PxVec3& n, float t, ImpactEmitterBaseEvent* e)
			: triggerParams(p, d, n), triggerTime(t), eventDef(e) {}

		bool trigger(float time);

	protected:
		ImpactEventTriggerParams	triggerParams;
		float				triggerTime;
		ImpactEmitterBaseEvent* 	eventDef;
	};
	typedef physx::Array<QueuedImpactEvent> ImpactEventQueue;

	ImpactEmitterActorImpl(const ImpactEmitterActorDesc&, ImpactEmitterAssetImpl&, ResourceList&, EmitterScene&);
	ImpactEmitterActorImpl(const NvParameterized::Interface&, ImpactEmitterAssetImpl&, ResourceList&, EmitterScene&);

	~ImpactEmitterActorImpl();

	Renderable* 			getRenderable()
	{
		return NULL;
	}
	Actor* 				getActor()
	{
		return this;
	}

	void						getLodRange(float& min, float& max, bool& intOnly) const;
	float				getActiveLod() const;
	void						forceLod(float lod);
	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		WRITE_ZONE();
		ApexActor::setEnableDebugVisualization(state);
	}

	/* ApexResourceInterface, ApexResource */
	void				        release();
	uint32_t				getListIndex() const
	{
		return m_listIndex;
	}
	void				        setListIndex(class ResourceList& list, uint32_t index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	/* EmitterActorBase */
	void                        destroy();
	Asset*		        getOwner() const;
	void						visualize(RenderDebugInterface&);

	void						setPhysXScene(PxScene*)  {}
	PxScene*					getPhysXScene() const
	{
		return NULL;
	}

	void						submitTasks();
	void						setTaskDependencies();
	void						fetchResults();

	SceneIntl*                getApexScene() const;
	void						removeActorAtIndex(uint32_t index);
	void						setPreferredRenderVolume(nvidia::apex::RenderVolume*);

	ImpactEmitterAsset* 		getEmitterAsset() const;

	/* Override some asset settings at run time */

	/* Actor callable methods */
	void						registerImpact(const PxVec3& hitPos, const PxVec3& hitDir, const PxVec3& surfNorm, uint32_t surfType);

	void							setSeed(uint32_t seed)
	{
		mRand.setSeed(seed);
	}

protected:
	void						tick();

	float  mTime;
	physx::Array<ImpactEmitterEventSet>   mEventSets;
	
	physx::Array<IosInjectorIntl*> mActiveParticleInjectors;
	ImpactEventQueue mPendingEvents;

	ImpactEmitterAssetImpl* 		mAsset;
	EmitterScene* 				mScene;

	PxTask* 		mTickTask;
	friend class ImpactEmitterTickTask;

	nvidia::QDSRand				mRand;

	friend class ImpactEmitterParticleEvent;
};

}
} // end namespace nvidia

////////////////////////////////////////////////////////////////////////////////
#endif
