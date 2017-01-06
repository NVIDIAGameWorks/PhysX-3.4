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

#ifndef PT_PARTICLE_SYSTEM_SIM_CPU_H
#define PT_PARTICLE_SYSTEM_SIM_CPU_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "PtParticleSystemSim.h"
#include "PtDynamics.h"
#include "PtCollision.h"
#include "PtGridCellVector.h"
#include "PsAllocator.h"
#include "PtParticleData.h"
#include "CmTask.h"
#include "PtContextCpu.h"

namespace physx
{

class PxParticleDeviceExclusiveAccess;
class PxBaseTask;

namespace Pt
{
class Context;
struct ConstraintPair;
class SpatialHash;
class ParticleShapeCpu;

class ParticleSystemSimCpu : public ParticleSystemSim
{
	PX_NOCOPY(ParticleSystemSimCpu)
  public:
	//---------------------------
	// Implements ParticleSystemSim
	virtual ParticleSystemState& getParticleStateV();
	virtual void getSimParticleDataV(ParticleSystemSimDataDesc& simParticleData, bool devicePtr) const;

	virtual void getShapesUpdateV(ParticleShapeUpdateResults& updateResults) const;

	virtual void setExternalAccelerationV(const PxVec3& v);
	virtual const PxVec3& getExternalAccelerationV() const;

	virtual void setSimulationTimeStepV(PxReal value);
	virtual PxReal getSimulationTimeStepV() const;

	virtual void setSimulatedV(bool);
	virtual Ps::IntBool isSimulatedV() const;

	virtual void addInteractionV(const ParticleShape&, ShapeHandle, BodyHandle, bool, bool)
	{
	}
	virtual void removeInteractionV(const ParticleShape& particleShape, ShapeHandle shape, BodyHandle body,
	                                bool isDynamic, bool isDyingRb, bool ccdBroadphase);
	virtual void onRbShapeChangeV(const ParticleShape& particleShape, ShapeHandle shape);

	virtual void flushBufferedInteractionUpdatesV()
	{
	}

	virtual void passCollisionInputV(ParticleCollisionUpdateInput input);
#if PX_SUPPORT_GPU_PHYSX
	virtual Ps::IntBool isGpuV() const
	{
		return false;
	}
	virtual void enableDeviceExclusiveModeGpuV()
	{
		PX_ASSERT(0);
	}
	virtual PxParticleDeviceExclusiveAccess* getDeviceExclusiveAccessGpuV() const
	{
		PX_ASSERT(0);
		return NULL;
	}
#endif

	//~Implements ParticleSystemSim
	//---------------------------

	ParticleSystemSimCpu(ContextCpu* context, PxU32 index);
	virtual ~ParticleSystemSimCpu();
	void init(ParticleData& particleData, const ParticleSystemParameter& parameter);
	void clear();
	ParticleData* obtainParticleState();

	PX_FORCE_INLINE ContextCpu& getContext() const
	{
		return mContext;
	}

	PX_FORCE_INLINE void getPacketBounds(const GridCellVector& coord, PxBounds3& bounds);

	PX_FORCE_INLINE PxReal computeViscosityMultiplier(PxReal viscosityStd, PxReal particleMassStd, PxReal radius6Std);

	PX_FORCE_INLINE PxU32 getIndex() const
	{
		return mIndex;
	}

	void packetShapesUpdate(physx::PxBaseTask* continuation);
	void packetShapesFinalization(physx::PxBaseTask* continuation);
	void dynamicsUpdate(physx::PxBaseTask* continuation);
	void collisionUpdate(physx::PxBaseTask* continuation);
	void collisionFinalization(physx::PxBaseTask* continuation);
	void spatialHashUpdateSections(physx::PxBaseTask* continuation);

	physx::PxBaseTask& schedulePacketShapesUpdate(const ParticleShapesUpdateInput& input,
	                                              physx::PxBaseTask& continuation);
	physx::PxBaseTask& scheduleDynamicsUpdate(physx::PxBaseTask& continuation);
	physx::PxBaseTask& scheduleCollisionUpdate(physx::PxBaseTask& continuation);

  private:
	void remapShapesToPackets(ParticleShape* const* shapes, PxU32 numShapes);
	void clearParticleConstraints();
	void initializeParameter();
	void updateDynamicsParameter();
	void updateCollisionParameter();
	void removeTwoWayRbReferences(const ParticleShapeCpu& particleShape, const PxsBodyCore* rigidBody);
	void setCollisionCacheInvalid(const ParticleShapeCpu& particleShape, const Gu::GeometryUnion& geometry);

  private:
	ContextCpu& mContext;
	ParticleData* mParticleState;
	const ParticleSystemParameter* mParameter;

	Ps::IntBool mSimulated;

	TwoWayData* mFluidTwoWayData;

	ParticleShape** mCreatedDeletedParticleShapes; // Handles of created and deleted particle packet shapes.
	PxU32 mNumCreatedParticleShapes;
	PxU32 mNumDeletedParticleShapes;
	PxU32* mPacketParticlesIndices; // Dense array of sorted particle indices.
	PxU32 mNumPacketParticlesIndices;

	ConstraintBuffers mConstraintBuffers; // Particle constraints.

	ParticleOpcodeCache* mOpcodeCacheBuffer; // Opcode cache.
	PxVec3* mTransientBuffer;                // force in SPH , collision normal
	PxVec3* mCollisionVelocities;

	// Spatial ordering, packet generation
	SpatialHash* mSpatialHash;

	// Dynamics update
	Dynamics mDynamics;

	// Collision update
	Collision mCollision;

	PxReal mSimulationTimeStep;
	bool mIsSimulated;

	PxVec3 mExternalAcceleration; // This includes the gravity of the scene

	PxU32 mIndex;

	// pipeline tasks
	typedef Cm::DelegateTask<ParticleSystemSimCpu, &ParticleSystemSimCpu::packetShapesUpdate> PacketShapesUpdateTask;
	typedef Cm::DelegateTask<ParticleSystemSimCpu, &ParticleSystemSimCpu::packetShapesFinalization> PacketShapesFinalizationTask;
	typedef Cm::DelegateTask<ParticleSystemSimCpu, &ParticleSystemSimCpu::dynamicsUpdate> DynamicsUpdateTask;
	typedef Cm::DelegateTask<ParticleSystemSimCpu, &ParticleSystemSimCpu::collisionUpdate> CollisionUpdateTask;
	typedef Cm::DelegateTask<ParticleSystemSimCpu, &ParticleSystemSimCpu::collisionFinalization> CollisionFinalizationTask;
	typedef Cm::DelegateTask<ParticleSystemSimCpu, &ParticleSystemSimCpu::spatialHashUpdateSections> SpatialHashUpdateSectionsTask;

	PacketShapesUpdateTask mPacketShapesUpdateTask;
	PacketShapesFinalizationTask mPacketShapesFinalizationTask;
	DynamicsUpdateTask mDynamicsUpdateTask;
	CollisionUpdateTask mCollisionUpdateTask;
	CollisionFinalizationTask mCollisionFinalizationTask;
	SpatialHashUpdateSectionsTask mSpatialHashUpdateSectionsTask;

	ParticleShapesUpdateInput mPacketShapesUpdateTaskInput;
	ParticleCollisionUpdateInput mCollisionUpdateTaskInput;

	Ps::AlignedAllocator<16, Ps::ReflectionAllocator<char> > mAlign16;

	friend class Collision;
	friend class Dynamics;
};

//----------------------------------------------------------------------------//

/*!
Compute AABB of a packet given its coordinates.
Enlarge the bounding box such that a particle on the current boundary could
travel the maximum distance and would still be inside the enlarged volume.
*/
PX_FORCE_INLINE void ParticleSystemSimCpu::getPacketBounds(const GridCellVector& coord, PxBounds3& bounds)
{
	PxVec3 gridOrigin(static_cast<PxReal>(coord.x), static_cast<PxReal>(coord.y), static_cast<PxReal>(coord.z));
	gridOrigin *= mCollision.getParameter().packetSize;

	PxVec3 collisionRangeVec(mCollision.getParameter().collisionRange);
	bounds.minimum = gridOrigin - collisionRangeVec;
	bounds.maximum = gridOrigin + PxVec3(mCollision.getParameter().packetSize) + collisionRangeVec;
}

PX_FORCE_INLINE PxReal
ParticleSystemSimCpu::computeViscosityMultiplier(PxReal viscosityStd, PxReal particleMassStd, PxReal radius6Std)
{
	PxReal wViscosityLaplacianScalarStd = 45.0f / (PxPi * radius6Std);
	return (wViscosityLaplacianScalarStd * viscosityStd * particleMassStd);
}

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_PARTICLE_SYSTEM_SIM_CPU_H
