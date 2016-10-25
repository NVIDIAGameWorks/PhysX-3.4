/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD__SAMPLER_PHYSX_MONITOR_H___
#define __FIELD__SAMPLER_PHYSX_MONITOR_H___

#include "Apex.h"
#include <PsArray.h>

#include "FieldSamplerSceneIntl.h"
#include "FieldSamplerPhysXMonitorParams.h"

#if PX_PHYSICS_VERSION_MAJOR == 3
#include <PxScene.h>
#endif

namespace nvidia
{
namespace apex
{
class FieldSamplerQueryIntl;
}

namespace fieldsampler
{

class FieldSamplerScene;
class FieldSamplerManager;


struct ShapeData : public UserAllocated
{
	uint32_t	fdIndex;
	uint32_t	rbIndex;
	float	mass;
	PxVec3	pos;
	PxVec3	vel;

	static bool sortPredicate (ShapeData* sd1, ShapeData* sd2)
	{
		if (sd1 == 0) return false;
		if (sd2 == 0) return true;
		return sd1->fdIndex < sd2->fdIndex;
	}
};


class FieldSamplerPhysXMonitor : public UserAllocated
{
private:


public:
	FieldSamplerPhysXMonitor(FieldSamplerScene& scene);
	virtual ~FieldSamplerPhysXMonitor();

#if PX_PHYSICS_VERSION_MAJOR == 3
	virtual void	update();
	virtual void	updatePhysX();

	/* PhysX scene management */
	void	setPhysXScene(PxScene* scene);
	PX_INLINE PxScene*	getPhysXScene() const
	{
		return mScene;
	}

	/* Toggle PhysX Monitor on/off */
	PX_INLINE void enablePhysXMonitor(bool enable)
	{
		mEnable = enable;
	}

	/* Is PhysX Monitor enabled */
	PX_INLINE bool isEnable()
	{
		return mEnable;
	}

	PX_INLINE void FieldSamplerPhysXMonitor::setPhysXFilterData(physx::PxFilterData filterData)
	{
		mFilterData = filterData;
	}

private:
	FieldSamplerPhysXMonitor& operator=(const FieldSamplerPhysXMonitor&);

	void getParticles(uint32_t taskId);
	void updateParticles();
	void getRigidBodies(uint32_t taskId);
	void updateRigidBodies();
	//void getCloth(PxTask& task, bool isDataOnDevice);
	//void updateCloth();

protected:

	void commonInitArray();

	FieldSamplerScene*				mFieldSamplerScene;
	FieldSamplerManager*			mFieldSamplerManager;

	PxScene*						mScene;
	
	FieldSamplerPhysXMonitorParams*	mParams;
	
	PxFilterData					mFilterData;

	//Particles
	uint32_t					mNumPS;  //Number of particle systems
	uint32_t					mPCount; //Number of particles in buffer
	Array<PxActor*>					mParticleSystems;
	Array<float>					mPSMass;
	Array<PxVec4>					mPSOutField;
	Array<PxVec3>					mOutVelocities;
	Array<uint32_t>					mOutIndices;
	Array<physx::PxParticleReadData*>		mParticleReadData;
	Array<FieldSamplerQueryIntl*>		mPSFieldSamplerQuery;	
	Array<PxTaskID>			mPSFieldSamplerTaskID;


	//Rigid bodies
	uint32_t					mNumRB; //Number of rigid bodies
	Array<PxActor*>					mRBActors;
	Array<ShapeData*>				mRBIndex;
	Array<PxVec4>					mRBInPosition;
	Array<PxVec4>					mRBInVelocity;
	Array<PxVec4>					mRBOutField;
	Array<PxFilterData>				mRBFilterData;
	Array<FieldSamplerQueryIntl*>		mRBFieldSamplerQuery;	

	//Enable or disable PhysX Monitor
	bool mEnable;

public:
	class RunAfterActorUpdateTask : public PxTask
	{
	public:
		RunAfterActorUpdateTask(FieldSamplerPhysXMonitor& owner) : mOwner(owner) {}
		const char* getName() const
		{
			return FSST_PHYSX_MONITOR_UPDATE;
		}
		void run()
		{
			mOwner.updatePhysX();
		}

	protected:
		FieldSamplerPhysXMonitor& mOwner;

	private:
		RunAfterActorUpdateTask operator=(const RunAfterActorUpdateTask&);
	};
	RunAfterActorUpdateTask				mTaskRunAfterActorUpdate;
#endif
};


}
} // end namespace nvidia::apex

#endif

