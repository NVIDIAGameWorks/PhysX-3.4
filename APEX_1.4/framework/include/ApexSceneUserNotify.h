/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SCENE_USER_NOTIFY_H
#define APEX_SCENE_USER_NOTIFY_H

#include "ApexDefs.h"

#if PX_PHYSICS_VERSION_MAJOR == 3

#include <ApexUsingNamespace.h>

#include <PxSimulationEventCallback.h>
#include <PxContactModifyCallback.h>

#include "PxSimpleTypes.h"
#include <PsArray.h>
#include <PsAllocator.h>


namespace nvidia
{
namespace apex
{

class ApexSceneUserNotify : public physx::PxSimulationEventCallback
{
public:
	ApexSceneUserNotify() : mAppNotify(NULL), mBatchAppNotify(false) {}
	virtual ~ApexSceneUserNotify();

	void addModuleNotifier(physx::PxSimulationEventCallback& notify);
	void removeModuleNotifier(physx::PxSimulationEventCallback& notify);

	void setApplicationNotifier(physx::PxSimulationEventCallback* notify)
	{
		mAppNotify = notify;
	}
	PxSimulationEventCallback* getApplicationNotifier() const
	{
		return mAppNotify;
	}

	void setBatchAppNotify(bool enable)
	{
		mBatchAppNotify = enable;
	}
	void playBatchedNotifications();

private:
	// from PxSimulationEventCallback
	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, uint32_t count);
	virtual void onWake(PxActor** actors, uint32_t count);
	virtual void onSleep(PxActor** actors, uint32_t count);
	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, uint32_t nbPairs);
	virtual void onTrigger(physx::PxTriggerPair* pairs, uint32_t count);
	virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count);

private:
	Array<physx::PxSimulationEventCallback*>	mModuleNotifiers;
	PxSimulationEventCallback*			mAppNotify;


	// for batch notification
	bool								mBatchAppNotify;

	// onConstraintBreak
	Array<physx::PxConstraintInfo>		mBatchedBreakNotifications;

	// onContact
	struct BatchedContactNotification
	{
		BatchedContactNotification(const physx::PxContactPairHeader& _pairHeader, const physx::PxContactPair* _pairs, uint32_t _nbPairs)
		{
			pairHeader			= _pairHeader;
			nbPairs				= _nbPairs;

			pairs = (physx::PxContactPair *)PX_ALLOC(sizeof(physx::PxContactPair) * nbPairs, PX_DEBUG_EXP("BatchedContactNotifications"));
			PX_ASSERT(pairs != NULL);
			for (uint32_t i=0; i<nbPairs; i++)
			{
				pairs[i] = _pairs[i];
			}
		}

		~BatchedContactNotification()
		{
			if (pairs)
			{
				PX_FREE(pairs);
				pairs = NULL;
			}
		}

		physx::PxContactPairHeader pairHeader;
		physx::PxContactPair *		pairs;
		uint32_t				nbPairs;
	};
	Array<BatchedContactNotification>		mBatchedContactNotifications;
	Array<uint32_t>							mBatchedContactStreams;

	// onWake/onSleep
	struct SleepWakeBorders
	{
		SleepWakeBorders(uint32_t s, uint32_t c, bool sleep) : start(s), count(c), sleepEvents(sleep) {}
		uint32_t start;
		uint32_t count;
		bool sleepEvents;
	};
	Array<SleepWakeBorders>				mBatchedSleepWakeEventBorders;
	Array<PxActor*>						mBatchedSleepEvents;
	Array<PxActor*>						mBatchedWakeEvents;

	// onTrigger
	Array<physx::PxTriggerPair> mBatchedTriggerReports;
};


class ApexSceneUserContactModify : public PxContactModifyCallback
{
public:
	ApexSceneUserContactModify();
	virtual ~ApexSceneUserContactModify();

	void addModuleContactModify(physx::PxContactModifyCallback& contactModify);
	void removeModuleContactModify(physx::PxContactModifyCallback& contactModify);

	void setApplicationContactModify(physx::PxContactModifyCallback* contactModify);
	PxContactModifyCallback* getApplicationContactModify() const
	{
		return mAppContactModify;
	}

private:
	// from PxContactModifyCallback
	virtual void onContactModify(physx::PxContactModifyPair* const pairs, uint32_t count);

private:
	Array<physx::PxContactModifyCallback*>	mModuleContactModify;
	PxContactModifyCallback*	   	mAppContactModify;
};

}
} // namespace nvidia::apex

#endif // PX_PHYSICS_VERSION_MAJOR == 3

#endif