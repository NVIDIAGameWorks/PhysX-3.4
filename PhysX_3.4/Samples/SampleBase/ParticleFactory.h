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

#ifndef PARTICLE_FACTORY_H
#define PARTICLE_FACTORY_H

//----------------------------------------------------------------------------//
#include "PxPhysicsAPI.h"
#include "PhysXSampleApplication.h"

//----------------------------------------------------------------------------//

struct ParticleData : public SampleAllocateable
{
	ParticleData() : maxParticles(0), numParticles(0) {}

	ParticleData(PxU32 _maxParticles)
	{
		initialize(_maxParticles);
	}

	void initialize(PxU32 _maxParticles)
	{
		maxParticles = _maxParticles;
		numParticles = 0;
		positions.resize(maxParticles);
		velocities.resize(maxParticles);
		restOffsets.resize(0);
	}

	SampleArray<PxVec3> positions;
	SampleArray<PxVec3> velocities;
	SampleArray<PxF32> restOffsets;
	PxU32	maxParticles;
	PxU32	numParticles;
};

//----------------------------------------------------------------------------//

void CreateParticleAABB(ParticleData& particleData, const PxBounds3& aabb, const PxVec3& vel, float distance);
void CreateParticleSphere(ParticleData& particleData, const PxVec3& pos, const PxVec3& vel, float distance, unsigned sideNum);
void CreateParticleRand(ParticleData& particleData, const PxVec3& center, const PxVec3& range,const PxVec3& vel);
void SetParticleRestOffsetVariance(ParticleData& particleData, PxF32 maxRestOffset, PxF32 restOffsetVariance);

//----------------------------------------------------------------------------//

#endif // PARTICLE_FACTORY_H
