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


#ifndef PX_PHYSICS_EXTENSIONS_NP_DEFAULT_CPU_DISPATCHER_H
#define PX_PHYSICS_EXTENSIONS_NP_DEFAULT_CPU_DISPATCHER_H

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PsSync.h"
#include "PsSList.h"
#include "PxDefaultCpuDispatcher.h"
#include "ExtSharedQueueEntryPool.h"
#include "foundation/PxProfiler.h"
#include "task/PxTask.h"

namespace physx
{
	
namespace Ext
{
	class CpuWorkerThread;

#if PX_VC
#pragma warning(push)
#pragma warning(disable:4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#endif							// Because of the SList member I assume

	class DefaultCpuDispatcher : public PxDefaultCpuDispatcher, public Ps::UserAllocated
	{
		friend class TaskQueueHelper;

	private:
												DefaultCpuDispatcher() : mQueueEntryPool(0) {}
												~DefaultCpuDispatcher();
	public:
												DefaultCpuDispatcher(PxU32 numThreads, PxU32* affinityMasks);

		//---------------------------------------------------------------------------------
		// PxCpuDispatcher implementation
		//---------------------------------------------------------------------------------
		virtual			void					submitTask(PxBaseTask& task);
		virtual			PxU32					getWorkerCount()	const	{ return mNumThreads;	}

		//---------------------------------------------------------------------------------
		// PxDefaultCpuDispatcher implementation
		//---------------------------------------------------------------------------------
		virtual			void					release();

		virtual			void					setRunProfiled(bool runProfiled) { mRunProfiled = runProfiled; }

		virtual			bool					getRunProfiled()	const	{ return mRunProfiled;	}

		//---------------------------------------------------------------------------------
		// DefaultCpuDispatcher
		//---------------------------------------------------------------------------------
						PxBaseTask*				getJob();
						PxBaseTask*				stealJob();
						PxBaseTask*				fetchNextTask();
		PX_FORCE_INLINE	void					runTask(PxBaseTask& task)
												{
#if PX_SUPPORT_PXTASK_PROFILING
													if(mRunProfiled)
													{
														PX_PROFILE_ZONE(task.getName(), task.getContextId());
														task.run();
													}
													else
#endif
														task.run();
												}

    					void					waitForWork() { mWorkReady.wait(); }
						void					resetWakeSignal();

		static			void					getAffinityMasks(PxU32* affinityMasks, PxU32 threadCount);

	protected:
						CpuWorkerThread*		mWorkerThreads;
						SharedQueueEntryPool<>	mQueueEntryPool;
						Ps::SList				mJobList;
						Ps::Sync				mWorkReady;
						PxU8*					mThreadNames;
						PxU32					mNumThreads;
						bool					mShuttingDown;
						bool					mRunProfiled;
	};

#if PX_VC
#pragma warning(pop)
#endif

} // namespace Ext
}

#endif
