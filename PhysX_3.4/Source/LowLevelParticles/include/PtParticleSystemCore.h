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

#ifndef PT_PARTICLE_SYSTEM_CORE_H
#define PT_PARTICLE_SYSTEM_CORE_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "foundation/PxBounds3.h"
#include "foundation/PxStrideIterator.h"

#include "CmPhysXCommon.h"
#include "PtParticleSystemFlags.h"
#include "particles/PxParticleReadData.h"
#include "CmBitMap.h"

namespace physx
{

class PxParticleCreationData;
class PxSerializationContext;

namespace Pt
{

typedef size_t ShapeHandle;
typedef size_t BodyHandle;

/*!
Particle system / particle fluid parameter. API + internal.
*/
struct ParticleSystemParameter
{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

	ParticleSystemParameter(const PxEMPTY) : particleReadDataFlags(PxEmpty)
	{
	}
	ParticleSystemParameter()
	{
	}

	PxReal restParticleDistance;
	PxReal kernelRadiusMultiplier;
	PxReal viscosity;
	PxReal surfaceTension;
	PxReal fadeInTime;
	PxU32 flags;
	PxU32 packetSizeMultiplierLog2;
	PxReal restitution;
	PxReal dynamicFriction;
	PxReal staticFriction;
	PxReal restDensity;
	PxReal damping;
	PxReal stiffness;
	PxReal maxMotionDistance;
	PxReal restOffset;
	PxReal contactOffset;
	PxPlane projectionPlane;
	PxParticleReadDataFlags particleReadDataFlags;
	PxU32 noiseCounter; // Needed for deterministic temporal noise
};

/*!
Descriptor for particle retrieval
*/
struct ParticleSystemStateDataDesc
{
	PxU32 maxParticles;
	PxU32 numParticles;
	PxU32 validParticleRange;
	const Cm::BitMap* bitMap;
	PxStrideIterator<const PxVec3> positions;
	PxStrideIterator<const PxVec3> velocities;
	PxStrideIterator<const ParticleFlags> flags;
	PxStrideIterator<const PxF32> restOffsets;
};

/*!
Descriptor for particle retrieval: TODO
*/
struct ParticleSystemSimDataDesc
{
	PxStrideIterator<const PxF32> densities;            //! Particle densities
	PxStrideIterator<const PxVec3> collisionNormals;    //! Particle collision normals
	PxStrideIterator<const PxVec3> collisionVelocities; //! Particle collision velocities
	PxStrideIterator<const PxVec3> twoWayImpluses;      //! collision impulses(for two way interaction)
	PxStrideIterator<BodyHandle> twoWayBodies;          //! Colliding rigid bodies each particle (zero if no collision)
};

class ParticleSystemState
{
  public:
	virtual ~ParticleSystemState()
	{
	}
	virtual bool addParticlesV(const PxParticleCreationData& creationData) = 0;
	virtual void removeParticlesV(PxU32 count, const PxStrideIterator<const PxU32>& indices) = 0;
	virtual void removeParticlesV() = 0;

	/**
	If fullState is set, the entire particle state is read, ignoring PxParticleReadDataFlags
	*/
	virtual void getParticlesV(ParticleSystemStateDataDesc& particles, bool fullState, bool devicePtr) const = 0;
	virtual void setPositionsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices,
	                           const PxStrideIterator<const PxVec3>& positions) = 0;
	virtual void setVelocitiesV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices,
	                            const PxStrideIterator<const PxVec3>& velocities) = 0;
	virtual void setRestOffsetsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices,
	                             const PxStrideIterator<const PxF32>& restOffsets) = 0;
	virtual void addDeltaVelocitiesV(const Cm::BitMap& bufferMap, const PxVec3* buffer, PxReal multiplier) = 0;

	virtual PxBounds3 getWorldBoundsV() const = 0;
	virtual PxU32 getMaxParticlesV() const = 0;
	virtual PxU32 getParticleCountV() const = 0;
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_PARTICLE_SYSTEM_CORE_H
