/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __PARTICLE_IOS_ACTOR_GPU_H__
#define __PARTICLE_IOS_ACTOR_GPU_H__

#include "Apex.h"

#include "ParticleIosActorImpl.h"
#include "ParticleIosAssetImpl.h"
#include "InstancedObjectSimulationIntl.h"
#include "ParticleIosScene.h"
#include "ApexActor.h"
#include "ApexContext.h"
#include "ApexFIFO.h"
#include "FieldSamplerQueryIntl.h"

#include "PxGpuTask.h"

namespace nvidia
{

namespace iofx
{
class IofxActor;
class RenderVolume;
}
	
namespace pxparticleios
{

class ParticleIosActorGPU;

class ParticleIosActorGPU : public ParticleIosActorImpl
{
public:
	ParticleIosActorGPU(ResourceList&, ParticleIosAssetImpl&, ParticleIosScene&, IofxAsset&);
	~ParticleIosActorGPU();

	virtual PxTaskID				submitTasks(PxTaskManager* tm);
	virtual void						setTaskDependencies(PxTaskID taskStartAfterID, PxTaskID taskFinishBeforeID);
	virtual void						fetchResults();

private:
	bool								launch(CUstream stream, int kernelIndex);
	void								trigger();

#if APEX_CUDA_SUPPORT
	CUevent								mCuSyncEvent;
	PxGpuCopyDescQueue			mCopyQueue;
#endif

	ApexMirroredArray<uint32_t>		mHoleScanSum;
	ApexMirroredArray<uint32_t>		mMoveIndices;

	ApexMirroredArray<uint32_t>		mTmpReduce;
	ApexMirroredArray<uint32_t>		mTmpHistogram;
	ApexMirroredArray<uint32_t>		mTmpScan;
	ApexMirroredArray<uint32_t>		mTmpScan1;

	ApexMirroredArray<uint32_t>		mTmpOutput;	// 0:STATUS_LASTACTIVECOUNT, ...
	ApexMirroredArray<uint32_t>		mTmpBoundParams;	// min, max

	class LaunchTask : public PxGpuTask
	{
	public:
		LaunchTask(ParticleIosActorGPU& actor) : mActor(actor) {}
		const char*	getName() const
		{
			return "ParticleIosActorGPU::LaunchTask";
		}
		void		run()
		{
			PX_ALWAYS_ASSERT();
		}
		bool		launchInstance(CUstream stream, int kernelIndex)
		{
			return mActor.launch(stream, kernelIndex);
		}
		PxGpuTaskHint::Enum getTaskHint() const
		{
			return PxGpuTaskHint::Kernel;
		}

	protected:
		ParticleIosActorGPU& mActor;

	private:
		LaunchTask& operator=(const LaunchTask&);
	};
	class TriggerTask : public PxTask
	{
	public:
		TriggerTask(ParticleIosActorGPU& actor) : mActor(actor) {}

		const char* getName() const
		{
			return "ParticleIosActorGPU::TriggerTask";
		}
		void run()
		{
			mActor.trigger();
		}

	protected:
		ParticleIosActorGPU& mActor;

	private:
		TriggerTask& operator=(const TriggerTask&);
	};


	static PX_CUDA_CALLABLE PX_INLINE PxMat44 inverse(const PxMat44& in);
	static float distance(PxVec4 a, PxVec4 b);

	LaunchTask							mLaunchTask;
	TriggerTask							mTriggerTask;
};

}
} // namespace nvidia

#endif // __PARTICLE_IOS_ACTOR_GPU_H__
