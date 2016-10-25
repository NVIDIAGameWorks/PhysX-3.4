/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_IOS_ACTOR_GPU_H__
#define __BASIC_IOS_ACTOR_GPU_H__

#include "Apex.h"

#if ENABLE_TEST
#include "BasicIosTestActor.h"
#endif
#include "BasicIosActorImpl.h"
#include "BasicIosAssetImpl.h"
#include "InstancedObjectSimulationIntl.h"
#include "BasicIosSceneGPU.h"
#include "ApexActor.h"
#include "ApexContext.h"
#include "ApexFIFO.h"
#include "FieldSamplerQueryIntl.h"

#include "PxGpuTask.h"

namespace nvidia
{
namespace IOFX
{
class IofxActor;
class RenderVolume;
}

namespace basicios
{

#if ENABLE_TEST
#define BASIC_IOS_ACTOR BasicIosTestActor
#else
#define BASIC_IOS_ACTOR BasicIosActorImpl
#endif

class BasicIosActorGPU : public BASIC_IOS_ACTOR
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	BasicIosActorGPU(ResourceList&, BasicIosAssetImpl&, BasicIosScene&, nvidia::apex::IofxAsset&, const ApexMirroredPlace::Enum defaultPlace = ApexMirroredPlace::GPU);
	~BasicIosActorGPU();

	virtual void						submitTasks();
	virtual void						setTaskDependencies();
	virtual void						fetchResults();

protected:
	bool								launch(CUstream stream, int kernelIndex);

	PxGpuCopyDescQueue		mCopyQueue;

	ApexMirroredArray<uint32_t>		mHoleScanSum;
	ApexMirroredArray<uint32_t>		mMoveIndices;

	ApexMirroredArray<uint32_t>		mTmpReduce;
	ApexMirroredArray<uint32_t>		mTmpHistogram;
	ApexMirroredArray<uint32_t>		mTmpScan;
	ApexMirroredArray<uint32_t>		mTmpScan1;

	ApexMirroredArray<uint32_t>		mTmpOutput;
	ApexMirroredArray<uint32_t>		mTmpOutput1;

	class LaunchTask : public PxGpuTask
	{
	public:
		LaunchTask(BasicIosActorGPU& actor) : mActor(actor) {}
		const char* getName() const
		{
			return "BasicIosActorGPU::LaunchTask";
		}
		void         run()
		{
			PX_ALWAYS_ASSERT();
		}
		bool         launchInstance(CUstream stream, int kernelIndex)
		{
			return mActor.launch(stream, kernelIndex);
		}
		PxGpuTaskHint::Enum getTaskHint() const
		{
			return PxGpuTaskHint::Kernel;
		}

	protected:
		BasicIosActorGPU& mActor;

	private:
		LaunchTask& operator=(const LaunchTask&);
	};

	static PX_CUDA_CALLABLE PX_INLINE PxMat44 inverse(const PxMat44& in);
	static float distance(PxVec4 a, PxVec4 b);

	LaunchTask							mLaunchTask;
};

}
} // namespace nvidia

#endif // __BASIC_IOS_ACTOR_GPU_H__
