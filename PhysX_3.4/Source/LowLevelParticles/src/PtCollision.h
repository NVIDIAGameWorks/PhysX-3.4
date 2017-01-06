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

#ifndef PT_COLLISION_H
#define PT_COLLISION_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "foundation/PxTransform.h"
#include "PsBitUtils.h"
#include "PtConfig.h"
#include "PtCollisionData.h"
#include "PtCollisionMethods.h"
#include "PtParticle.h"
#include "PtTwoWayData.h"
#include "PtCollisionParameters.h"
#include "PsAlignedMalloc.h"
#include "CmTask.h"
#include "PtParticleContactManagerStream.h"

namespace physx
{

class PxsRigidBody;
class PxBaseTask;

namespace Pt
{

class ParticleShape;
class BodyTransformVault;
struct W2STransformTemp;

class Collision
{
  public:
	Collision(class ParticleSystemSimCpu& particleSystem);
	~Collision();

	void updateCollision(const PxU8* contactManagerStream, physx::PxBaseTask& continuation);

	// Update position and velocity of particles that have PxParticleFlag::eSPATIAL_DATA_STRUCTURE_OVERFLOW set.
	void updateOverflowParticles();

	PX_FORCE_INLINE CollisionParameters& getParameter()
	{
		return mParams;
	}

  private:
	typedef Ps::Array<W2STransformTemp, shdfnd::AlignedAllocator<16, Ps::ReflectionAllocator<W2STransformTemp> > >
	TempContactManagerArray;
	struct TaskData
	{
		TempContactManagerArray tempContactManagers;
		ParticleContactManagerStreamIterator packetBegin;
		ParticleContactManagerStreamIterator packetEnd;
		PxBounds3 bounds;
	};

	void processShapeListWithFilter(PxU32 taskDataIndex, const PxU32 skipNum = 0);
	void mergeResults(physx::PxBaseTask* continuation);

	void updateFluidShapeCollision(Particle* particles, TwoWayData* fluidTwoWayData, PxVec3* transientBuf,
	                               PxVec3* collisionVelocities, ConstraintBuffers& constraintBufs,
	                               ParticleOpcodeCache* opcodeCache, PxBounds3& worldBounds,
	                               const PxU32* fluidShapeParticleIndices, const PxF32* restOffsets,
	                               const W2STransformTemp* w2sTransforms, const ParticleStreamShape& streamShape);

	PX_FORCE_INLINE void updateSubPacket(Particle* particlesSp, TwoWayData* fluidTwoWayData, PxVec3* transientBuf,
	                                     PxVec3* collisionVelocities, ConstraintBuffers& constraintBufs,
	                                     ParticleOpcodeCache* perParticleCacheLocal,
	                                     ParticleOpcodeCache* perParticleCacheGlobal, LocalCellHash& localCellHash,
	                                     PxBounds3& worldBounds, const PxVec3& packetCorner,
	                                     const PxU32* particleIndicesSp, const PxU32 numParticlesSp,
	                                     const ParticleStreamContactManager* contactManagers,
	                                     const W2STransformTemp* w2sTransforms, const PxU32 numContactManagers,
	                                     const PxF32* restOffsetsSp);

	void updateFluidBodyContactPair(const Particle* particles, PxU32 numParticles, ParticleCollData* particleCollData,
	                                ConstraintBuffers& constraintBufs, ParticleOpcodeCache* perParticleCacheLocal,
	                                LocalCellHash& localCellHash, const PxVec3& packetCorner,
	                                const ParticleStreamContactManager& contactManager,
	                                const W2STransformTemp& w2sTransform);

	void PX_FORCE_INLINE addTempW2STransform(TaskData& taskData, const ParticleStreamContactManager& cm);

  private:
	Collision& operator=(const Collision&);
	CollisionParameters mParams;
	ParticleSystemSimCpu& mParticleSystem;
	TaskData mTaskData[PT_NUM_PACKETS_PARALLEL_COLLISION];

	typedef Cm::DelegateTask<Collision, &Collision::mergeResults> MergeTask;
	MergeTask mMergeTask;
	friend class CollisionTask;
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_COLLISION_H
