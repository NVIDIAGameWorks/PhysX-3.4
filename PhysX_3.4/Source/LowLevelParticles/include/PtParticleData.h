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

#ifndef PT_PARTICLE_DATA_H
#define PT_PARTICLE_DATA_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "PtParticleSystemCore.h"
#include "PtParticle.h"

namespace physx
{

namespace Pt
{

class ParticleData : public ParticleSystemState
{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	PX_NOCOPY(ParticleData)
  public:
	//---------------------------
	// Implements ParticleSystemState
	virtual bool addParticlesV(const PxParticleCreationData& creationData);
	virtual void removeParticlesV(PxU32 count, const PxStrideIterator<const PxU32>& indices);
	virtual void removeParticlesV();
	virtual PxU32 getParticleCountV() const;
	virtual void getParticlesV(ParticleSystemStateDataDesc& particles, bool fullState, bool devicePtr) const;
	virtual void setPositionsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices,
	                           const PxStrideIterator<const PxVec3>& positions);
	virtual void setVelocitiesV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices,
	                            const PxStrideIterator<const PxVec3>& velocities);
	virtual void setRestOffsetsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices,
	                             const PxStrideIterator<const PxF32>& restOffsets);
	virtual void addDeltaVelocitiesV(const Cm::BitMap& bufferMap, const PxVec3* buffer, PxReal multiplier);

	virtual PxBounds3 getWorldBoundsV() const;
	virtual PxU32 getMaxParticlesV() const;
	//~Implements ParticleSystemState
	//---------------------------

	PX_FORCE_INLINE PxU32 getMaxParticles() const
	{
		return mMaxParticles;
	}
	PX_FORCE_INLINE PxU32 getParticleCount() const
	{
		return mValidParticleCount;
	}
	PX_FORCE_INLINE const Cm::BitMap& getParticleMap() const
	{
		return mParticleMap;
	}
	PX_FORCE_INLINE PxU32 getValidParticleRange() const
	{
		return mValidParticleRange;
	}

	PX_FORCE_INLINE Particle* getParticleBuffer()
	{
		return mParticleBuffer;
	}
	PX_FORCE_INLINE PxF32* getRestOffsetBuffer()
	{
		return mRestOffsetBuffer;
	}
	PX_FORCE_INLINE PxBounds3& getWorldBounds()
	{
		return mWorldBounds;
	}

	// creation with copy
	static ParticleData* create(ParticleSystemStateDataDesc& particles, const PxBounds3& bounds);

	// creation with init
	static ParticleData* create(PxU32 maxParticles, bool perParticleRestOffsets);
	static ParticleData* create(PxDeserializationContext& context);

	// creation from memory

	// release this instance and associated memory
	void release();

	// exports particle state and aggregated data to the binary stream.
	void exportData(PxSerializationContext& stream);
	static void getBinaryMetaData(PxOutputStream& stream);
	// special function to get rid of non transferable state
	void clearSimState();

	void onOriginShift(const PxVec3& shift);

  private:
	// placement serialization
	ParticleData(PxU32 maxParticles, bool perParticleRestOffset);
	ParticleData(ParticleSystemStateDataDesc& particles, const PxBounds3& bounds);

	// inplace deserialization
	ParticleData(PxU8* address);

	virtual ~ParticleData();

	PX_FORCE_INLINE static PxU32 getHeaderSize()
	{
		return (sizeof(ParticleData) + 15) & ~15;
	}
	PX_FORCE_INLINE static PxU32 getParticleBufferSize(PxU32 maxParticles)
	{
		return maxParticles * sizeof(Particle);
	}
	PX_FORCE_INLINE static PxU32 getRestOffsetBufferSize(PxU32 maxParticles, bool perParticleRestOffsets)
	{
		return perParticleRestOffsets ? maxParticles * sizeof(PxF32) : 0;
	}
	PX_FORCE_INLINE static PxU32 getBitmapSize(PxU32 maxParticles)
	{
		return ((maxParticles + 31) >> 5) * 4;
	}

	PX_FORCE_INLINE static PxU32 getTotalSize(PxU32 maxParticles, bool perParticleRestOffsets)
	{
		return getHeaderSize() + getDataSize(maxParticles, perParticleRestOffsets);
	}

	static PxU32 getDataSize(PxU32 maxParticles, bool perParticleRestOffsets)
	{
		PxU32 size = (getBitmapSize(maxParticles) + 15) & ~15;
		size += getParticleBufferSize(maxParticles);
		size += getRestOffsetBufferSize(maxParticles, perParticleRestOffsets);
		return size;
	}

	PX_FORCE_INLINE void removeParticle(PxU32 particleIndex);

	void fixupPointers();

  private:
	// This class is laid out following a strict convention for serialization/deserialization
	bool mOwnMemory;
	PxU32 mMaxParticles;       // Maximal number of particles.
	bool mHasRestOffsets;      // Whether per particle offsets are supported.
	PxU32 mValidParticleRange; // Index range in which valid particles are situated.
	PxU32 mValidParticleCount; // The number of valid particles.
	PxBounds3 mWorldBounds;    // World bounds including all particles.
	Particle* mParticleBuffer; // Main particle data buffer.
	PxF32* mRestOffsetBuffer;  // Per particle rest offsets.
	Cm::BitMap mParticleMap;   // Contains occupancy of all per particle data buffers.
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_PARTICLE_DATA_H
