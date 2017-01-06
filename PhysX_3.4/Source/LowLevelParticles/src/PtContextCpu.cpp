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

#include "PtContextCpu.h"
#if PX_USE_PARTICLE_SYSTEM_API

#if PX_SUPPORT_GPU_PHYSX
#include "task/PxGpuDispatcher.h"
#include "PxvGlobals.h"
#include "PxPhysXGpu.h"
#include "PxSceneGpu.h"
#include "gpu/PtRigidBodyAccessGpu.h"
#endif

#include "foundation/PxFoundation.h"
#include "PtParticleData.h"
#include "PtParticleSystemSimCpu.h"
#include "PtParticleShapeCpu.h"
#include "PtBatcher.h"
#include "PtBodyTransformVault.h"
#include "PsFoundation.h"

using namespace physx::shdfnd;
using namespace physx;
using namespace Pt;

namespace
{
ParticleSystemSim* (ContextCpu::*addParticleSystemFn)(ParticleData*, const ParticleSystemParameter&, bool);
ParticleData* (ContextCpu::*removeParticleSystemFn)(ParticleSystemSim*, bool);
Context* (*createContextFn)(physx::PxTaskManager*, Cm::FlushPool&);
void (ContextCpu::*destroyContextFn)();

PxBaseTask& (Batcher::*scheduleShapeGenerationFn)(ParticleSystemSim** particleSystems, ParticleShapesUpdateInput* inputs,
                                                  PxU32 batchSize, PxBaseTask& continuation) = 0;
PxBaseTask& (Batcher::*scheduleDynamicsCpuFn)(ParticleSystemSim** particleSystems, PxU32 batchSize,
                                              PxBaseTask& continuation) = 0;
PxBaseTask& (Batcher::*scheduleCollisionPrepFn)(ParticleSystemSim** particleSystems, PxLightCpuTask** inputPrepTasks,
                                                PxU32 batchSize, PxBaseTask& continuation) = 0;
PxBaseTask& (Batcher::*scheduleCollisionCpuFn)(ParticleSystemSim** particleSystems, PxU32 batchSize,
                                               PxBaseTask& continuation) = 0;
PxBaseTask& (Batcher::*schedulePipelineGpuFn)(ParticleSystemSim** particleSystems, PxU32 batchSize,
                                              PxBaseTask& continuation) = 0;
}

namespace physx
{
namespace Pt
{
void registerParticles()
{
	ContextCpu::registerParticles();
}

Context* createParticleContext(class physx::PxTaskManager* taskManager, Cm::FlushPool& taskPool)
{
	if(::createContextFn)
	{
		return ::createContextFn(taskManager, taskPool);
	}
	return NULL;
}
} // namespace Pt
} // namespace physx

void ContextCpu::registerParticles()
{
	::createContextFn = &ContextCpu::createContextImpl;
	::destroyContextFn = &ContextCpu::destroyContextImpl;
	::addParticleSystemFn = &ContextCpu::addParticleSystemImpl;
	::removeParticleSystemFn = &ContextCpu::removeParticleSystemImpl;

	::scheduleShapeGenerationFn = &Batcher::scheduleShapeGeneration;
	::scheduleDynamicsCpuFn = &Batcher::scheduleDynamicsCpu;
	::scheduleCollisionPrepFn = &Batcher::scheduleCollisionPrep;
	::scheduleCollisionCpuFn = &Batcher::scheduleCollisionCpu;
	::schedulePipelineGpuFn = &Batcher::schedulePipelineGpu;
}

Context* ContextCpu::createContextImpl(PxTaskManager* taskManager, Cm::FlushPool& taskPool)
{
	return PX_NEW(ContextCpu)(taskManager, taskPool);
}

void ContextCpu::destroy()
{
	(this->*destroyContextFn)();
}

void ContextCpu::destroyContextImpl()
{
	PX_DELETE(this);
}

ParticleSystemSim* ContextCpu::addParticleSystem(ParticleData* particleData, const ParticleSystemParameter& parameter,
                                                 bool useGpuSupport)
{
	return (this->*addParticleSystemFn)(particleData, parameter, useGpuSupport);
}

ParticleData* ContextCpu::removeParticleSystem(ParticleSystemSim* particleSystem, bool acquireParticleData)
{
	return (this->*removeParticleSystemFn)(particleSystem, acquireParticleData);
}

ContextCpu::ContextCpu(PxTaskManager* taskManager, Cm::FlushPool& taskPool)
: mParticleSystemPool("mParticleSystemPool", this, 16, 1024)
, mParticleShapePool("mParticleShapePool", this, 256, 1024)
, mBatcher(NULL)
, mTaskManager(taskManager)
, mTaskPool(taskPool)
#if PX_SUPPORT_GPU_PHYSX
, mGpuRigidBodyAccess(NULL)
#endif
{
	mBatcher = PX_NEW(Batcher)(*this);
	mBodyTransformVault = PX_NEW(BodyTransformVault);
	mSceneGpu = NULL;
}

ContextCpu::~ContextCpu()
{
#if PX_SUPPORT_GPU_PHYSX
	if(mSceneGpu)
	{
		mSceneGpu->release();
	}

	if(mGpuRigidBodyAccess)
	{
		PX_DELETE(mGpuRigidBodyAccess);
	}
#endif

	PX_DELETE(mBatcher);
	PX_DELETE(mBodyTransformVault);
}

ParticleSystemSim* ContextCpu::addParticleSystemImpl(ParticleData* particleData,
                                                     const ParticleSystemParameter& parameter, bool useGpuSupport)
{
	PX_ASSERT(particleData);

#if PX_SUPPORT_GPU_PHYSX
	if(useGpuSupport)
	{
		PxSceneGpu* sceneGPU = createOrGetSceneGpu();
		if(sceneGPU)
		{
			ParticleSystemStateDataDesc particles;
			particleData->getParticlesV(particles, true, false);
			ParticleSystemSim* sim = sceneGPU->addParticleSystem(particles, parameter);

			if(sim)
			{
				particleData->release();
				return sim;
			}
		}
		return NULL;
	}
	else
	{
		ParticleSystemSimCpu* sim = mParticleSystemPool.get();
		sim->init(*particleData, parameter);
		return sim;
	}
#else
	PX_UNUSED(useGpuSupport);
	ParticleSystemSimCpu* sim = mParticleSystemPool.get();
	sim->init(*particleData, parameter);
	return sim;
#endif
}

ParticleData* ContextCpu::removeParticleSystemImpl(ParticleSystemSim* particleSystem, bool acquireParticleData)
{
	ParticleData* particleData = NULL;

#if PX_SUPPORT_GPU_PHYSX
	if(particleSystem->isGpuV())
	{
		PX_ASSERT(getSceneGpuFast());
		if(acquireParticleData)
		{
			ParticleSystemStateDataDesc particles;
			particleSystem->getParticleStateV().getParticlesV(particles, true, false);
			particleData = ParticleData::create(particles, particleSystem->getParticleStateV().getWorldBoundsV());
		}
		getSceneGpuFast()->removeParticleSystem(particleSystem);
		return particleData;
	}
#endif

	ParticleSystemSimCpu& sim = *static_cast<ParticleSystemSimCpu*>(particleSystem);

	if(acquireParticleData)
		particleData = sim.obtainParticleState();

	sim.clear();
	mParticleSystemPool.put(&sim);
	return particleData;
}

ParticleShapeCpu* ContextCpu::createParticleShape(ParticleSystemSimCpu* particleSystem, const ParticleCell* packet)
{
	// for now just lock the mParticleShapePool for concurrent access from different tasks
	Ps::Mutex::ScopedLock lock(mParticleShapePoolMutex);
	ParticleShapeCpu* shape = mParticleShapePool.get();

	if(shape)
		shape->init(particleSystem, packet);

	return shape;
}

void ContextCpu::releaseParticleShape(ParticleShapeCpu* shape)
{
	// for now just lock the mParticleShapePool for concurrent access from different tasks
	Ps::Mutex::ScopedLock lock(mParticleShapePoolMutex);
	mParticleShapePool.put(shape);
}

#if PX_SUPPORT_GPU_PHYSX

PxSceneGpu* ContextCpu::createOrGetSceneGpu()
{
	if(mSceneGpu)
		return mSceneGpu;

	// get PxCudaContextManager

	if(!mTaskManager || !mTaskManager->getGpuDispatcher() || !mTaskManager->getGpuDispatcher()->getCudaContextManager())
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__,
		                          "GPU operation failed. No PxCudaContextManager available.");
		return NULL;
	}
	physx::PxCudaContextManager& contextManager = *mTaskManager->getGpuDispatcher()->getCudaContextManager();

	// load PhysXGpu dll interface

	PxPhysXGpu* physXGpu = PxvGetPhysXGpu(true);
	if(!physXGpu)
	{
		getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__,
		                      "GPU operation failed. PhysXGpu dll unavailable.");
		return NULL;
	}

	// create PxsGpuRigidBodyAccess

	PX_ASSERT(!mGpuRigidBodyAccess);
	mGpuRigidBodyAccess = PX_NEW(RigidBodyAccessGpu)(*mBodyTransformVault);

	// finally create PxSceneGpu
	mSceneGpu = physXGpu->createScene(contextManager, *mGpuRigidBodyAccess);
	if(!mSceneGpu)
	{
		PX_DELETE_AND_RESET(mGpuRigidBodyAccess);
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__,
		                          "GPU operation failed. PxSceneGpu creation unsuccessful.");
	}

	return mSceneGpu;
}
#endif // PX_SUPPORT_GPU_PHYSX

PxBaseTask& ContextCpu::scheduleShapeGeneration(class ParticleSystemSim** particleSystems,
                                                struct ParticleShapesUpdateInput* inputs, PxU32 batchSize,
                                                PxBaseTask& continuation)
{
	return (mBatcher->*::scheduleShapeGenerationFn)(particleSystems, inputs, batchSize, continuation);
}

PxBaseTask& ContextCpu::scheduleDynamicsCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
                                            PxBaseTask& continuation)
{
	return (mBatcher->*::scheduleDynamicsCpuFn)(particleSystems, batchSize, continuation);
}

PxBaseTask& ContextCpu::scheduleCollisionPrep(class ParticleSystemSim** particleSystems,
                                              PxLightCpuTask** inputPrepTasks, PxU32 batchSize, PxBaseTask& continuation)
{
	return (mBatcher->*::scheduleCollisionPrepFn)(particleSystems, inputPrepTasks, batchSize, continuation);
}

PxBaseTask& ContextCpu::scheduleCollisionCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
                                             PxBaseTask& continuation)
{
	return (mBatcher->*::scheduleCollisionCpuFn)(particleSystems, batchSize, continuation);
}

PxBaseTask& ContextCpu::schedulePipelineGpu(ParticleSystemSim** particleSystems, PxU32 batchSize, PxBaseTask& continuation)
{
	return (mBatcher->*::schedulePipelineGpuFn)(particleSystems, batchSize, continuation);
}

#endif // PX_USE_PARTICLE_SYSTEM_API
