/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SCENE_TASKS_H
#define APEX_SCENE_TASKS_H

#include "ApexScene.h"

#include "PsAllocator.h"

namespace nvidia
{
namespace apex
{

class PhysXSimulateTask : public PxTask, public UserAllocated
{
public:
	PhysXSimulateTask(ApexScene& scene, CheckResultsTask& checkResultsTask); 
	~PhysXSimulateTask();
	
	const char* getName() const;
	void run();
	void setElapsedTime(float elapsedTime);
	void setFollowingTask(PxBaseTask* following);

#if PX_PHYSICS_VERSION_MAJOR == 3
	void setScratchBlock(void* scratchBlock, uint32_t size)
	{
		mScratchBlock = scratchBlock;
		mScratchBlockSize = size;
	}
#endif

protected:
	ApexScene* mScene;
	float mElapsedTime;

	PxBaseTask* mFollowingTask;
	CheckResultsTask& mCheckResultsTask;

#if PX_PHYSICS_VERSION_MAJOR == 3
	void*			mScratchBlock;
	uint32_t			mScratchBlockSize;
#endif

private:
	PhysXSimulateTask& operator=(const PhysXSimulateTask&);
};



class CheckResultsTask : public PxTask, public UserAllocated
{
public:
	CheckResultsTask(ApexScene& scene) : mScene(&scene) {}

	const char* getName() const;
	void run();

protected:
	ApexScene* mScene;
};



class FetchResultsTask : public PxTask, public UserAllocated
{
public:
	FetchResultsTask(ApexScene& scene) 
	:	mScene(&scene)
	,	mFollowingTask(NULL)
	{}

	const char* getName() const;
	void run();

	/**
	* \brief Called by dispatcher after Task has been run.
	*
	* If you re-implement this method, you must call this base class
	* version before returning.
	*/
	void release();

	void setFollowingTask(PxBaseTask* following);

protected:
	ApexScene*					mScene;
	PxBaseTask*	mFollowingTask;
};


/**  
*	This task is solely meant to record the duration of APEX's "during tick" tasks.
*	It could be removed and replaced with only the check results task if it is found
*	to be a performance issue.
*/
#if APEX_DURING_TICK_TIMING_FIX
class DuringTickCompleteTask : public PxTask, public UserAllocated
{
public:
	DuringTickCompleteTask(ApexScene& scene) : mScene(&scene) {}

	const char* getName() const;
	void run();

protected:
	ApexScene* mScene;
};
#endif

/* This tasks loops all intermediate steps until the final fetchResults can be called */
class PhysXBetweenStepsTask : public PxLightCpuTask, public UserAllocated
{
public:
	PhysXBetweenStepsTask(ApexScene& scene) : mScene(scene), mSubStepSize(0.0f),
		mNumSubSteps(0), mSubStepNumber(0), mLast(NULL) {}

	const char* getName() const;
	void run();
	void setSubstepSize(float substepSize, uint32_t numSubSteps);
	void setFollower(uint32_t substepNumber, PxTask* last);

protected:
	ApexScene& mScene;
	float mSubStepSize;
	uint32_t mNumSubSteps;

	uint32_t mSubStepNumber;
	PxTask* mLast;

private:
	PhysXBetweenStepsTask& operator=(const PhysXBetweenStepsTask&);
};

}
}

#endif
