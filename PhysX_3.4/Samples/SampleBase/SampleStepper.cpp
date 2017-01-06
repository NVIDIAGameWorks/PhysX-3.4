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

#include "SampleStepper.h"
#include "PhysXSample.h"
#include "PxScene.h"


bool DebugStepper::advance(PxScene* scene, PxReal dt, void* scratchBlock, PxU32 scratchBlockSize)
{
	mTimer.getElapsedSeconds();
	
	{
		PxSceneWriteLock writeLock(*scene);
		scene->simulate(mStepSize, NULL, scratchBlock, scratchBlockSize);
	}

	return true;
}

void DebugStepper::wait(PxScene* scene)
{
	mSample->onSubstepPreFetchResult();
	{
		PxSceneWriteLock writeLock(*scene);
		scene->fetchResults(true, NULL);
	}
	mSimulationTime = (PxReal)mTimer.getElapsedSeconds();
	mSample->onSubstep(mStepSize);
}

void StepperTask::run()
{
	mStepper->substepDone(this);
	release();
}

void StepperTaskSimulate::run()
{
	mStepper->simulate(mCont);
	mStepper->getSample().onSubstepStart(mStepper->getSubStepSize());
}


void MultiThreadStepper::simulate(physx::PxBaseTask* ownerTask)
{
	PxSceneWriteLock writeLock(*mScene);

	mScene->simulate(mSubStepSize, ownerTask, mScratchBlock, mScratchBlockSize);
}

void MultiThreadStepper::renderDone()
{
	if(mFirstCompletionPending)
	{
		mCompletion0.removeReference();
		mFirstCompletionPending = false;
	}
}



bool MultiThreadStepper::advance(PxScene* scene, PxReal dt, void* scratchBlock, PxU32 scratchBlockSize)
{
	mScratchBlock = scratchBlock;
	mScratchBlockSize = scratchBlockSize;

	if(!mSync)
		mSync = SAMPLE_NEW(PsSyncAlloc);

	substepStrategy(dt, mNbSubSteps, mSubStepSize);
	
	if(mNbSubSteps == 0) return false;

	mScene = scene;

	mSync->reset();

	mCurrentSubStep = 1;

	mCompletion0.setContinuation(*mScene->getTaskManager(), NULL);

	mSimulationTime = 0.0f;
	mTimer.getElapsedSeconds();

	// take first substep
	substep(mCompletion0);	
	mFirstCompletionPending = true;

	return true;
}

void MultiThreadStepper::substepDone(StepperTask* ownerTask)
{
	mSample->onSubstepPreFetchResult();

	{
#if !PX_PROFILE
		PxSceneWriteLock writeLock(*mScene);
#endif
		mScene->fetchResults(true);
	}

	PxReal delta = (PxReal)mTimer.getElapsedSeconds();
	mSimulationTime += delta;

	mSample->onSubstep(mSubStepSize);

	if(mCurrentSubStep>=mNbSubSteps)
	{
		mSync->set();
	}
	else
	{
		StepperTask &s = ownerTask == &mCompletion0 ? mCompletion1 : mCompletion0;
		s.setContinuation(*mScene->getTaskManager(), NULL);
		mCurrentSubStep++;

		mTimer.getElapsedSeconds();

		substep(s);

		// after the first substep, completions run freely
		s.removeReference();
	}
}


void MultiThreadStepper::substep(StepperTask& completionTask)
{
	// setup any tasks that should run in parallel to simulate()
	mSample->onSubstepSetup(mSubStepSize, &completionTask);

	// step
	{
		mSimulateTask.setContinuation(&completionTask);
		mSimulateTask.removeReference();
	}
	// parallel sample tasks are started in mSolveTask (after solve was called which acquires a write lock).
}

void FixedStepper::substepStrategy(const PxReal stepSize, PxU32& substepCount, PxReal& substepSize)
{
	if(mAccumulator > mFixedSubStepSize)
		mAccumulator = 0.0f;

	// don't step less than the step size, just accumulate
	mAccumulator  += stepSize;
	if(mAccumulator < mFixedSubStepSize)
	{
		substepCount = 0;
		return;
	}

	substepSize = mFixedSubStepSize;
	substepCount = PxMin(PxU32(mAccumulator/mFixedSubStepSize), mMaxSubSteps);

	mAccumulator -= PxReal(substepCount)*substepSize;
}

void VariableStepper::substepStrategy(const PxReal stepSize, PxU32& substepCount, PxReal& substepSize)
{
	if(mAccumulator > mMaxSubStepSize)
		mAccumulator = 0.0f;

	// don't step less than the min step size, just accumulate
	mAccumulator  += stepSize;
	if(mAccumulator < mMinSubStepSize)
	{
		substepCount = 0;
		return;
	}

	substepCount = PxMin(PxU32(PxCeil(mAccumulator/mMaxSubStepSize)), mMaxSubSteps);
	substepSize = PxMin(mAccumulator/substepCount, mMaxSubStepSize);

	mAccumulator -= PxReal(substepCount)*substepSize;
}


