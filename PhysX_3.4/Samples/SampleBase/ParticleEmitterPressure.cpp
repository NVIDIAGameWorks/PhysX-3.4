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

#include "ParticleEmitterPressure.h"
#include "PsMathUtils.h"

//----------------------------------------------------------------------------//

ParticleEmitterPressure::ParticleEmitterPressure(ParticleEmitter::Shape::Enum shape, PxReal	extentX, PxReal extentY, PxReal spacing) :
	ParticleEmitter(shape, extentX, extentY, spacing),
	mSimulationAcceleration(0.0f),
	mSimulationMaxVelocity(1.0f),
	mMaxRate(10000.0f)
{
	mMaxZNoiseOffset = spacing/4.0f;
	mSites.resize(mNumSites);
	clearPredecessors();
}

//----------------------------------------------------------------------------//

ParticleEmitterPressure::~ParticleEmitterPressure()
{
}

//----------------------------------------------------------------------------//

void ParticleEmitterPressure::stepInternal(ParticleData& particles, PxReal dt, const PxVec3& externalAcceleration, PxReal maxParticleVelocity)
{
	PX_ASSERT(mNumX > 0 && mNumY > 0);
	
	mSimulationAcceleration = externalAcceleration;
	mSimulationMaxVelocity = maxParticleVelocity;
	
	PxU32 numEmittedParticles = 0;

	PxU32 maxParticlesPerStep = (PxU32)physx::shdfnd::floor(mMaxRate*dt);
	PxU32 maxParticles = PxMin(particles.maxParticles - particles.numParticles, maxParticlesPerStep);

	PxU32 siteNr = 0;	
	for(PxU32 y = 0; y != mNumY; y++)
	{
		PxReal offset = 0.0f;
		if (y%2) offset = mSpacingX * 0.5f;

		for(PxU32 x = 0; x != mNumX; x++)
		{
			if (isOutsideShape(x,y,offset))
				continue;

			SiteData& siteData = mSites[siteNr];

			//position noise
			PxVec3 posNoise;
			posNoise.x = randInRange(-mRandomPos.x, mRandomPos.x);
			posNoise.y = randInRange(-mRandomPos.y, mRandomPos.y);
			
			//special code for Z noise 
			if (!siteData.predecessor) 
				siteData.noiseZ = randInRange(-mRandomPos.z, mRandomPos.z);
			else
			{
				PxReal noiseZOffset = PxMin(mMaxZNoiseOffset, mRandomPos.z);
				siteData.noiseZ += randInRange(-noiseZOffset, noiseZOffset);
				siteData.noiseZ = PxClamp(siteData.noiseZ, mRandomPos.z, -mRandomPos.z);
			}

			posNoise.z = siteData.noiseZ;

			//set position
			PxVec3 sitePos = mBasePos + mAxisX*(offset+mSpacingX*x) + mAxisY*(mSpacingY*y) + mAxisZ*siteData.noiseZ;
			PxVec3 particlePos = sitePos + mAxisX*posNoise.x + mAxisY*posNoise.y;

			PxVec3 siteVel;
			computeSiteVelocity(siteVel, particlePos);

			if (siteData.predecessor)
			{
				predictPredecessorPos(siteData, dt);
			}
			else			{
				bool isSpawned = spawnParticle(particles, numEmittedParticles, maxParticles, particlePos, siteVel);
				if(isSpawned)
				{
					updatePredecessor(siteData, particlePos, siteVel);
				}
				else
				{
					siteData.predecessor = false;
					return;
				}
			}
			
			bool allSpawned = stepEmissionSite(siteData, particles, numEmittedParticles, maxParticles, sitePos, siteVel, dt);
			if(!allSpawned)
				return;

			siteNr++;
		}
	}
}

//----------------------------------------------------------------------------//

void ParticleEmitterPressure::clearPredecessors()
{
	PX_ASSERT(mSites.size() == mNumSites);
	for (PxU32 i = 0; i < mNumSites; i++) 
		mSites[i].predecessor = false;
}

//----------------------------------------------------------------------------//

bool ParticleEmitterPressure::stepEmissionSite(
	SiteData& siteData,
	ParticleData& spawnData,
	PxU32& spawnNum, 
	const PxU32 spawnMax,
	const PxVec3 &sitePos, 
	const PxVec3 &siteVel,
	const PxReal dt)
{
	PxReal maxDistanceMoved = 5.0f * mSpacingZ;	// don't generate long particle beams

	/**
	 * Find displacement vector of the particle's motion this frame
	 * this is not necessarily v*stepSize because a collision might have occured
	 */
	PxVec3 displacement = siteData.position - sitePos;
	PxVec3 normal = displacement;
	PxReal distanceMoved = normal.normalize();

	if (distanceMoved > maxDistanceMoved)
		distanceMoved = maxDistanceMoved;
	
	/**
	 * Place particles along line between emission point and new position
	 * starting backwards from the new position
	 * spacing between the particles is the rest spacing of the fluid 
	 */
	PxReal lastPlaced = 0.0f;
	while((lastPlaced + mSpacingZ) <= distanceMoved)
	{
		PxVec3 pos = sitePos + normal * (distanceMoved - (lastPlaced + mSpacingZ));

		PxVec3 posNoise;
		posNoise.x = randInRange(-mRandomPos.x, mRandomPos.x);
		posNoise.y = randInRange(-mRandomPos.y, mRandomPos.y);		
		
		pos += mAxisX*posNoise.x + mAxisY*posNoise.y;

		bool isSpawned = spawnParticle(spawnData, spawnNum, spawnMax, pos, siteVel);
		if(isSpawned)
		{
			updatePredecessor(siteData, pos, siteVel);
			lastPlaced += mSpacingZ;
		}
		else
		{
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------------//

void ParticleEmitterPressure::predictPredecessorPos(SiteData& siteData, PxReal dt)
{
	PxReal compensationHack = 2.0f/3.0f;

	siteData.velocity += dt*(mSimulationAcceleration);
	PxReal velAbs = siteData.velocity.magnitude();
	PxReal maxVel = mSimulationMaxVelocity;

	if (velAbs > maxVel)
	{
		PxReal scale = maxVel/velAbs;
		siteData.velocity *= scale; 
	}

	siteData.position += dt*compensationHack*siteData.velocity;
}

//----------------------------------------------------------------------------//

void ParticleEmitterPressure::updatePredecessor(SiteData& siteData, const PxVec3& position, const PxVec3& velocity)
{
	siteData.predecessor = true;
	siteData.position = position;
	siteData.velocity = velocity;
}

//----------------------------------------------------------------------------//

