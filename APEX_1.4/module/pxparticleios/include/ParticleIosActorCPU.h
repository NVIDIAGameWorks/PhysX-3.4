/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __PARTICLE_IOS_ACTOR_CPU_H__
#define __PARTICLE_IOS_ACTOR_CPU_H__

#include "Apex.h"

#include "ParticleIosActorImpl.h"
#include "ParticleIosAssetImpl.h"
#include "InstancedObjectSimulationIntl.h"
#include "ParticleIosScene.h"
#include "ApexActor.h"
#include "ApexContext.h"
#include "ApexFIFO.h"

#include "PxTask.h"

namespace nvidia
{

namespace iofx
{
class RenderVolume;
class IofxAsset;
}

namespace pxparticleios
{

class ParticleIosActorCPU : public ParticleIosActorImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ParticleIosActorCPU(ResourceList&, ParticleIosAssetImpl&, ParticleIosScene&, IofxAsset&);
	~ParticleIosActorCPU();

	virtual PxTaskID				submitTasks(PxTaskManager* tm);
	virtual void						setTaskDependencies(PxTaskID taskStartAfterID, PxTaskID taskFinishBeforeID);

private:
	/* Internal utility functions */
	void								simulateParticles();

	static const uint32_t HISTOGRAM_BIN_COUNT = 1024;
	uint32_t						computeHistogram(uint32_t dataCount, float dataMin, float dataMax, uint32_t& bound);

	/* particle data (output to the IOFX actors, and some state) */

	struct NewParticleData
	{
		uint32_t  destIndex;
		PxVec3 position;
		PxVec3 velocity;
	};
	physx::Array<uint32_t>			mNewIndices;
	physx::Array<uint32_t>			mRemovedParticleList;
	physx::Array<NewParticleData>		mAddedParticleList;
	physx::PxParticleExt::IndexPool*			mIndexPool;

	/* Field sampler update velocity */
	physx::Array<uint32_t>			mUpdateIndexBuffer;
	physx::Array<PxVec3>			mUpdateVelocityBuffer;

	class SimulateTask : public PxTask
	{
	public:
		SimulateTask(ParticleIosActorCPU& actor) : mActor(actor) {}

		const char* getName() const
		{
			return "ParticleIosActorCPU::SimulateTask";
		}
		void run()
		{
			mActor.simulateParticles();
		}

	protected:
		ParticleIosActorCPU& mActor;

	private:
		SimulateTask& operator=(const SimulateTask&);
	};
	SimulateTask						mSimulateTask;

	nvidia::apex::ApexCpuInplaceStorage	mSimulationStorage;

	friend class ParticleIosAssetImpl;
};

}
} // namespace nvidia

#endif // __PARTICLE_IOS_ACTOR_CPU_H__
