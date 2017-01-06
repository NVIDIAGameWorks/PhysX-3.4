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

#ifndef PT_BATCHER_H
#define PT_BATCHER_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "CmTask.h"

namespace physx
{

namespace Pt
{

class Batcher : public Ps::UserAllocated
{
  public:
	Batcher(class Context& _context);

	/**
	Issues shape update stages for a batch of particle systems.
	Ownership of Pt::ParticleShapeUpdateInput::shapes passed to callee!
	*/
	physx::PxBaseTask& scheduleShapeGeneration(class ParticleSystemSim** particleSystems,
	                                           struct ParticleShapesUpdateInput* inputs, PxU32 batchSize,
	                                           physx::PxBaseTask& continuation);

	/**
	Issues dynamics (SPH) update on CPUs.
	*/
	physx::PxBaseTask& scheduleDynamicsCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                       physx::PxBaseTask& continuation);

	/**
	Schedules collision prep work.
	*/
	physx::PxBaseTask& scheduleCollisionPrep(class ParticleSystemSim** particleSystems,
	                                         physx::PxLightCpuTask** inputPrepTasks, PxU32 batchSize,
	                                         physx::PxBaseTask& continuation);

	/**
	Schedules collision update stages for a batch of particle systems on CPU.
	Ownership of Pt::ParticleCollisionUpdateInput::contactManagerStream passed to callee!
	*/
	physx::PxBaseTask& scheduleCollisionCpu(class ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                        physx::PxBaseTask& continuation);

	/**
	Schedule gpu pipeline.
	*/
	physx::PxBaseTask& schedulePipelineGpu(ParticleSystemSim** particleSystems, PxU32 batchSize,
	                                       physx::PxBaseTask& continuation);

	Cm::FanoutTask shapeGenTask;
	Cm::FanoutTask dynamicsCpuTask;
	Cm::FanoutTask collPrepTask;
	Cm::FanoutTask collisionCpuTask;

	class Context& context;

  private:
	Batcher(const Batcher&);
	Batcher& operator=(const Batcher&);
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_BATCHER_H
