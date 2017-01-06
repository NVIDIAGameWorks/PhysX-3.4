/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "ClothingActorTasks.h"
#include "ClothingActorImpl.h"
#include "ModulePerfScope.h"


namespace nvidia
{
namespace clothing
{

void ClothingActorBeforeTickTask::run()
{
#if APEX_UE4	// SHORTCUT_CLOTH_TASKS
	mActor->simulate(mDeltaTime);
#else
#ifdef PROFILE
	PIXBeginNamedEvent(0, "ClothingActorBeforeTickTask");
#endif
	//PX_ASSERT(mDeltaTime > 0.0f); // need to allow simulate(0) calls
	mActor->tickSynchBeforeSimulate_LocksPhysX(mDeltaTime, mSubstepSize, 0, mNumSubSteps);
#ifdef PROFILE
	PIXEndNamedEvent();
#endif
#endif // SHORTCUT_CLOTH_TASKS
}



const char* ClothingActorBeforeTickTask::getName() const
{
	return "ClothingActorImpl::BeforeTickTask";
}


// --------------------------------------------------------------------


void ClothingActorDuringTickTask::run()
{
	mActor->tickAsynch_NoPhysX();
}



const char* ClothingActorDuringTickTask::getName() const
{
	return "ClothingActorImpl::DuringTickTask";
}

// --------------------------------------------------------------------

void ClothingActorFetchResultsTask::run()
{
#ifdef PROFILE
	PIXBeginNamedEvent(0, "ClothingActorFetchResultsTask");
#endif
	mActor->fetchResults();
	ClothingActorData& actorData = mActor->getActorData();

	actorData.tickSynchAfterFetchResults_LocksPhysX();
#ifdef PROFILE
	PIXEndNamedEvent();
#endif
#if APEX_UE4
	mActor->setFetchResultsSync();
#endif
}


#if APEX_UE4
void ClothingActorFetchResultsTask::release()
{
	PxTask::release();
}
#endif


const char* ClothingActorFetchResultsTask::getName() const
{
	return "ClothingActorImpl::FetchResultsTask";
}


}
}


