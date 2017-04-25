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

#ifndef CRAB_MANAGER_H
#define	CRAB_MANAGER_H

#include "foundation/Px.h"
#include "foundation/PxSimpleTypes.h"
#include "common/PxPhysXCommonConfig.h"
#include "PsSync.h"
#include "CmTask.h"
#include "PsArray.h"

class SceneCrab;
class Crab;
class Compound;
struct ShaderMaterial;
class ShaderShadow;
class CrabManager;


class CompletionTask : public physx::Cm::Task
{
	physx::shdfnd::Sync& mSync;

public:

	CompletionTask(physx::shdfnd::Sync& sync) : physx::Cm::Task(0), mSync(sync)
	{
	}

	virtual void runInternal() { mSync.set(); }

	virtual const char* getName() const { return "CrabCompletionTask"; }
};

class CrabUpdateStateTask : public physx::Cm::Task
{
public:
	CrabManager* mCrabManager;
	physx::PxU32 mStartIndex;
	physx::PxU32 mEndIndex;


	CrabUpdateStateTask(CrabManager* crabManager, const physx::PxU32 startIndex, const physx::PxU32 endIndex) :
		physx::Cm::Task(0), mCrabManager(crabManager), mStartIndex(startIndex), mEndIndex(endIndex)
	{
	}

	virtual void runInternal();


	virtual const char* getName() const { return "CrabCompletionTask"; }
};

class CrabManager
{
public:
	CrabManager();
	~CrabManager();

	void initialize(const physx::PxU32 nbCrabs);
	void setCrabScene(SceneCrab* sceneCrab) { mSceneCrab = sceneCrab; }

	void createCrab(const physx::PxVec3& crabPos, const physx::PxReal crabDepth, const physx::PxReal scale, const physx::PxReal legMass, const physx::PxU32 nbLegs);
	void update(const physx::PxReal dt);

	physx::PxScene& getScene();
	physx::PxPhysics& getPhysics();
	//SimScene* getSimScene();

	void setScene(physx::PxScene* scene);

	void setSceneCrab(SceneCrab* sceneCrab);
	SceneCrab* getSceneCrab();

	Compound* createObject(const physx::PxTransform &pose, const physx::PxVec3 &vel, const physx::PxVec3 &omega,
		bool particles, const ShaderMaterial &mat, bool useSecondaryPattern = false, ShaderShadow* shader = NULL);

	void syncWork();
	Crab** getCrabs() { return mCrabs.begin();  }

	

private:

	SceneCrab* mSceneCrab;
	//Crab*	   mCrabs;
	//physx::PxU32	   mNbCrabs;

	physx::shdfnd::Array<Crab*> mCrabs;

	physx::shdfnd::Sync mSync;

	CompletionTask mCompletionTask;
	//CrabUpdateStateTask* mUpdateStateTask;
	physx::shdfnd::Array<CrabUpdateStateTask*> mUpdateStateTask;
};

#endif
