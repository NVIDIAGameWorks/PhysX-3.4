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

#ifndef BP_MBP_TASKS_H
#define BP_MBP_TASKS_H

#include "PsUserAllocated.h"
#include "CmTask.h"

namespace physx
{
	class PxcScratchAllocator;

	namespace Bp
	{
		class BroadPhaseMBP;
	}

#define MBP_USE_SCRATCHPAD

	class MBPTask : public Cm::Task, public shdfnd::UserAllocated
	{
		public:
												MBPTask(PxU64 contextId) :
												Cm::Task		(contextId),
												mMBP			(NULL),
												mNumCpuTasks	(0)
												{}

		PX_FORCE_INLINE	void					setBroadphase(Bp::BroadPhaseMBP* mbp)			{ mMBP = mbp;					}
		PX_FORCE_INLINE	void					setScratchAllocator(PxcScratchAllocator* sa)	{ mScratchAllocator = sa;		}
		PX_FORCE_INLINE void					setNumCpuTasks(const PxU32 numCpuTasks)			{ mNumCpuTasks = numCpuTasks;	}

		protected:
						Bp::BroadPhaseMBP*		mMBP;
						PxU32					mNumCpuTasks;

						PxcScratchAllocator*	mScratchAllocator;

		private:
		MBPTask& operator=(const MBPTask&);
	};

	// PT: this is the main 'update' task doing the actual box pruning work.
	class MBPUpdateWorkTask : public MBPTask
	{
	public:							
								MBPUpdateWorkTask(PxU64 contextId);
								~MBPUpdateWorkTask();
		// PxBaseTask
		virtual const char*		getName() const { return "BpMBP.updateWork"; }
		//~PxBaseTask

		// Cm::Task
		virtual void			runInternal();
		//~Cm::Task

	private:
		MBPUpdateWorkTask& operator=(const MBPUpdateWorkTask&);
	};

	// PT: this task runs after MBPUpdateWorkTask. This is where MBP_PairManager::removeMarkedPairs is called, to finalize
	// the work and come up with created/removed lists. This is single-threaded.
	class MBPPostUpdateWorkTask : public MBPTask
	{
	public:
								MBPPostUpdateWorkTask(PxU64 contextId);					

		// PxBaseTask
		virtual const char*		getName() const { return "BpMBP.postUpdateWork"; }
		//~PxBaseTask

		// Cm::Task
		virtual void			runInternal();
		//~Cm::Task

	private:
		MBPPostUpdateWorkTask& operator=(const MBPPostUpdateWorkTask&);
	};

} //namespace physx

#endif // BP_MBP_TASKS_H
