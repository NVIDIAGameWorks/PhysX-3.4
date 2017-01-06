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

#include "ParticleEmitterRate.h"
#include "PsHash.h"

//----------------------------------------------------------------------------//

/**
If the emitter has less emission sites than PARTICLE_EMITTER_SPARSE_FACTOR 
times the number of particles it needs to emit, "sparse mode" code path is applied.
*/
#define PARTICLE_EMITTER_SPARSE_FACTOR 4

/**
Defines how many random sites are choosen before giving up, avoiding spawning more than
one particle per site.
*/
#define PARTICLE_EMITTER_NUM_HASH_TRIALS 10

//----------------------------------------------------------------------------//

ParticleEmitterRate::ParticleEmitterRate(Shape::Enum shape, PxReal extentX, PxReal extentY, PxReal spacing) :
	ParticleEmitter(shape, extentX, extentY, spacing),
	mRate(1.0f),
	mParticlesToEmit(0)
{
}

//----------------------------------------------------------------------------//

ParticleEmitterRate::~ParticleEmitterRate()
{
}

//----------------------------------------------------------------------------//

void ParticleEmitterRate::stepInternal(ParticleData& particles, PxReal dt, const PxVec3& externalAcceleration, PxReal maxParticleVelocity)
{
	PX_ASSERT(mNumX > 0 && mNumY > 0);
	PxU32 numEmittedParticles = 0;

	//figure out how many particle have to be emitted with the given rate.
	mParticlesToEmit += mRate*dt;
	PxU32 numEmit = (PxU32)(mParticlesToEmit);
	if(numEmit == 0)
		return;
	
	PxU32 numLayers = (PxU32)(numEmit / (mNumX * mNumY)) + 1;
	PxReal layerDistance = dt * mVelocity / numLayers;

	PxU32 sparseMax = 0;

	//either shuffle or draw without repeat (approximation)
	bool denseEmission = (PxU32(PARTICLE_EMITTER_SPARSE_FACTOR*numEmit) > mNumSites);
	if(denseEmission)
	{
		initDenseSites();
	}
	else
	{
		sparseMax = PARTICLE_EMITTER_SPARSE_FACTOR*numEmit;
		mSites.resize(sparseMax);
	}

	// generate particles
	PxU32 l = 0;
	while(numEmit > 0)
	{
		PxVec3 layerVec = mAxisZ * (layerDistance * (PxReal)l);
		l++;

		if(denseEmission)
			shuffleDenseSites();
		else
			initSparseSiteHash(numEmit, sparseMax);

		for (PxU32 i = 0; i < mNumSites && numEmit > 0; i++)
		{
			PxU32 emissionSite;
			if (denseEmission)
				emissionSite = mSites[i];
			else
				emissionSite = pickSparseEmissionSite(sparseMax);

			PxU32 x = emissionSite / mNumY;
			PxU32 y = emissionSite % mNumY;

			PxReal offset = 0.0f;
			if (y%2) offset = mSpacingX * 0.5f;
				
			if (isOutsideShape(x,y,offset)) 
				continue;

			//position noise
			PxVec3 posNoise;
			posNoise.x = randInRange(-mRandomPos.x, mRandomPos.x);
			posNoise.y = randInRange(-mRandomPos.y, mRandomPos.y);	
			posNoise.z = randInRange(-mRandomPos.z, mRandomPos.z);	

			PxVec3 emissionPoint = mBasePos + layerVec + 
				mAxisX*(posNoise.x+offset+mSpacingX*x) + mAxisY*(posNoise.y+mSpacingY*y) + mAxisZ*posNoise.z;

			PxVec3 particleVelocity;
			computeSiteVelocity(particleVelocity, emissionPoint);

			bool isSpawned = spawnParticle(particles, numEmittedParticles, particles.maxParticles - particles.numParticles, emissionPoint, particleVelocity);
			if(isSpawned)
			{
				numEmit--;
				mParticlesToEmit -= 1.0f;
			}
			else
				return;
		}
	}
}

//----------------------------------------------------------------------------//

void ParticleEmitterRate::initDenseSites()
{
	mSites.resize(mNumSites);

	for(PxU32 i = 0; i < mNumSites; i++)
		mSites[i] = i;
}

//----------------------------------------------------------------------------//

void ParticleEmitterRate::shuffleDenseSites()
{
	PxU32 i,j;
	PX_ASSERT(mSites.size() == mNumSites);

	for (i = 0; i < mNumSites; i++) 
	{
		j = randInRange(mNumSites);
		PX_ASSERT(j<mNumSites);

		PxU32 k = mSites[i];
		mSites[i] = mSites[j]; 
		mSites[j] = k;
	}
}

//----------------------------------------------------------------------------//

void ParticleEmitterRate::initSparseSiteHash(PxU32 numEmit, PxU32 sparseMax)
{
	PX_ASSERT(PxU32(PARTICLE_EMITTER_SPARSE_FACTOR*numEmit) <= sparseMax);
	PX_ASSERT(mSites.size() == sparseMax);
	for(PxU32 i = 0; i < sparseMax; i++)
		mSites[i] = 0xffffffff;
}

//----------------------------------------------------------------------------//

PxU32 ParticleEmitterRate::pickSparseEmissionSite(PxU32 sparseMax)
{
	PxU32 emissionSite = randInRange(mNumSites);
	PxU32 hashKey = Ps::hash(emissionSite);
	PxU32 hashIndex = hashKey % sparseMax;
	PxU32 numTrials = 0;
	while(mSites[hashIndex] != 0xffffffff && numTrials < PARTICLE_EMITTER_NUM_HASH_TRIALS)
	{
		emissionSite = randInRange(mNumSites);
		hashKey = Ps::hash(emissionSite);
		hashIndex = hashKey % sparseMax;
		numTrials++;
	}
	//allow sites to be used multiple times if mSites[hashIndex] == 0xffffffff
	return mSites[hashIndex] = emissionSite;
}

//----------------------------------------------------------------------------//

