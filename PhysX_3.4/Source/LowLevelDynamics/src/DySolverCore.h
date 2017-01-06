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


#ifndef DY_SOLVERCORE_H
#define DY_SOLVERCORE_H

#include "PxvConfig.h"
#include "PsArray.h"
#include "PsThread.h"


namespace physx
{

struct PxSolverBody;
struct PxSolverBodyData;
struct PxSolverConstraintDesc;
struct PxConstraintBatchHeader;

namespace Dy
{
struct ThresholdStreamElement;
	

struct ArticulationSolverDesc;
class Articulation;
struct SolverContext;

typedef void (*WriteBackMethod)(const PxSolverConstraintDesc& desc, SolverContext& cache, PxSolverBodyData& sbd0, PxSolverBodyData& sbd1);
typedef void (*SolveMethod)(const PxSolverConstraintDesc& desc, SolverContext& cache);
typedef void (*SolveBlockMethod)(const PxSolverConstraintDesc* desc, const PxU32 constraintCount, SolverContext& cache);
typedef void (*SolveWriteBackBlockMethod)(const PxSolverConstraintDesc* desc, const PxU32 constraintCount, SolverContext& cache);
typedef void (*WriteBackBlockMethod)(const PxSolverConstraintDesc* desc, const PxU32 constraintCount, SolverContext& cache);

#define PX_PROFILE_SOLVE_STALLS 0
#if PX_PROFILE_SOLVE_STALLS
#if PX_WINDOWS
#include <windows.h>


PX_FORCE_INLINE PxU64 readTimer()
{
	//return __rdtsc();

	LARGE_INTEGER i;
	QueryPerformanceCounter(&i);
	return i.QuadPart;
}

#endif
#endif


#define YIELD_THREADS 1

#if YIELD_THREADS

#define ATTEMPTS_BEFORE_BACKOFF 30000
#define ATTEMPTS_BEFORE_RETEST 10000

#endif

PX_INLINE void WaitForProgressCount(volatile PxI32* pGlobalIndex, const PxI32 targetIndex)
{
#if YIELD_THREADS
	if(*pGlobalIndex < targetIndex)
	{
		bool satisfied = false;
		PxU32 count = ATTEMPTS_BEFORE_BACKOFF;
		do
		{
			satisfied = true;
			while(*pGlobalIndex < targetIndex)
			{
				if(--count == 0)
				{
					satisfied = false;
					break;
				}
			}
			if(!satisfied)
				Ps::Thread::yield();
			count = ATTEMPTS_BEFORE_RETEST;
		}
		while(!satisfied);
	}
#else
	while(*pGlobalIndex < targetIndex);
#endif
}


#if PX_PROFILE_SOLVE_STALLS
PX_INLINE void WaitForProgressCount(volatile PxI32* pGlobalIndex, const PxI32 targetIndex, PxU64& stallTime)
{
	if(*pGlobalIndex < targetIndex)
	{
		bool satisfied = false;
		PxU32 count = ATTEMPTS_BEFORE_BACKOFF;
		do
		{
			satisfied = true;
			PxU64 startTime = readTimer();
			while(*pGlobalIndex < targetIndex)
			{
				if(--count == 0)
				{
					satisfied = false;
					break;
				}
			}
			PxU64 endTime = readTimer();
			stallTime += (endTime - startTime);
			if(!satisfied)
				Ps::Thread::yield();
			count = ATTEMPTS_BEFORE_BACKOFF;
		}
		while(!satisfied);
	}
}

#define WAIT_FOR_PROGRESS(pGlobalIndex, targetIndex) if(*pGlobalIndex < targetIndex) WaitForProgressCount(pGlobalIndex, targetIndex, stallCount)
#else
#define WAIT_FOR_PROGRESS(pGlobalIndex, targetIndex) if(*pGlobalIndex < targetIndex) WaitForProgressCount(pGlobalIndex, targetIndex)
#endif
#define WAIT_FOR_PROGRESS_NO_TIMER(pGlobalIndex, targetIndex) if(*pGlobalIndex < targetIndex) WaitForProgressCount(pGlobalIndex, targetIndex)


struct SolverIslandParams
{
	//Default friction model params
	PxU32 positionIterations;
	PxU32 velocityIterations;
	PxSolverBody* PX_RESTRICT bodyListStart;
	PxSolverBodyData* PX_RESTRICT bodyDataList;
	PxU32 bodyListSize;
	PxU32 solverBodyOffset;
	ArticulationSolverDesc* PX_RESTRICT articulationListStart; 
	PxU32 articulationListSize;
	PxSolverConstraintDesc* PX_RESTRICT constraintList;
	PxConstraintBatchHeader* constraintBatchHeaders;
	PxU32 numConstraintHeaders;
	PxU32* headersPerPartition;
	PxU32 nbPartitions;
	Cm::SpatialVector* PX_RESTRICT motionVelocityArray;
	PxU32 batchSize;
	PxsBodyCore*const* bodyArray;
	PxsRigidBody** PX_RESTRICT rigidBodies;

	//Shared state progress counters
	PxI32 constraintIndex;
	PxI32 constraintIndex2;
	PxI32 bodyListIndex;
	PxI32 bodyListIndex2;
	PxI32 bodyIntegrationListIndex;
	PxI32 numObjectsIntegrated;


	//Additional 1d/2d friction model params
	PxSolverConstraintDesc* PX_RESTRICT frictionConstraintList;
	
	PxConstraintBatchHeader* frictionConstraintBatches;
	PxU32 numFrictionConstraintHeaders;
	PxU32* frictionHeadersPerPartition;
	PxU32 nbFrictionPartitions;

	//Additional Shared state progress counters
	PxI32 frictionConstraintIndex;

	//Write-back threshold information
	ThresholdStreamElement* PX_RESTRICT thresholdStream;
	PxU32 thresholdStreamLength;

	PxI32* outThresholdPairs;
};


/*!
Interface to constraint solver cores

*/    
class SolverCore
{
public:
	virtual void destroyV() = 0;
    virtual ~SolverCore() {}
	/*
	solves dual problem exactly by GS-iterating until convergence stops
	only uses regular velocity vector for storing results, and backs up initial state, which is restored.
	the solution forces are saved in a vector.

	state should not be stored, this function is safe to call from multiple threads.

	Returns the total number of constraints that should be solved across all threads. Used for synchronization outside of this method
	*/

	virtual PxI32 solveVParallelAndWriteBack
		(SolverIslandParams& params) const = 0;


	virtual void solveV_Blocks
		(SolverIslandParams& params) const = 0;


	virtual void writeBackV
		(const PxSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize, PxConstraintBatchHeader* contactConstraintBatches, const PxU32 numConstraintBatches,
	 	 ThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs,
		 PxSolverBodyData* atomListData, WriteBackBlockMethod writeBackTable[]) const = 0;
};

}

}

#endif //DY_SOLVERCORE_H
