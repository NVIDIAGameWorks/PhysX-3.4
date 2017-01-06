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

#ifndef PT_CONTEXT_H
#define PT_CONTEXT_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "PtParticleSystemSim.h"
#include "PtParticleShape.h"
#include "PtBodyTransformVault.h"

namespace physx
{

class PxSceneGpu;
class PxBaseTask;
class PxLightCpuTask;
class PxTaskManager;

namespace Cm
{
class FlushPool;
}

namespace Pt
{

class BodyTransformVault;

/**
Per scene manager class for particle systems. All particle systems part of a context are stepped collectively.

The class represents a common interface for CPU and GPU particles. Currently the Pt::ContextCpu implementation
also contains code to create and schedule GPU particle systems. Eventually all that logic should move either to a
Pt::ContextGPU class or PxSceneGpu should be changed to implement the Content interface directly.
*/
class Context
{
	PX_NOCOPY(Context)
  public:
	/**
	Deallocates all resources created for the particle context instance.
	*/
	virtual void destroy() = 0;

	/**
	Add a particle system to the particle low level context.
	Onwership of ParticleData is transfered to the low level particle system in case of the
	CPU implementation. For the GPU implementation, the content is copied and the ParticleData released.
	If GPU creation fails, NULL is returned.
	*/
	virtual ParticleSystemSim* addParticleSystem(class ParticleData* particleData,
	                                             const ParticleSystemParameter& parameter, bool useGpuSupport) = 0;

	/**
	Removes a particle system from the particle low level context. If acquireParticleData is specified, the particle
	state is returned. In case of the CPU implementation, ParticleData ownership is returned - in case of the GPU
	implementation a new ParticleData instance is created and the particle state copied to it.
	*/
	virtual ParticleData* removeParticleSystem(ParticleSystemSim* system, bool acquireParticleData) = 0;

	/**
	Issues shape update stages for a batch of particle systems.
	Ownership of Pt::ParticleShapeUpdateInput::shapes passed to callee!
	*/
	virtual PxBaseTask& scheduleShapeGeneration(class ParticleSystemSim** particleSystems,
	                                            struct ParticleShapesUpdateInput* inputs, PxU32 batchSize,
	                                            PxBaseTask& continuation) = 0;

	/**
	Issues dynamics (SPH) update on CPUs.
	*/
	virtual physx::PxBaseTask& scheduleDynamicsCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                               physx::PxBaseTask& continuation) = 0;

	/**
	Schedules collision prep work.
	*/
	virtual physx::PxBaseTask& scheduleCollisionPrep(class ParticleSystemSim** particleSystems,
	                                                 physx::PxLightCpuTask** inputPrepTasks, PxU32 batchSize,
	                                                 physx::PxBaseTask& continuation) = 0;

	/**
	Schedules collision update stages for a batch of particle systems on CPU.
	Ownership of Pt::ParticleCollisionUpdateInput::contactManagerStream passed to callee!
	*/
	virtual physx::PxBaseTask& scheduleCollisionCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                                physx::PxBaseTask& continuation) = 0;

	/**
	Schedule gpu pipeline.
	*/
	virtual physx::PxBaseTask& schedulePipelineGpu(ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                               physx::PxBaseTask& continuation) = 0;

#if PX_SUPPORT_GPU_PHYSX
	/**
	Lazily creates a GPU scene (context)
	and returns it.
	*/
	virtual class PxSceneGpu* createOrGetSceneGpu() = 0;
#endif

#if PX_SUPPORT_GPU_PHYSX
	/**
	Returns the GPU scene (context).
	Non-virtual implementation.
	*/
	PX_FORCE_INLINE PxSceneGpu* getSceneGpuFast()
	{
		return mSceneGpu;
	}
#endif

	/**
	Returns Body hash data structure, which is used to store previous pose for RBs for particle collision.
	*/
	PX_FORCE_INLINE BodyTransformVault& getBodyTransformVaultFast()
	{
		return *mBodyTransformVault;
	}

  protected:
	Context() : mBodyTransformVault(NULL), mSceneGpu(NULL)
	{
	}
	virtual ~Context()
	{
	}

	// members for 'fast', non virtual access
	BodyTransformVault* mBodyTransformVault; // Hash table to store last frames world to body transforms.
	PxSceneGpu* mSceneGpu;
};

/**
Creates a particle system context
*/
Context* createParticleContext(physx::PxTaskManager* taskManager, Cm::FlushPool& taskPool);

/**
Register particle functionality.
Not calling this should allow the code to be stripped at link time.
*/
void registerParticles();

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_CONTEXT_H
