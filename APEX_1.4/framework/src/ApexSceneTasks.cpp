/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexSceneTasks.h"
#include "FrameworkPerfScope.h"

#if PX_PHYSICS_VERSION_MAJOR == 3
#include "ScopedPhysXLock.h"
#endif

#include "PsTime.h"

namespace nvidia
{
namespace apex
{

// --------- PhysXSimulateTask

PhysXSimulateTask::PhysXSimulateTask(ApexScene& scene, CheckResultsTask& checkResultsTask) 
: mScene(&scene)
, mElapsedTime(0.0f)
, mFollowingTask(NULL)
, mCheckResultsTask(checkResultsTask) 
#if PX_PHYSICS_VERSION_MAJOR == 3
, mScratchBlock(NULL)
, mScratchBlockSize(0)
#endif
{}

PhysXSimulateTask::~PhysXSimulateTask() 
{
#if PX_PHYSICS_VERSION_MAJOR == 3
	mScratchBlock = NULL;
	mScratchBlockSize = 0;
#endif
}

const char* PhysXSimulateTask::getName() const
{
	return AST_PHYSX_SIMULATE;
}


void PhysXSimulateTask::run()
{
	// record the pretick APEX time
	StatValue dataVal;
	uint64_t qpc = Time::getCurrentCounterValue();
	dataVal.Float = ApexScene::ticksToMilliseconds(mScene->mApexSimulateTickCount, qpc);
	APEX_CHECK_STAT_TIMER("--------- ApexBeforeTickTime (mApexSimulateTickCount)");
	
	APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
	mScene->mApexSimulateTickCount = qpc;

	mScene->setApexStatValue(ApexScene::ApexBeforeTickTime, dataVal);

	// start the PhysX simulation time timer
	APEX_CHECK_STAT_TIMER("--------- Set mPhysXSimulateTickCount");
	mScene->mPhysXSimulateTickCount = Time::getCurrentCounterValue();

#if PX_PHYSICS_VERSION_MAJOR == 3
	if (mScene->mPhysXScene)
	{
		PX_ASSERT(mElapsedTime >= 0.0f);
		SCOPED_PHYSX_LOCK_WRITE(mScene);
		mScene->mPhysXScene->simulate(mElapsedTime, &mCheckResultsTask, mScratchBlock, mScratchBlockSize, false);
	}
#endif

#if PX_PHYSICS_VERSION_MAJOR == 0
	if (mFollowingTask != NULL)
	{
		mFollowingTask->removeReference();
	}
#endif
}



void PhysXSimulateTask::setElapsedTime(float elapsedTime)
{
	PX_ASSERT(elapsedTime >= 0.0f);
	mElapsedTime = elapsedTime;
}



void PhysXSimulateTask::setFollowingTask(PxBaseTask* following)
{
	mFollowingTask = following;
}



// --------- CheckResultsTask

const char* CheckResultsTask::getName() const
{
	return AST_PHYSX_CHECK_RESULTS;
}


void CheckResultsTask::run()
{
#if !APEX_DURING_TICK_TIMING_FIX
	{
		// mark the end of the "during tick" simulation time
		StatValue dataVal;
		{
			uint64_t qpc = Time::getCurrentCounterValue();
			dataVal.Float = ApexScene::ticksToSeconds(mScene->mApexSimulateTickCount, qpc);
			APEX_CHECK_STAT_TIMER("--------- ApexDuringTickTime (mApexSimulateTickCount)");

			APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
			mScene->mApexSimulateTickCount = qpc;
		}
		mScene->setApexStatValue(ApexScene::ApexDuringTickTime, dataVal);
	}
#endif

#if PX_PHYSICS_VERSION_MAJOR == 3
	{
		SCOPED_PHYSX_LOCK_WRITE(mScene);
		if (mScene->mPhysXScene)
		{
			mScene->mPhysXScene->checkResults(true);
		}
	}
#endif

	// get the PhysX simulation time and add it to the ApexStats
	{
		StatValue dataVal;
		{
			uint64_t qpc = Time::getCurrentCounterValue();
			dataVal.Float = ApexScene::ticksToMilliseconds(mScene->mPhysXSimulateTickCount, qpc);
			APEX_CHECK_STAT_TIMER("--------- PhysXSimulationTime (mPhysXSimulateTickCount)");
		}

		mScene->setApexStatValue(ApexScene::PhysXSimulationTime, dataVal);
	}
}



// --------- FetchResultsTask

const char* FetchResultsTask::getName() const
{
	return AST_PHYSX_FETCH_RESULTS;
}


void FetchResultsTask::run()
{
}


void FetchResultsTask::setFollowingTask(PxBaseTask* following)
{
	mFollowingTask = following;
	if (mFollowingTask)
	{
		mFollowingTask->addReference();
	}
}



/*
* \brief Called by dispatcher after Task has been run.
*
* If you re-implement this method, you must call this base class
* version before returning.
*/
void FetchResultsTask::release()
{
	PxTask::release();

	// copy mFollowingTask into local variable, because it might be overwritten
	// as soon as mFetchResultsReady.set() is called (and before removeReference() is called on it)
	PxBaseTask* followingTask = mFollowingTask;
	mFollowingTask = NULL;

	// Allow ApexScene::fetchResults() to run (potentially unblocking game thread)
	mScene->mFetchResultsReady.set();
	
	// remove reference to the scene completion task submitted in Scene::simulate
	// this must be done after the scene's mFetchResultsReady event is set so that the
	// app's completion task can be assured that fetchResults is ready to run
	if (followingTask)
	{
		followingTask->removeReference();
	}	
}



#if APEX_DURING_TICK_TIMING_FIX
// --------- DuringTickCompleteTask

const char* DuringTickCompleteTask::getName() const
{
	return AST_DURING_TICK_COMPLETE;
}



void DuringTickCompleteTask::run()
{
	// mark the end of the "during tick" simulation time
	StatValue dataVal;
	uint64_t qpc = Time::getCurrentCounterValue();
	dataVal.Float = ApexScene::ticksToMilliseconds(mScene->mApexSimulateTickCount, qpc);
	APEX_CHECK_STAT_TIMER("--------- ApexDuringTickTime (mApexSimulateTickCount)");

	APEX_CHECK_STAT_TIMER("--------- Set mApexSimulateTickCount");
	mScene->mApexSimulateTickCount = qpc;
	
	mScene->setApexStatValue(ApexScene::ApexDuringTickTime, dataVal);
}
#endif


// --------- PhysXBetweenStepsTask

const char* PhysXBetweenStepsTask::getName() const
{
	return AST_PHYSX_BETWEEN_STEPS;
}



void PhysXBetweenStepsTask::run()
{
	PX_ASSERT(mSubStepSize > 0.0f);
	PX_ASSERT(mNumSubSteps > 0);
#if PX_PHYSICS_VERSION_MAJOR == 3
	PxScene* scene = mScene.getPhysXScene();

	if (scene != NULL)
	{
		while (mSubStepNumber < mNumSubSteps)
		{
			PX_PROFILE_ZONE("ApexSceneManualSubstep", GetInternalApexSDK()->getContextId());
			// fetch the first substep
			uint32_t errorState = 0;
			{
				SCOPED_PHYSX_LOCK_WRITE(&mScene);
				scene->fetchResults(true, &errorState);
			}
			PX_ASSERT(errorState == 0);

			for (uint32_t i = 0; i < mScene.mModuleScenes.size(); i++)
			{
				PX_PROFILE_ZONE("ModuleSceneManualSubstep", GetInternalApexSDK()->getContextId());
				mScene.mModuleScenes[i]->interStep(mSubStepNumber, mNumSubSteps);
			}

			// run the next substep
			{
				SCOPED_PHYSX_LOCK_WRITE(&mScene);
				scene->simulate(mSubStepSize);
			}

			mSubStepNumber++;
		}
	}
#endif

	mLast->removeReference(); // decrement artificially high ref count that prevented checkresults from being executed
}



void PhysXBetweenStepsTask::setSubstepSize(float substepSize, uint32_t numSubSteps)
{
	mSubStepSize = substepSize;
	mNumSubSteps = numSubSteps;
}



void PhysXBetweenStepsTask::setFollower(uint32_t substepNumber, PxTask* last)
{
	mSubStepNumber = substepNumber;
	mLast = last;

	setContinuation(last);
}



} // namespace apex
} // namespace nvidia
