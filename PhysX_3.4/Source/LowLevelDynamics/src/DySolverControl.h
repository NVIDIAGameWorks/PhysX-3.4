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


#ifndef DY_SOLVERCOREGENERAL_H
#define DY_SOLVERCOREGENERAL_H

#include "DySolverCore.h"
#include "DySolverConstraintDesc.h"

namespace physx
{

namespace Dy
{

struct FsData;

inline void BusyWaitState(volatile PxU32* state, const PxU32 requiredState)
{
	while(requiredState != *state );
}

inline void WaitBodyRequiredState(PxU32* state, const PxU32 requiredState)
{
	if(*state != requiredState)
	{
		BusyWaitState(state, requiredState);
	}
}

inline void BusyWaitStates(volatile PxU32* stateA, volatile PxU32* stateB, const PxU32 requiredStateA, const PxU32 requiredStateB)
{
	while(*stateA != requiredStateA);
	while(*stateB != requiredStateB);
}


class BatchIterator
{
public:
	PxConstraintBatchHeader* constraintBatchHeaders;
	PxU32 mSize;
	PxU32 mCurrentIndex;

	BatchIterator(PxConstraintBatchHeader* _constraintBatchHeaders, PxU32 size) : constraintBatchHeaders(_constraintBatchHeaders),
		mSize(size), mCurrentIndex(0)
	{
	}

	PX_FORCE_INLINE const PxConstraintBatchHeader& GetCurrentHeader(const PxU32 constraintIndex)
	{
		PxU32 currentIndex = mCurrentIndex;
		while((constraintIndex - constraintBatchHeaders[currentIndex].mStartIndex) >= constraintBatchHeaders[currentIndex].mStride)
			currentIndex = (currentIndex + 1)%mSize;
		Ps::prefetchLine(&constraintBatchHeaders[currentIndex], 128);
		mCurrentIndex = currentIndex;
		return constraintBatchHeaders[currentIndex];
	}
private:
	BatchIterator& operator=(const BatchIterator&);
};


inline void SolveBlockParallel	(PxSolverConstraintDesc* PX_RESTRICT constraintList, const PxI32 batchCount, const PxI32 index,  
						 const PxI32 headerCount, SolverContext& cache, BatchIterator& iterator,
						 SolveBlockMethod solveTable[],
						 const PxI32 iteration
						)
{
	const PxI32 indA = index - (iteration * headerCount);

	const PxConstraintBatchHeader* PX_RESTRICT headers = iterator.constraintBatchHeaders;

	const PxI32 endIndex = indA + batchCount;
	for(PxI32 i = indA; i < endIndex; ++i)
	{
		const PxConstraintBatchHeader& header = headers[i];

		const PxI32 numToGrab = header.mStride;
		PxSolverConstraintDesc* PX_RESTRICT block = &constraintList[header.mStartIndex];

		Ps::prefetch(block[0].constraint, 384);

		for(PxI32 b = 0; b < numToGrab; ++b)
		{
			Ps::prefetchLine(block[b].bodyA);
			Ps::prefetchLine(block[b].bodyB);
		}

		//OK. We have a number of constraints to run...
		solveTable[header.mConstraintType](block, PxU32(numToGrab), cache);
	}
}




class SolverCoreGeneral : public SolverCore
{
public:
	static SolverCoreGeneral* create();

	// Implements SolverCore
	virtual void destroyV();

	virtual PxI32 solveVParallelAndWriteBack
		(SolverIslandParams& params) const;

	virtual void solveV_Blocks
		(SolverIslandParams& params) const;

	virtual void writeBackV
		(const PxSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize, PxConstraintBatchHeader* contactConstraintBatches, const PxU32 numBatches,
		 ThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs,
		 PxSolverBodyData* atomListData, WriteBackBlockMethod writeBackTable[]) const;

private:

	//~Implements SolverCore
};

#define SOLVEV_BLOCK_METHOD_ARGS											\
	SolverCore*	solverCore,												\
	SolverIslandParams& params

void solveVBlock(SOLVEV_BLOCK_METHOD_ARGS);

SolveBlockMethod* getSolveBlockTable();

SolveBlockMethod* getSolverConcludeBlockTable();

SolveWriteBackBlockMethod* getSolveWritebackBlockTable();


}

}

#endif //DY_SOLVERCOREGENERAL_H
