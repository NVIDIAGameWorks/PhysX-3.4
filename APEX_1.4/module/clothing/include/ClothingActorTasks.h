/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_ACTOR_TASKS_H
#define CLOTHING_ACTOR_TASKS_H

#include "PxTask.h"

#if APEX_UE4
#include "PsSync.h"
#include "ApexInterface.h"
#endif

namespace nvidia
{
namespace clothing
{

class ClothingActorImpl;
class ClothingActorData;


class ClothingActorBeforeTickTask : public physx::PxLightCpuTask
{
public:
	ClothingActorBeforeTickTask(ClothingActorImpl* actor) : mActor(actor), mDeltaTime(0.0f), mSubstepSize(0.0f), mNumSubSteps(0) {}
#if APEX_UE4
	~ClothingActorBeforeTickTask() {}
#endif

	PX_INLINE void setDeltaTime(float simulationDelta, float substepSize, uint32_t numSubSteps)
	{
		mDeltaTime = simulationDelta;
		mSubstepSize = substepSize;
		mNumSubSteps = numSubSteps;
	}

	virtual void        run();
	virtual const char* getName() const;

private:
	ClothingActorImpl* mActor;
	float mDeltaTime;
	float mSubstepSize;
	uint32_t mNumSubSteps;
};



class ClothingActorDuringTickTask : public physx::PxTask
{
public:
	ClothingActorDuringTickTask(ClothingActorImpl* actor) : mActor(actor) {}

	virtual void		run();
	virtual const char*	getName() const;

private:
	ClothingActorImpl* mActor;
};



class ClothingActorFetchResultsTask : 
#if APEX_UE4
	public PxTask
#else
	public physx::PxLightCpuTask
#endif
{
public:
	ClothingActorFetchResultsTask(ClothingActorImpl* actor) : mActor(actor) {}

	virtual void		run();
	virtual const char*	getName() const;
#if APEX_UE4
	virtual void release();
#endif

private:
	ClothingActorImpl* mActor;
};


}
} // namespace nvidia

#endif
