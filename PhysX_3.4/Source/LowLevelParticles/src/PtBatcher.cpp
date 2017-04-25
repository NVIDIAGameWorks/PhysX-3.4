// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "PtBatcher.h"
#if PX_USE_PARTICLE_SYSTEM_API

#if PX_SUPPORT_GPU_PHYSX
#include "PxPhysXGpu.h"
#endif

#include "task/PxTask.h"
#include "PtContext.h"
#include "PtParticleSystemSim.h"
#include "PtParticleSystemSimCpu.h"

using namespace physx;
using namespace Pt;

namespace
{
template <class T>
static void sortBatchedInputs(ParticleSystemSim** particleSystems, T* inputs, PxU32 batchSize, PxU32& cpuOffset,
                              PxU32& cpuCount, PxU32& gpuOffset, PxU32& gpuCount)
{
	PX_UNUSED(particleSystems);
	PX_UNUSED(inputs);

	cpuOffset = 0;
	gpuOffset = 0;

	// in place sort of both arrays
	PxU32 i = 0;
	PxU32 j = 0;

	while((i < batchSize) && (j < batchSize))
	{
#if PX_SUPPORT_GPU_PHYSX
		if(particleSystems[i]->isGpuV())
		{
			j = i + 1;
			while(j < batchSize && particleSystems[j]->isGpuV())
			{
				j++;
			}

			if(j < batchSize)
			{
				Ps::swap(particleSystems[i], particleSystems[j]);
				if(inputs)
				{
					Ps::swap(inputs[i], inputs[j]);
				}
				i++;
			}
		}
		else
#endif
		{
			i++;
		}
	}

	gpuOffset = i;
	cpuCount = gpuOffset;
	gpuCount = batchSize - cpuCount;
}
}

Batcher::Batcher(class Context& _context)
: shapeGenTask(0, "Pt::Batcher::shapeGen")
, dynamicsCpuTask(0, "Pt::Batcher::dynamicsCpu")
, collPrepTask(0, "Pt::Batcher::collPrep")
, collisionCpuTask(0, "Pt::Batcher::collisionCpu")
, context(_context)
{
}

PxBaseTask& Batcher::scheduleShapeGeneration(ParticleSystemSim** particleSystems, ParticleShapesUpdateInput* inputs,
                                             PxU32 batchSize, PxBaseTask& continuation)
{
	PxU32 cpuOffset = 0;
	PxU32 cpuCount = batchSize;

#if PX_SUPPORT_GPU_PHYSX
	PxU32 gpuOffset, gpuCount;
	sortBatchedInputs(particleSystems, inputs, batchSize, cpuOffset, cpuCount, gpuOffset, gpuCount);
	if(context.getSceneGpuFast() && gpuCount > 0)
	{
		PxBaseTask& task = context.getSceneGpuFast()->scheduleParticleShapeUpdate(
		    particleSystems + gpuOffset, inputs + gpuOffset, gpuCount, continuation);
		shapeGenTask.addDependent(task);
		task.removeReference();
	}
#endif
	for(PxU32 i = cpuOffset; i < (cpuOffset + cpuCount); ++i)
	{
		PxBaseTask& task =
		    static_cast<ParticleSystemSimCpu*>(particleSystems[i])->schedulePacketShapesUpdate(inputs[i], continuation);
		shapeGenTask.addDependent(task);
		task.removeReference();
	}

	if(shapeGenTask.getReference() == 0)
	{
		continuation.addReference();
		return continuation;
	}

	while(shapeGenTask.getReference() > 1)
		shapeGenTask.removeReference();

	return shapeGenTask;
}

PxBaseTask& Batcher::scheduleDynamicsCpu(ParticleSystemSim** particleSystems, PxU32 batchSize, PxBaseTask& continuation)
{
	PxU32 cpuOffset = 0;
	PxU32 cpuCount = batchSize;
#if PX_SUPPORT_GPU_PHYSX
	PxU32 gpuOffset, gpuCount;
	sortBatchedInputs(particleSystems, (PxU8*)NULL, batchSize, cpuOffset, cpuCount, gpuOffset, gpuCount);
#endif
	for(PxU32 i = cpuOffset; i < (cpuOffset + cpuCount); ++i)
	{
		PxBaseTask& task = static_cast<ParticleSystemSimCpu*>(particleSystems[i])->scheduleDynamicsUpdate(continuation);
		dynamicsCpuTask.addDependent(task);
		task.removeReference();
	}

	if(dynamicsCpuTask.getReference() == 0)
	{
		continuation.addReference();
		return continuation;
	}

	while(dynamicsCpuTask.getReference() > 1)
		dynamicsCpuTask.removeReference();

	return dynamicsCpuTask;
}

PxBaseTask& Batcher::scheduleCollisionPrep(ParticleSystemSim** particleSystems, PxLightCpuTask** inputPrepTasks,
                                           PxU32 batchSize, PxBaseTask& continuation)
{
	PxU32 cpuOffset = 0;
	PxU32 cpuCount = batchSize;
#if PX_SUPPORT_GPU_PHYSX
	PxU32 gpuOffset, gpuCount;
	sortBatchedInputs(particleSystems, inputPrepTasks, batchSize, cpuOffset, cpuCount, gpuOffset, gpuCount);
	if(context.getSceneGpuFast() && gpuCount > 0)
	{
		PxBaseTask& gpuCollisionInputTask = context.getSceneGpuFast()->scheduleParticleCollisionInputUpdate(
		    particleSystems + gpuOffset, gpuCount, continuation);
		for(PxU32 i = gpuOffset; i < (gpuOffset + gpuCount); ++i)
		{
			inputPrepTasks[i]->setContinuation(&gpuCollisionInputTask);
			collPrepTask.addDependent(*inputPrepTasks[i]);
			inputPrepTasks[i]->removeReference();
		}
		gpuCollisionInputTask.removeReference();
	}
#else
	PX_UNUSED(particleSystems);
	PX_UNUSED(batchSize);
#endif
	for(PxU32 i = cpuOffset; i < (cpuOffset + cpuCount); ++i)
	{
		inputPrepTasks[i]->setContinuation(&continuation);
		collPrepTask.addDependent(*inputPrepTasks[i]);
		inputPrepTasks[i]->removeReference();
	}

	if(collPrepTask.getReference() == 0)
	{
		continuation.addReference();
		return continuation;
	}

	while(collPrepTask.getReference() > 1)
		collPrepTask.removeReference();

	return collPrepTask;
}

PxBaseTask& Batcher::scheduleCollisionCpu(ParticleSystemSim** particleSystems, PxU32 batchSize, PxBaseTask& continuation)
{
	PxU32 cpuOffset = 0;
	PxU32 cpuCount = batchSize;
#if PX_SUPPORT_GPU_PHYSX
	PxU32 gpuOffset, gpuCount;
	sortBatchedInputs(particleSystems, (PxU8*)NULL, batchSize, cpuOffset, cpuCount, gpuOffset, gpuCount);
#endif
	for(PxU32 i = cpuOffset; i < (cpuOffset + cpuCount); ++i)
	{
		PxBaseTask& task = static_cast<ParticleSystemSimCpu*>(particleSystems[i])->scheduleCollisionUpdate(continuation);
		collisionCpuTask.addDependent(task);
		task.removeReference();
	}

	if(collisionCpuTask.getReference() == 0)
	{
		continuation.addReference();
		return continuation;
	}

	while(collisionCpuTask.getReference() > 1)
		collisionCpuTask.removeReference();

	return collisionCpuTask;
}

PxBaseTask& Batcher::schedulePipelineGpu(ParticleSystemSim** particleSystems, PxU32 batchSize, PxBaseTask& continuation)
{
#if PX_SUPPORT_GPU_PHYSX
	PxU32 cpuOffset, cpuCount, gpuOffset, gpuCount;
	sortBatchedInputs(particleSystems, (PxU8*)NULL, batchSize, cpuOffset, cpuCount, gpuOffset, gpuCount);
	if(context.getSceneGpuFast() && gpuCount > 0)
	{
		return context.getSceneGpuFast()->scheduleParticlePipeline(particleSystems + gpuOffset, gpuCount, continuation);
	}
#else
	PX_UNUSED(batchSize);
	PX_UNUSED(particleSystems);
#endif
	continuation.addReference();
	return continuation;
}

#endif // PX_USE_PARTICLE_SYSTEM_API
