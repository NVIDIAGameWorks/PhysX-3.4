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

#ifndef PT_PARTICLE_SHAPE_CPU_H
#define PT_PARTICLE_SHAPE_CPU_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "foundation/PxTransform.h"
#include "foundation/PxBounds3.h"
#include "PtConfig.h"
#include "PtSpatialHash.h"
#include "PtParticleShape.h"

namespace physx
{

namespace Pt
{

class Context;

class ParticleShapeCpu : public ParticleShape
{
  public:
	ParticleShapeCpu(Context* context, PxU32 index);
	virtual ~ParticleShapeCpu();

	void init(class ParticleSystemSimCpu* particleSystem, const ParticleCell* packet);

	// Implements ParticleShapeCpu
	virtual PxBounds3 getBoundsV() const
	{
		return mBounds;
	}
	virtual void setUserDataV(void* data)
	{
		mUserData = data;
	}
	virtual void* getUserDataV() const
	{
		return mUserData;
	}
	virtual void destroyV();
	//~Implements ParticleShapeCpu

	PX_FORCE_INLINE void setFluidPacket(const ParticleCell* packet)
	{
		PX_ASSERT(packet);
		mPacket = packet;
	}
	PX_FORCE_INLINE const ParticleCell* getFluidPacket() const
	{
		return mPacket;
	}

	PX_FORCE_INLINE PxU32 getIndex() const
	{
		return mIndex;
	}
	PX_FORCE_INLINE class ParticleSystemSimCpu* getParticleSystem()
	{
		return mParticleSystem;
	}
	PX_FORCE_INLINE const class ParticleSystemSimCpu* getParticleSystem() const
	{
		return mParticleSystem;
	}
	PX_FORCE_INLINE GridCellVector getPacketCoordinates() const
	{
		return mPacketCoordinates;
	}

  private:
	PxU32 mIndex;
	class ParticleSystemSimCpu* mParticleSystem;
	PxBounds3 mBounds;
	GridCellVector mPacketCoordinates; // This is needed for the remapping process.
	const ParticleCell* mPacket;
	void* mUserData;
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_PARTICLE_SHAPE_CPU_H
