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

#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include "SceneCrab.h"
#include "Crab.h"
#include "CrabManager.h"
#include "SimScene.h"
#include "CompoundCreator.h"
#include <GL/glut.h>

#include "PxTkStream.h"
#include "PxTkFile.h"
#include "CmTask.h"
using namespace PxToolkit;


CrabManager::CrabManager() : mSceneCrab(NULL), mCrabs(NULL), mCompletionTask(mSync)
{
	mSync.set();
}

CrabManager::~CrabManager()
{
	for (PxU32 i = 0; i < mCrabs.size(); ++i)
	{
		mCrabs[i]->~Crab();
		PX_FREE(mCrabs[i]);
	}

	mCrabs.clear();

	for (PxU32 i = 0; i < mUpdateStateTask.size(); ++i)
	{
		mUpdateStateTask[i]->~CrabUpdateStateTask();
		PX_FREE(mUpdateStateTask[i]);
	}

	mUpdateStateTask.clear();
}

PxScene& CrabManager::getScene()
{
	return mSceneCrab->getScene();
}

PxPhysics& CrabManager::getPhysics()
{
	return mSceneCrab->getPhysics();
}

void CrabManager::setSceneCrab(SceneCrab* sceneCrab)
{
	mSceneCrab = sceneCrab; 
}

void CrabManager::setScene(PxScene* scene)
{
	for (PxU32 a = 0; a < mCrabs.size(); ++a)
	{
		mCrabs[a]->setScene(scene);
	}
}

SceneCrab* CrabManager::getSceneCrab()
{ 
	return mSceneCrab; 
}

Compound* CrabManager::createObject(const PxTransform &pose, const PxVec3 &vel, const PxVec3 &omega,
	bool particles, const ShaderMaterial &mat, bool useSecondaryPattern, ShaderShadow* shader)
{
	return mSceneCrab->createObject(pose, vel, omega, particles, mat, useSecondaryPattern, shader,1,1);
}

void CrabManager::initialize(const PxU32 nbCrabs)
{
	mCrabs.reserve(nbCrabs);
}

void CrabManager::createCrab(const PxVec3& crabPos, const physx::PxReal crabDepth, const PxReal scale, const PxReal legMass, const PxU32 numLegs)
{
	PxU32 idx = mCrabs.size();
	mCrabs.pushBack(PX_PLACEMENT_NEW(PX_ALLOC(sizeof(Crab), PX_DEBUG_EXP("Crabs")), Crab)(this, idx%UPDATE_FREQUENCY_RESET));
	mCrabs[idx]->create(crabPos, crabDepth, scale, legMass, numLegs);
	PxU32 taskSize = mUpdateStateTask.size();
	if (taskSize == 0 || ((mCrabs.size() - mUpdateStateTask[taskSize - 1]->mStartIndex) == 64))
	{
		//Create a new task...
		mUpdateStateTask.pushBack(PX_PLACEMENT_NEW(PX_ALLOC(sizeof(CrabUpdateStateTask), PX_DEBUG_EXP("CrabUpdateTask")), CrabUpdateStateTask)(this, idx, mCrabs.size()));
	}
	mUpdateStateTask[mUpdateStateTask.size() - 1]->mEndIndex = mCrabs.size();
}

void CrabManager::update(const PxReal dt)
{
	mSync.reset();
	PxSceneWriteLock scopedLock(getScene());
	PxTaskManager* manager = getScene().getTaskManager();

	mCompletionTask.setContinuation(*manager, NULL);
	
	for (PxU32 i = 0; i < mCrabs.size(); ++i)
	{
		mCrabs[i]->update(dt);
	}

	for (PxU32 i = 0; i < mUpdateStateTask.size(); ++i)
	{
		mUpdateStateTask[i]->setContinuation(&mCompletionTask);
		mUpdateStateTask[i]->removeReference();
	}

	mCompletionTask.removeReference();

	//mSync.wait();
}

void CrabManager::syncWork()
{
	mSync.wait();
}

void CrabUpdateStateTask::runInternal()
{
	Crab** crab = mCrabManager->getCrabs();

	for (PxU32 i = mStartIndex; i < mEndIndex; ++i)
	{
		crab[i]->run();
	}
}