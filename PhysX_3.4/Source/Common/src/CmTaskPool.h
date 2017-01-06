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


#ifndef PX_PHYSICS_COMMON_TASKPOOL
#define PX_PHYSICS_COMMON_TASKPOOL

#include "foundation/Px.h"
#include "PsMutex.h"
#include "PsSList.h"
#include "PsAllocator.h"
#include "PsArray.h"

class PxTask;

/*
Implimentation of a thread safe task pool. (PxTask derived classes).

T is the actual type of the task(currently NphaseTask or GroupSolveTask).
*/

namespace Cm
{
	template<class T> class TaskPool : public Ps::AlignedAllocator<16>
	{
		const static PxU32 TaskPoolSlabSize=64;

	public:

		typedef Ps::SListEntry TaskPoolItem;

		PX_INLINE TaskPool() : slabArray(PX_DEBUG_EXP("taskPoolSlabArray"))
		{
			//we have to ensure that the list header is 16byte aligned for win64.
			freeTasks = (Ps::SList*)allocate(sizeof(Ps::SList), __FILE__, __LINE__);
			PX_PLACEMENT_NEW(freeTasks, Ps::SList)();

			slabArray.reserve(16);
		}

		~TaskPool()
		{
			Ps::Mutex::ScopedLock lock(slabAllocMutex);

			freeTasks->flush();

			for(PxU32 i=0;i<slabArray.size();i++)
			{
				// call destructors
				for(PxU32 j=0; j<TaskPoolSlabSize; j++)
					slabArray[i][j].~T();

				deallocate(slabArray[i]);
			}

			slabArray.clear();

			if(freeTasks!=NULL)
			{
				freeTasks->~SList();
				deallocate(freeTasks);
				freeTasks = NULL;
			}
		}

		T *allocTask()
		{
			T *rv = static_cast<T *>(freeTasks->pop());
			if(rv == NULL)
				return static_cast<T *>(allocateSlab());
			else
				return rv;
		}
		void freeTask(T *task)
		{
			freeTasks->push(*task);
		}

	private:

		T *allocateSlab()
		{
			//ack, convoluted memory macros.

			//T *newSlab=new T[TaskPoolSlabSize];

			// we must align this memory.
			T *newSlab=(T *)allocate(sizeof(T)*TaskPoolSlabSize, __FILE__, __LINE__);

			new (newSlab) T();

			//we keep one for the caller
			// and build a list of tasks and insert in the free list
			for(PxU32 i=1;i<TaskPoolSlabSize;i++)
			{
				new (&(newSlab[i])) T();
				freeTasks->push(newSlab[i]);
			}

			Ps::Mutex::ScopedLock lock(slabAllocMutex);
			slabArray.pushBack(newSlab);

			return newSlab;
		}

		Ps::Mutex slabAllocMutex;
		Ps::Array<T *> slabArray;

		Ps::SList *freeTasks;

	};


} // namespace Cm


#endif
