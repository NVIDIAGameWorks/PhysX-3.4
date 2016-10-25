/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_IOS_ACTOR_CPU_H__
#define __BASIC_IOS_ACTOR_CPU_H__

#include "Apex.h"

#if ENABLE_TEST
#include "BasicIosTestActor.h"
#endif
#include "BasicIosActorImpl.h"
#include "BasicIosAssetImpl.h"
#include "InstancedObjectSimulationIntl.h"
#include "BasicIosSceneCPU.h"
#include "ApexActor.h"
#include "ApexContext.h"
#include "ApexFIFO.h"
#include "ApexRWLockable.h"
#include "PxTask.h"

namespace nvidia
{
namespace apex
{
class RenderVolume;
}
namespace basicios
{

#if ENABLE_TEST
#define BASIC_IOS_ACTOR BasicIosTestActor
#else
#define BASIC_IOS_ACTOR BasicIosActorImpl
#endif

class BasicIosActorCPU : public BASIC_IOS_ACTOR
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	BasicIosActorCPU(ResourceList&, BasicIosAssetImpl&, BasicIosScene&, nvidia::apex::IofxAsset&);
	~BasicIosActorCPU();

	virtual void						submitTasks();
	virtual void						setTaskDependencies();
	virtual void						fetchResults();

protected:
	/* Internal utility functions */
	void								simulateParticles();

	static const uint32_t HISTOGRAM_BIN_COUNT = 1024;
	uint32_t						computeHistogram(uint32_t dataCount, float dataMin, float dataMax, uint32_t& bound);

private:
	/* particle data (output to the IOFX actors, and some state) */

	physx::Array<uint32_t>			mNewIndices;

	class SimulateTask : public PxTask
	{
	public:
		SimulateTask(BasicIosActorCPU& actor) : mActor(actor) {}

		const char* getName() const
		{
			return "BasicIosActorCPU::SimulateTask";
		}
		void run()
		{
			mActor.simulateParticles();
		}

	protected:
		BasicIosActorCPU& mActor;

	private:
		SimulateTask& operator=(const SimulateTask&);
	};
	SimulateTask						mSimulateTask;

	nvidia::apex::ApexCpuInplaceStorage	mSimulationStorage;

	friend class BasicIosAssetImpl;
};

}
} // namespace nvidia

#endif // __BASIC_IOS_ACTOR_CPU_H__
