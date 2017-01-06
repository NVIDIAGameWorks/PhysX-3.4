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

#ifndef SAMPLE_STEPPER_H
#define SAMPLE_STEPPER_H

#include "SampleAllocator.h"
#include "SampleAllocatorSDKClasses.h"
#include "RendererMemoryMacros.h"
#include "task/PxTask.h"
#include "PxPhysicsAPI.h"
#include "PsTime.h"

class PhysXSample;

class Stepper: public SampleAllocateable
{
	public:
							Stepper() : mSample(NULL) {}
	virtual					~Stepper() {}

	virtual	bool			advance(PxScene* scene, PxReal dt, void* scratchBlock, PxU32 scratchBlockSize)	= 0;
	virtual	void			wait(PxScene* scene)				= 0;
	virtual void			substepStrategy(const PxReal stepSize, PxU32& substepCount, PxReal& substepSize) = 0;
	virtual void			postRender(const PxReal stepSize) = 0;
	
	virtual void			setSubStepper(const PxReal stepSize, const PxU32 maxSteps)	{}
	virtual	void			renderDone()							{}
	virtual	void			shutdown()								{}

	PxReal					getSimulationTime()	const				{ return mSimulationTime; }

	PhysXSample&			getSample()								{ return *mSample; }
	const PhysXSample&		getSample()	const						{ return *mSample; }
	void					setSample(PhysXSample* sample)			{ mSample = sample; }

protected:
	PhysXSample*			mSample;
	Ps::Time				mTimer;
	PxReal					mSimulationTime;

};

class MultiThreadStepper;
class StepperTask : public physx::PxLightCpuTask
{
public:
	void						setStepper(MultiThreadStepper* stepper) { mStepper = stepper; }
	MultiThreadStepper*			getStepper()							{ return mStepper; }
	const MultiThreadStepper*	getStepper() const 						{ return mStepper; }
	const char*					getName() const							{ return "Stepper Task"; }
	void						run();
protected:
	MultiThreadStepper*	mStepper;
};

class StepperTaskSimulate : public StepperTask
{
	
public:
	StepperTaskSimulate(){}
	void run();
};

class MultiThreadStepper : public Stepper
{
public:
	MultiThreadStepper()
		: mFirstCompletionPending(false)
		, mScene(NULL)
		, mSync(NULL)
		, mCurrentSubStep(0)
		, mNbSubSteps(0)
	{
		mCompletion0.setStepper(this);
		mCompletion1.setStepper(this);
		mSimulateTask.setStepper(this);
	}

	~MultiThreadStepper()	{}

	virtual bool			advance(PxScene* scene, PxReal dt, void* scratchBlock, PxU32 scratchBlockSize);
	virtual void			substepDone(StepperTask* ownerTask);
	virtual void			renderDone();
	virtual void			postRender(const PxReal stepSize){}
	
	// if mNbSubSteps is 0 then the sync will never 
	// be set so waiting would cause a deadlock
	virtual void			wait(PxScene* scene)	{	if(mNbSubSteps && mSync)mSync->wait();	}
	virtual void			shutdown()				{	DELETESINGLE(mSync);	}
	virtual void			reset() = 0;
	virtual void			substepStrategy(const PxReal stepSize, PxU32& substepCount, PxReal& substepSize) = 0;
	virtual void			simulate(physx::PxBaseTask* ownerTask);
	PxReal					getSubStepSize() const	{ return mSubStepSize; }

protected:
	void					substep(StepperTask& completionTask);

	// we need two completion tasks because when multistepping we can't submit completion0 from the
	// substepDone function which is running inside completion0
	bool				mFirstCompletionPending;
	StepperTaskSimulate	mSimulateTask;
	StepperTask			mCompletion0, mCompletion1;
	PxScene*			mScene;
	PsSyncAlloc*		mSync;

	PxU32				mCurrentSubStep;
	PxU32				mNbSubSteps;
	PxReal				mSubStepSize;
	void*				mScratchBlock;
	PxU32				mScratchBlockSize;
};

class DebugStepper : public Stepper
{
public:
	DebugStepper(const PxReal stepSize) : mStepSize(stepSize) {}
	
	virtual void substepStrategy(const PxReal stepSize, PxU32& substepCount, PxReal& substepSize)
	{
		substepCount = 1;
		substepSize = mStepSize;
	}

	virtual bool advance(PxScene* scene, PxReal dt, void* scratchBlock, PxU32 scratchBlockSize);

	virtual void			postRender(const PxReal stepSize)
	{
	}

	virtual void setSubStepper(const PxReal stepSize, const PxU32 maxSteps)
	{
		mStepSize = stepSize;
	}

	virtual void wait(PxScene* scene);

	PxReal mStepSize;
};

// The way this should be called is:
// bool stepped = advance(dt)
//
// ... reads from the scene graph for rendering
//
// if(stepped) renderDone()
//
// ... anything that doesn't need access to the physics scene
//
// if(stepped) sFixedStepper.wait()
//
// Note that per-substep callbacks to the sample need to be issued out of here, 
// between fetchResults and simulate

class FixedStepper : public MultiThreadStepper
{
public:
	FixedStepper(const PxReal subStepSize, const PxU32 maxSubSteps)
		: MultiThreadStepper()
		, mAccumulator(0)
		, mFixedSubStepSize(subStepSize)
		, mMaxSubSteps(maxSubSteps)
	{
	}

	virtual void	substepStrategy(const PxReal stepSize, PxU32& substepCount, PxReal& substepSize);
	virtual void	reset() { mAccumulator = 0.0f; }
	
	virtual void	setSubStepper(const PxReal stepSize, const PxU32 maxSteps) { mFixedSubStepSize = stepSize; mMaxSubSteps = maxSteps;}

	virtual void			postRender(const PxReal stepSize)
	{
	}

	PxReal	mAccumulator;
	PxReal	mFixedSubStepSize;
	PxU32	mMaxSubSteps;
};


class VariableStepper : public MultiThreadStepper
{
public:
	VariableStepper(const PxReal minSubStepSize, const PxReal maxSubStepSize, const PxU32 maxSubSteps)
		: MultiThreadStepper()
		, mAccumulator(0)
		, mMinSubStepSize(minSubStepSize)
		, mMaxSubStepSize(maxSubStepSize)
		, mMaxSubSteps(maxSubSteps)
	{
	}

	virtual void	substepStrategy(const PxReal stepSize, PxU32& substepCount, PxReal& substepSize);
	virtual void	reset() { mAccumulator = 0.0f; }

private:
	VariableStepper& operator=(const VariableStepper&);
			PxReal	mAccumulator;
	const	PxReal	mMinSubStepSize;
	const	PxReal	mMaxSubStepSize;
	const	PxU32	mMaxSubSteps;
};
#endif
