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

#ifndef PARTICLE_EMITTER_PRESSURE_H
#define PARTICLE_EMITTER_PRESSURE_H

//----------------------------------------------------------------------------//

#include "ParticleEmitter.h"

//----------------------------------------------------------------------------//

class ParticleEmitterPressure : public ParticleEmitter
{

public:
						ParticleEmitterPressure(ParticleEmitter::Shape::Enum shape, PxReal	extentX, PxReal extentY, PxReal spacing);
	virtual				~ParticleEmitterPressure();

	virtual		void	stepInternal(ParticleData& particles, PxReal dt, const PxVec3& externalAcceleration, PxReal maxParticleVelocity);

				void	setMaxRate(PxReal v)									{ mMaxRate = v; }
				PxReal	getMaxRate() const										{ return mMaxRate; }


private:

	struct SiteData
	{
		PxVec3 position;
		PxVec3 velocity;
		bool predecessor;
		PxReal noiseZ;
	};

private:

	PxVec3 mSimulationAcceleration;
	PxReal mSimulationMaxVelocity;

	void clearPredecessors();

	bool stepEmissionSite(
		SiteData& siteData,
		ParticleData& spawnData,
		PxU32& spawnNum, 
		const PxU32 spawnMax,
		const PxVec3 &sitePos, 
		const PxVec3 &siteVel,
		const PxReal dt);

	void predictPredecessorPos(SiteData& siteData, PxReal dt);
	void updatePredecessor(SiteData& siteData, const PxVec3& position, const PxVec3& velocity);

private:

	std::vector<SiteData>	mSites;
	PxReal					mMaxZNoiseOffset;
	PxReal					mMaxRate;
};

//----------------------------------------------------------------------------//

#endif // PARTICLE_EMITTER_PRESSURE_H
