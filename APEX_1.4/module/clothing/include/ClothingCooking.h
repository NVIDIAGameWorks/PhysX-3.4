/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_COOKING_H
#define CLOTHING_COOKING_H

#include "ApexUsingNamespace.h"
#include "PsUserAllocated.h"

#include "PxTask.h"

namespace NvParameterized
{
class Interface;
}


namespace nvidia
{
namespace clothing
{

class ClothingScene;
class CookingAbstract;


class ClothingCookingLock
{
public:
	ClothingCookingLock() : mNumCookingDependencies(0) {}
	~ClothingCookingLock()
	{
		PX_ASSERT(mNumCookingDependencies == 0);
	}

	int32_t numCookingDependencies()
	{
		return mNumCookingDependencies;
	}
	void lockCooking();
	void unlockCooking();
private:
	int32_t mNumCookingDependencies;
};



/* These tasks contain a cooking job. They have no dependencies on each other, but the ClothingScene
 * make sure there's only ever one of them running (per ClothingScene that is).
 * ClothingScene::submitCookingTasks() will launch a new one only if no other cooking task is running.
 * At ClothingScene::fetchResults() this will be checked again.
 */
class ClothingCookingTask : public UserAllocated, public PxLightCpuTask
{
public:
	ClothingCookingTask(ClothingScene* clothingScene, CookingAbstract& job);
	~ClothingCookingTask();


	CookingAbstract* job;
	ClothingCookingTask* nextTask;

	// from LightCpuTask
	void				initCooking(PxTaskManager& tm, PxBaseTask* c);
	virtual const char* getName() const
	{
		return "ClothingCookingTask";
	}
	virtual void        run();

	NvParameterized::Interface* getResult();

	void lockObject(ClothingCookingLock* lockedObject);
	void unlockObject();

	bool waitsForBeingScheduled()
	{
		return mRefCount > 0;
	}
	bool readyForRelease()
	{
		return mState == ReadyForRelease;
	}
	void abort();

private:
	enum State
	{
		Uninit,
		WaitForRun,
		Running,
		Aborting,
		WaitForFetch,
		ReadyForRelease,
	};
	State mState;
	ClothingScene* mClothingScene;
	NvParameterized::Interface* mResult;
	ClothingCookingLock* mLockedObject;
};

}
} // namespace nvidia


#endif // CLOTHING_COOKING_H
