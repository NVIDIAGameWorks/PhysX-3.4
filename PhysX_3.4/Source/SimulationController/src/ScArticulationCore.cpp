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


#include "ScArticulationCore.h"

#include "PsFoundation.h"
#include "ScPhysics.h"
#include "ScBodyCore.h"
#include "ScArticulationSim.h"
#include "DyArticulation.h"

using namespace physx;

Sc::ArticulationCore::ArticulationCore() :
	mSim(NULL)
{
	const PxTolerancesScale& scale = Physics::getInstance().getTolerancesScale();

	mCore.internalDriveIterations	= 4;
	mCore.externalDriveIterations	= 4;
	mCore.maxProjectionIterations	= 4;
	mCore.separationTolerance		= 0.1f * scale.length;
	mCore.solverIterationCounts		= 1<<8 | 4;
	mCore.sleepThreshold			= 5e-5f * scale.speed * scale.speed;
	mCore.wakeCounter				= Physics::sWakeCounterOnCreation;
	mCore.freezeThreshold			= 5e-6f * scale.speed * scale.speed;
}


Sc::ArticulationCore::~ArticulationCore()
{
}


//--------------------------------------------------------------
//
// ArticulationCore interface implementation
//
//--------------------------------------------------------------

PxU32 Sc::ArticulationCore::getInternalDriveIterations() const
{
	return mCore.internalDriveIterations;
}

void Sc::ArticulationCore::setInternalDriveIterations(const PxU32 v)
{
	mCore.internalDriveIterations = v;
}

PxU32 Sc::ArticulationCore::getExternalDriveIterations() const
{
	return mCore.externalDriveIterations;
}

void Sc::ArticulationCore::setExternalDriveIterations(const PxU32 v)
{
	mCore.externalDriveIterations = v;
}

PxU32 Sc::ArticulationCore::getMaxProjectionIterations() const
{
	return mCore.maxProjectionIterations;
}

void Sc::ArticulationCore::setMaxProjectionIterations(const PxU32 v)
{
	mCore.maxProjectionIterations = v;
}

PxReal Sc::ArticulationCore::getSeparationTolerance() const
{
	return mCore.separationTolerance;
}

void Sc::ArticulationCore::setSeparationTolerance(const PxReal v)
{
	mCore.separationTolerance = v;
}

PxReal Sc::ArticulationCore::getWakeCounter() const
{
	return mCore.wakeCounter;
}

void Sc::ArticulationCore::setWakeCounterInternal(const PxReal v)
{
	mCore.wakeCounter = v;
}

void Sc::ArticulationCore::setWakeCounter(const PxReal v)
{
	mCore.wakeCounter = v;

#ifdef _DEBUG
	if(mSim)
		mSim->debugCheckWakeCounterOfLinks(v);
#endif
}

bool Sc::ArticulationCore::isSleeping() const
{
	return mSim ? mSim->isSleeping() : (mCore.wakeCounter == 0.0f);
}

void Sc::ArticulationCore::wakeUp(PxReal wakeCounter)
{
	mCore.wakeCounter = wakeCounter;

#ifdef _DEBUG
	if(mSim)
		mSim->debugCheckSleepStateOfLinks(false);
#endif
}

void Sc::ArticulationCore::putToSleep()
{
	mCore.wakeCounter = 0.0f;

#ifdef _DEBUG
	if(mSim)
		mSim->debugCheckSleepStateOfLinks(true);
#endif
}

PxReal Sc::ArticulationCore::getSleepThreshold() const
{
	return mCore.sleepThreshold;
}

void Sc::ArticulationCore::setSleepThreshold(const PxReal v)
{
	mCore.sleepThreshold = v;
}

PxReal Sc::ArticulationCore::getFreezeThreshold() const
{
	return mCore.freezeThreshold;
}

void Sc::ArticulationCore::setFreezeThreshold(const PxReal v)
{
	mCore.freezeThreshold = v;
}

PxU16 Sc::ArticulationCore::getSolverIterationCounts() const
{
	return mCore.solverIterationCounts;
}

void Sc::ArticulationCore::setSolverIterationCounts(const PxU16 v)
{
	mCore.solverIterationCounts = v;
}


PxArticulation* Sc::ArticulationCore::getPxArticulation()
{
	return gOffsetTable.convertScArticulation2Px(this);
}


const PxArticulation* Sc::ArticulationCore::getPxArticulation() const
{
	return gOffsetTable.convertScArticulation2Px(this);
}


Sc::ArticulationDriveCache* Sc::ArticulationCore::createDriveCache(PxReal compliance,
												  PxU32 driveIterations) const
{
	return mSim? mSim->createDriveCache(compliance, driveIterations) : NULL;
}


void Sc::ArticulationCore::updateDriveCache(ArticulationDriveCache& cache,
											PxReal compliance,
											PxU32 driveIterations) const
{
	mSim->updateDriveCache(cache, compliance, driveIterations);
}


void Sc::ArticulationCore::releaseDriveCache(Sc::ArticulationDriveCache& driveCache) const
{
	if(mSim)
		mSim->releaseDriveCache(driveCache);
}


PxU32 Sc::ArticulationCore::getCacheLinkCount(const ArticulationDriveCache& cache) const
{
	return Dy::PxvArticulationDriveCache::getLinkCount(cache);
}

void Sc::ArticulationCore::applyImpulse(Sc::BodyCore& link,
										const Sc::ArticulationDriveCache& driveCache,
										const PxVec3& force,
										const PxVec3& torque)
{
	if(mSim)
		mSim->applyImpulse(link, driveCache, force, torque);
}

void Sc::ArticulationCore::computeImpulseResponse(Sc::BodyCore& link,
												  PxVec3& linearResponse, 
												  PxVec3& angularResponse,
												  const Sc::ArticulationDriveCache& driveCache,
												  const PxVec3& force,
												  const PxVec3& torque) const
{
	if(mSim)
		mSim->computeImpulseResponse(link, linearResponse, angularResponse, driveCache, force, torque);
}

IG::NodeIndex Sc::ArticulationCore::getIslandNodeIndex() const
{
	if (mSim)
		return mSim->getIslandNodeIndex();
	return IG::NodeIndex(IG_INVALID_NODE);
}
