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

#ifndef PT_CONTEXT_CPU_H
#define PT_CONTEXT_CPU_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "CmPool.h"
#include "PtContext.h"

namespace physx
{

class PxBaseTask;
class PxLightCpuTask;
class PxTaskManager;

namespace Pt
{

class Batcher;
class BodyTransformVault;
class ParticleShapeCpu;
class ParticleSystemSimCpu;
struct ParticleCell;

/**
Per scene manager class for particle systems.
*/
class ContextCpu : public Context, Ps::UserAllocated
{
	PX_NOCOPY(ContextCpu)
  public:
	/**
	Register particle functionality.
	Not calling this should allow the code to be stripped at link time.
	*/
	static void registerParticles();

	// Pt::Context implementation
	virtual void destroy();
	virtual ParticleSystemSim* addParticleSystem(class ParticleData* particleData,
	                                             const ParticleSystemParameter& parameter, bool useGpuSupport);
	virtual ParticleData* removeParticleSystem(ParticleSystemSim* system, bool acquireParticleData);
	virtual PxBaseTask& scheduleShapeGeneration(class ParticleSystemSim** particleSystems,
	                                            struct ParticleShapesUpdateInput* inputs, PxU32 batchSize,
	                                            PxBaseTask& continuation);
	virtual PxBaseTask& scheduleDynamicsCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                        PxBaseTask& continuation);
	virtual PxBaseTask& scheduleCollisionPrep(class ParticleSystemSim** particleSystems, PxLightCpuTask** inputPrepTasks,
	                                          PxU32 batchSize, PxBaseTask& continuation);
	virtual PxBaseTask& scheduleCollisionCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                         PxBaseTask& continuation);
	virtual PxBaseTask& schedulePipelineGpu(ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                        PxBaseTask& continuation);
#if PX_SUPPORT_GPU_PHYSX
	virtual class PxSceneGpu* createOrGetSceneGpu();
#endif
	//~Pt::Context implementation

	ParticleShapeCpu* createParticleShape(ParticleSystemSimCpu* particleSystem, const ParticleCell* packet);
	void releaseParticleShape(ParticleShapeCpu* shape);

	Cm::FlushPool& getTaskPool()
	{
		return mTaskPool;
	}

  private:
	ContextCpu(physx::PxTaskManager* taskManager, Cm::FlushPool& taskPool);

	virtual ~ContextCpu();

	ParticleSystemSim* addParticleSystemImpl(ParticleData* particleData, const ParticleSystemParameter& parameter,
	                                         bool useGpuSupport);
	ParticleData* removeParticleSystemImpl(ParticleSystemSim* system, bool acquireParticleData);

	static Context* createContextImpl(physx::PxTaskManager* taskManager, Cm::FlushPool& taskPool);

	void destroyContextImpl();

	Cm::PoolList<ParticleSystemSimCpu, ContextCpu> mParticleSystemPool;
	Cm::PoolList<ParticleShapeCpu, ContextCpu> mParticleShapePool;
	Ps::Mutex mParticleShapePoolMutex;
	Batcher* mBatcher;

	physx::PxTaskManager* mTaskManager;
	Cm::FlushPool& mTaskPool;

#if PX_SUPPORT_GPU_PHYSX
	class RigidBodyAccessGpu* mGpuRigidBodyAccess;
#endif
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_CONTEXT_CPU_H
