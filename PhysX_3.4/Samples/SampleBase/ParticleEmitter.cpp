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

//----------------------------------------------------------------------------//

#if PARTICLE_EMITTER_RANDOMIZE_EMISSION
PxToolkit::BasicRandom ParticleEmitter::mRandom(42);
#endif

ParticleEmitter::ParticleEmitter(Shape::Enum shape, PxReal extentX, PxReal extentY, PxReal spacing) :
	mRandomPos(0.0f),
	mRandomAngle(0.0f),
	mVelocity(1.0f),
	mParticleMass(0.0f),
	mNumSites(0),
	mSpacingX(spacing),
	mLocalPose(PxTransform()),
	mFrameBody(NULL),
	mExtentX(extentX),
	mExtentY(extentY),
	mShape(shape)
{
	PX_ASSERT(spacing > 0.0f);
	updateDerivedBase();
}

//----------------------------------------------------------------------------//

ParticleEmitter::~ParticleEmitter()
{

}

//----------------------------------------------------------------------------//

void ParticleEmitter::updateDerivedBase()
{
	PX_ASSERT(mSpacingX > 0.0f);
	mSpacingY = mSpacingX * PxSqrt(3.0f) * 0.5f;
	mSpacingZ = mSpacingX;

	mNumX = 2*(int)floor(mExtentX/mSpacingX);
	mNumY = 2*(int)floor(mExtentY/mSpacingY);
	
	//SDS: limit minimal dimension to 1
	if (mNumX == 0)
	{
		mNumX = 1;
		mSpacingX = 0.0f;
	}
	if (mNumY == 0)
	{
		mNumY = 1;
		mSpacingY = 0.0f;
	}

	mNumSites = mNumX * mNumY;

	if (mShape == Shape::eELLIPSE) 
	{
		if (mNumX > 1) 
			mEllipseRadius2 = 0.5f - 1.0f/(mNumX-1);
		else			
			mEllipseRadius2 = 0.5f; 
		mEllipseRadius2 *= mEllipseRadius2;

		mEllipseConstX0 = (mNumX-0.5f) * 0.5f;
		mEllipseConstX1 = 1.0f/mNumX;
		mEllipseConstY0 = (mNumY-1.0f) * 0.5f;
		mEllipseConstY1 = PxSqrt(3.0f) * 0.5f / mNumY;
	}
	else 
	{
		mEllipseRadius2 = 0;
		mEllipseConstX0 = 0;
		mEllipseConstX1 = 0;
		mEllipseConstY0 = 0;
		mEllipseConstY1 = 0;
	}
}

//----------------------------------------------------------------------------//

void ParticleEmitter::step(ParticleData& particles, PxReal dt, const PxVec3& externalAcceleration, PxReal maxParticleVelocity)
{
	initStep(particles, dt);
	stepInternal(particles, dt, externalAcceleration, maxParticleVelocity);
	finalizeStep();
}

//----------------------------------------------------------------------------//

void ParticleEmitter::initStep(ParticleData& particles, PxReal dt)
{
	mLinMomentum = PxVec3(0.0f);
	mAngMomentum = PxVec3(0.0f);

	// state of frameBody
	if (mFrameBody) 
	{
		mBodyAngVel = mFrameBody->getAngularVelocity();
		mBodyLinVel = mFrameBody->getLinearVelocity();
		mBodyCenter = mFrameBody->getGlobalPose().p;
		mGlobalPose = mFrameBody->getGlobalPose() * mLocalPose;
	}
	else
	{
		mBodyAngVel = PxVec3(0.0f);
		mBodyLinVel = PxVec3(0.0f);
		mBodyCenter = PxVec3(0.0f);
		mGlobalPose = mLocalPose;
	}

	mAxisX = mGlobalPose.q.getBasisVector0();
	mAxisY = mGlobalPose.q.getBasisVector1();
	mAxisZ = mGlobalPose.q.getBasisVector2();

	mBasePos = mGlobalPose.p 
		- mAxisX * (mNumX-0.5f)*0.5f*mSpacingX
		- mAxisY * (mNumY-1.0f)*0.5f*mSpacingY;
}

//----------------------------------------------------------------------------//

void ParticleEmitter::finalizeStep()
{
	// apply impulse on attached body
	if (mFrameBody && mParticleMass > 0.0f)
	{
		mLinMomentum *= mParticleMass;
		mAngMomentum *= mParticleMass;

		mFrameBody->addForce(mLinMomentum, PxForceMode::eIMPULSE);
		mFrameBody->addTorque(mAngMomentum, PxForceMode::eIMPULSE);
	}
}

//----------------------------------------------------------------------------//

void ParticleEmitter::computeSiteVelocity(PxVec3& siteVel, const PxVec3& sitePos)
{
	//velocity dir noise
	PxReal noiseXYAngle = randInRange(0.0f, PxTwoPi);
	PxReal noiseZAngle  = randInRange(0.0f, mRandomAngle);

	PxVec3 noiseDirVec = mAxisX * PxCos(noiseXYAngle) + mAxisY * PxSin(noiseXYAngle);
	noiseDirVec.normalize();
	noiseDirVec = mAxisZ * PxCos(noiseZAngle) + noiseDirVec * PxSin(noiseZAngle);

	siteVel = noiseDirVec * mVelocity;

	//add emitter repulsion
	if (mParticleMass > 0.0f)
	{
		mLinMomentum -= siteVel;
		mAngMomentum -= (sitePos - mBodyCenter).cross(siteVel);
	}

	if (mFrameBody)
		siteVel += mBodyLinVel + (mBodyAngVel.cross(sitePos - mBodyCenter)); 
}

//----------------------------------------------------------------------------//

bool ParticleEmitter::spawnParticle(ParticleData& data, 
									PxU32& num, 
									const PxU32 max, 
									const PxVec3& position, 
									const PxVec3& velocity)
{
	PX_ASSERT(PxI32(max) - PxI32(num) <= PxI32(data.maxParticles) - PxI32(data.numParticles));
	if(num >= max) 
		return false;

	data.positions[data.numParticles] = position;
	data.velocities[data.numParticles] = velocity;
	data.numParticles++;
	num++;
	return true;
}

//----------------------------------------------------------------------------//
