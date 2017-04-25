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

#ifndef BP_AABB_MANAGER_TASKS_H
#define BP_AABB_MANAGER_TASKS_H

#include "PsUserAllocated.h"
#include "CmTask.h"

namespace physx
{
	class PxcScratchAllocator;
namespace Bp
{
	class SimpleAABBManager;
	class Aggregate;

	class AggregateBoundsComputationTask : public Cm::Task, public shdfnd::UserAllocated
	{
		public:
										AggregateBoundsComputationTask(PxU64 contextId) :
											Cm::Task	(contextId),
											mManager	(NULL),
											mStart		(0),
											mNbToGo		(0),
											mAggregates	(NULL)
										{}
										~AggregateBoundsComputationTask()	{}

		virtual const char*				getName() const { return "AggregateBoundsComputationTask"; }
		virtual void					runInternal();

				void					Init(SimpleAABBManager* manager, PxU32 start, PxU32 nb, Aggregate** aggregates)
										{
											mManager	= manager;
											mStart		= start;
											mNbToGo		= nb;
											mAggregates	= aggregates;
										}
		private:
				SimpleAABBManager*		mManager;
				PxU32					mStart;
				PxU32					mNbToGo;
				Aggregate**				mAggregates;

		AggregateBoundsComputationTask& operator=(const AggregateBoundsComputationTask&);
	};

	class FinalizeUpdateTask : public Cm::Task, public shdfnd::UserAllocated
	{
		public:
										FinalizeUpdateTask(PxU64 contextId) :
											Cm::Task				(contextId),
											mManager				(NULL),
											mNumCpuTasks			(0),
											mScratchAllocator		(NULL),
											mNarrowPhaseUnlockTask	(NULL)
										{}
										~FinalizeUpdateTask()	{}

		virtual const char*				getName() const { return "FinalizeUpdateTask"; }
		virtual void					runInternal();

				void					Init(SimpleAABBManager* manager, PxU32 numCpuTasks, PxcScratchAllocator* scratchAllocator, PxBaseTask* narrowPhaseUnlockTask)
										{
											mManager				= manager;
											mNumCpuTasks			= numCpuTasks;
											mScratchAllocator		= scratchAllocator;
											mNarrowPhaseUnlockTask	= narrowPhaseUnlockTask;
										}
		private:
				SimpleAABBManager*		mManager;
				PxU32					mNumCpuTasks;
				PxcScratchAllocator*	mScratchAllocator;
				PxBaseTask*				mNarrowPhaseUnlockTask;

		FinalizeUpdateTask& operator=(const FinalizeUpdateTask&);
	};

}
} //namespace physx

#endif // BP_AABB_MANAGER_TASKS_H
