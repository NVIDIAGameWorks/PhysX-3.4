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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "foundation/PxPreprocessor.h"

#include "PsAllocator.h"
#include <new>
#include <stdio.h>
#include "CmPhysXCommon.h"
#include "DySolverBody.h"
#include "DySolverConstraint1D.h"
#include "DySolverContact.h"
#include "DyThresholdTable.h"
#include "DySolverControl.h"
#include "DyArticulationHelper.h"
#include "PsAtomic.h"
#include "PsIntrinsics.h"
#include "DyArticulationPImpl.h"
#include "PsThread.h"
#include "DySolverConstraintDesc.h"
#include "DySolverContext.h"

namespace physx
{

namespace Dy
{

//-----------------------------------

void solve1DBlock					(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactBlock				(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveExtContactBlock			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveExt1DBlock				(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContact_BStaticBlock		(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactPreBlock			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactPreBlock_Static	(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solve1D4_Block					(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);


void solve1DConcludeBlock				(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactConcludeBlock			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveExtContactConcludeBlock		(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveExt1DConcludeBlock			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContact_BStaticConcludeBlock	(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactPreBlock_Conclude		(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactPreBlock_ConcludeStatic(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solve1D4Block_Conclude				(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);

void solve1DBlockWriteBack				(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactBlockWriteBack			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveExtContactBlockWriteBack		(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveExt1DBlockWriteBack			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContact_BStaticBlockWriteBack	(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactPreBlock_WriteBack		(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solveContactPreBlock_WriteBackStatic(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void solve1D4Block_WriteBack			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);

void writeBack1DBlock				(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void contactBlockWriteBack			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void extContactBlockWriteBack		(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void ext1DBlockWriteBack			(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void contactPreBlock_WriteBack		(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);
void writeBack1D4Block				(const PxSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, SolverContext& cache);

// could move this to PxPreprocessor.h but 
// no implementation available for MSVC
#if PX_GCC_FAMILY
#define PX_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define PX_UNUSED_ATTRIBUTE 
#endif
 
#define DYNAMIC_ARTICULATION_REGISTRATION(x) 0

static SolveBlockMethod gVTableSolveBlock[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactBlock,														// DY_SC_TYPE_RB_CONTACT
	solve1DBlock,															// DY_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactBlock),				// DY_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DBlock),						// DY_SC_TYPE_EXT_1D
	solveContact_BStaticBlock,												// DY_SC_TYPE_STATIC_CONTACT
	solveContactBlock,														// DY_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactPreBlock,													// DY_SC_TYPE_BLOCK_RB_CONTACT
	solveContactPreBlock_Static,											// DY_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4_Block,															// DY_SC_TYPE_BLOCK_1D,
};

static SolveWriteBackBlockMethod gVTableSolveWriteBackBlock[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactBlockWriteBack,												// DY_SC_TYPE_RB_CONTACT
	solve1DBlockWriteBack,													// DY_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactBlockWriteBack),		// DY_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DBlockWriteBack),			// DY_SC_TYPE_EXT_1D
	solveContact_BStaticBlockWriteBack,										// DY_SC_TYPE_STATIC_CONTACT
	solveContactBlockWriteBack,												// DY_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactPreBlock_WriteBack,											// DY_SC_TYPE_BLOCK_RB_CONTACT
	solveContactPreBlock_WriteBackStatic,									// DY_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4Block_WriteBack,												// DY_SC_TYPE_BLOCK_1D,
};

static SolveBlockMethod gVTableSolveConcludeBlock[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactConcludeBlock,												// DY_SC_TYPE_RB_CONTACT
	solve1DConcludeBlock,													// DY_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactConcludeBlock),		// DY_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DConcludeBlock),				// DY_SC_TYPE_EXT_1D
	solveContact_BStaticConcludeBlock,										// DY_SC_TYPE_STATIC_CONTACT
	solveContactConcludeBlock,												// DY_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactPreBlock_Conclude,											// DY_SC_TYPE_BLOCK_RB_CONTACT
	solveContactPreBlock_ConcludeStatic,									// DY_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4Block_Conclude,													// DY_SC_TYPE_BLOCK_1D,
};

void SolverCoreRegisterArticulationFns()
{
	gVTableSolveBlock[DY_SC_TYPE_EXT_CONTACT] = solveExtContactBlock;
	gVTableSolveBlock[DY_SC_TYPE_EXT_1D] = solveExt1DBlock;

	gVTableSolveWriteBackBlock[DY_SC_TYPE_EXT_CONTACT] = solveExtContactBlockWriteBack;
	gVTableSolveWriteBackBlock[DY_SC_TYPE_EXT_1D] = solveExt1DBlockWriteBack;
	gVTableSolveConcludeBlock[DY_SC_TYPE_EXT_CONTACT] = solveExtContactConcludeBlock;
	gVTableSolveConcludeBlock[DY_SC_TYPE_EXT_1D] = solveExt1DConcludeBlock;
}


SolveBlockMethod* getSolveBlockTable()
{
	return gVTableSolveBlock;
}

SolveBlockMethod* getSolverConcludeBlockTable()
{
	return gVTableSolveConcludeBlock;
}

SolveWriteBackBlockMethod* getSolveWritebackBlockTable()
{
	return gVTableSolveWriteBackBlock;
}




SolverCoreGeneral* SolverCoreGeneral::create()
{
	SolverCoreGeneral* scg = reinterpret_cast<SolverCoreGeneral*>(
		PX_ALLOC(sizeof(SolverCoreGeneral), "SolverCoreGeneral"));

	if(scg)
		new (scg) SolverCoreGeneral;

	return scg;
}

void SolverCoreGeneral::destroyV()
{
	this->~SolverCoreGeneral();
	PX_FREE(this);
}

void SolverCoreGeneral::solveV_Blocks(SolverIslandParams& params) const
{

	const PxI32 TempThresholdStreamSize = 32;
	ThresholdStreamElement tempThresholdStream[TempThresholdStreamSize];

	SolverContext cache;
	cache.solverBodyArray			= params.bodyDataList;
	cache.mThresholdStream			= tempThresholdStream;
	cache.mThresholdStreamLength	= TempThresholdStreamSize;
	cache.mThresholdStreamIndex		= 0;
	cache.writeBackIteration		= false;

	PxI32 batchCount = PxI32(params.numConstraintHeaders);

	PxSolverBody* PX_RESTRICT bodyListStart = params.bodyListStart;
	const PxU32 bodyListSize = params.bodyListSize;

	Cm::SpatialVector* PX_RESTRICT motionVelocityArray = params.motionVelocityArray;

	const PxU32 velocityIterations = params.velocityIterations;
	const PxU32 positionIterations = params.positionIterations;

	const PxU32 numConstraintHeaders = params.numConstraintHeaders;
	const PxU32 articulationListSize = params.articulationListSize;

	ArticulationSolverDesc* PX_RESTRICT articulationListStart = params.articulationListStart;

	PX_ASSERT(velocityIterations >= 1);
	PX_ASSERT(positionIterations >= 1);

	if(numConstraintHeaders == 0)
	{
		for (PxU32 baIdx = 0; baIdx < bodyListSize; baIdx++)
		{
			Cm::SpatialVector& motionVel = motionVelocityArray[baIdx];
			PxSolverBody& atom = bodyListStart[baIdx];

			motionVel.linear = atom.linearVelocity;
			motionVel.angular = atom.angularState;
		}

		for (PxU32 i = 0; i < articulationListSize; i++)
			ArticulationPImpl::saveVelocity(articulationListStart[i]);

		return;
	}

	BatchIterator contactIterator(params.constraintBatchHeaders, params.numConstraintHeaders);

	PxSolverConstraintDesc* PX_RESTRICT constraintList = params.constraintList;

	//0-(n-1) iterations
	PxI32 normalIter = 0;

	for (PxU32 iteration = positionIterations; iteration > 0; iteration--)	//decreasing positive numbers == position iters
	{
		cache.doFriction = iteration<=3;

		SolveBlockParallel(constraintList, batchCount, normalIter * batchCount, batchCount, 
			cache, contactIterator, iteration == 1 ? gVTableSolveConcludeBlock : gVTableSolveBlock, normalIter);

		++normalIter;
	}

	for (PxU32 baIdx = 0; baIdx < bodyListSize; baIdx++)
	{
		const PxSolverBody& atom = bodyListStart[baIdx];
		Cm::SpatialVector& motionVel = motionVelocityArray[baIdx];
		motionVel.linear = atom.linearVelocity;
		motionVel.angular = atom.angularState;
	}
	

	for (PxU32 i = 0; i < articulationListSize; i++)
		ArticulationPImpl::saveVelocity(articulationListStart[i]);


	const PxI32 velItersMinOne = (PxI32(velocityIterations)) - 1;

	PxI32 iteration = 0;

	for(; iteration < velItersMinOne; ++iteration)
	{	

		SolveBlockParallel(constraintList, batchCount, normalIter * batchCount, batchCount, 
			cache, contactIterator, gVTableSolveBlock, normalIter);
		++normalIter;

	}

	PxI32* outThresholdPairs = params.outThresholdPairs;
	ThresholdStreamElement* PX_RESTRICT thresholdStream = params.thresholdStream;
	PxU32 thresholdStreamLength = params.thresholdStreamLength;

	cache.writeBackIteration = true;
	cache.mSharedThresholdStream = thresholdStream;
	cache.mSharedThresholdStreamLength = thresholdStreamLength;
	cache.mSharedOutThresholdPairs = outThresholdPairs;
	for(; iteration < PxI32(velocityIterations); ++iteration)
	{

		SolveBlockParallel(constraintList, batchCount, normalIter * batchCount, batchCount, 
			cache, contactIterator, gVTableSolveWriteBackBlock, normalIter);
		++normalIter;

	}	

	//Write back remaining threshold streams
	if(cache.mThresholdStreamIndex > 0)
	{
		//Write back to global buffer
		PxI32 threshIndex = physx::shdfnd::atomicAdd(outThresholdPairs, PxI32(cache.mThresholdStreamIndex)) - PxI32(cache.mThresholdStreamIndex);
		for(PxU32 b = 0; b < cache.mThresholdStreamIndex; ++b)
		{
			thresholdStream[b + threshIndex] = cache.mThresholdStream[b];
		}
		cache.mThresholdStreamIndex = 0;
	}
}

PxI32 SolverCoreGeneral::solveVParallelAndWriteBack
(SolverIslandParams& params) const
{
#if PX_PROFILE_SOLVE_STALLS
	PxU64 startTime = readTimer();

	PxU64 stallCount = 0;
#endif

	SolverContext cache;
	cache.solverBodyArray = params.bodyDataList;
	const PxU32 batchSize = params.batchSize;

	const PxI32 UnrollCount = PxI32(batchSize);
	const PxI32 SaveUnrollCount = 32;

	const PxI32 TempThresholdStreamSize = 32;
	ThresholdStreamElement tempThresholdStream[TempThresholdStreamSize];

	const PxI32 bodyListSize = PxI32(params.bodyListSize);
	const PxI32 articulationListSize = PxI32(params.articulationListSize);


	const PxI32 batchCount = PxI32(params.numConstraintHeaders);
	cache.mThresholdStream = tempThresholdStream;
	cache.mThresholdStreamLength = TempThresholdStreamSize;
	cache.mThresholdStreamIndex = 0;
	cache.writeBackIteration = false;

	const PxI32 positionIterations = PxI32(params.positionIterations);
	const PxI32 velocityIterations = PxI32(params.velocityIterations);

	PxI32* constraintIndex = &params.constraintIndex;
	PxI32* constraintIndex2 = &params.constraintIndex2;

	PxSolverConstraintDesc* PX_RESTRICT constraintList = params.constraintList;

	const PxU32 nbPartitions = params.nbPartitions;	

	PxU32* headersPerPartition = params.headersPerPartition;

	PX_UNUSED(velocityIterations);

	PX_ASSERT(velocityIterations >= 1);
	PX_ASSERT(positionIterations >= 1);

	PxI32 endIndexCount = UnrollCount;
	PxI32 index = physx::shdfnd::atomicAdd(constraintIndex, UnrollCount) - UnrollCount;
	
	BatchIterator contactIter(params.constraintBatchHeaders, params.numConstraintHeaders);

	PxI32 maxNormalIndex = 0;
	PxI32 normalIteration = 0;
	PxU32 a = 0;
	PxI32 targetConstraintIndex = 0;
	for(PxU32 i = 0; i < 2; ++i)
	{
		SolveBlockMethod* solveTable = i == 0 ? gVTableSolveBlock : gVTableSolveConcludeBlock;
		for(; a < positionIterations - 1 + i; ++a)
		{
			cache.doFriction = (positionIterations - a) <= 3;
			for(PxU32 b = 0; b < nbPartitions; ++b)
			{
				WAIT_FOR_PROGRESS(constraintIndex2, targetConstraintIndex);

				maxNormalIndex += headersPerPartition[b];
				
				PxI32 nbSolved = 0;
				while(index < maxNormalIndex)
				{
					const PxI32 remainder = PxMin(maxNormalIndex - index, endIndexCount);
					SolveBlockParallel(constraintList, remainder, index, batchCount, cache, contactIter, solveTable, 
						normalIteration);
					index += remainder;
					endIndexCount -= remainder;
					nbSolved += remainder;
					if(endIndexCount == 0)
					{
						endIndexCount = UnrollCount;
						index = physx::shdfnd::atomicAdd(constraintIndex, UnrollCount) - UnrollCount;
					}
				}
				if(nbSolved)
				{
					Ps::memoryBarrier();
					physx::shdfnd::atomicAdd(constraintIndex2, nbSolved);
				}
				targetConstraintIndex += headersPerPartition[b]; //Increment target constraint index by batch count
			}
			++normalIteration;
		}
	}

	PxI32* bodyListIndex = &params.bodyListIndex;
	PxI32* bodyListIndex2 = &params.bodyListIndex2;

	ArticulationSolverDesc* PX_RESTRICT articulationListStart = params.articulationListStart;

	PxSolverBody* PX_RESTRICT bodyListStart = params.bodyListStart;
	Cm::SpatialVector* PX_RESTRICT motionVelocityArray = params.motionVelocityArray;


	//Save velocity - articulated
	PxI32 endIndexCount2 = SaveUnrollCount;
	PxI32 index2 = physx::shdfnd::atomicAdd(bodyListIndex, SaveUnrollCount) - SaveUnrollCount;
	{
		WAIT_FOR_PROGRESS(constraintIndex2, targetConstraintIndex);
		PxI32 nbConcluded = 0;
		while(index2 < articulationListSize)
		{
			const PxI32 remainder = PxMin(SaveUnrollCount, (articulationListSize - index2));
			endIndexCount2 -= remainder;
			for(PxI32 b = 0; b < remainder; ++b, ++index2)
			{
				ArticulationPImpl::saveVelocity(articulationListStart[index2]);
			}
			if(endIndexCount2 == 0)
			{
				index2 = physx::shdfnd::atomicAdd(bodyListIndex, SaveUnrollCount) - SaveUnrollCount;
				endIndexCount2 = SaveUnrollCount;
			}
			nbConcluded += remainder;
		}

		index2 -= articulationListSize;

		//save velocity
		

		while(index2 < bodyListSize)
		{
			const PxI32 remainder = PxMin(endIndexCount2, (bodyListSize - index2));
			endIndexCount2 -= remainder;
			for(PxI32 b = 0; b < remainder; ++b, ++index2)
			{
				Ps::prefetchLine(&bodyListStart[index2 + 8]);
				Ps::prefetchLine(&motionVelocityArray[index2 + 8]);
				PxSolverBody& body = bodyListStart[index2];
				Cm::SpatialVector& motionVel = motionVelocityArray[index2];
				motionVel.linear = body.linearVelocity;
				motionVel.angular = body.angularState;
				PX_ASSERT(motionVel.linear.isFinite());
				PX_ASSERT(motionVel.angular.isFinite());
			}

			nbConcluded += remainder;
			
			//Branch not required because this is the last time we use this atomic variable
			//if(index2 < articulationListSizePlusbodyListSize)
			{
				index2 = physx::shdfnd::atomicAdd(bodyListIndex, SaveUnrollCount) - SaveUnrollCount - articulationListSize;
				endIndexCount2 = SaveUnrollCount;
			}
		}

		if(nbConcluded)
		{
			Ps::memoryBarrier();
			physx::shdfnd::atomicAdd(bodyListIndex2, nbConcluded);
		}
	}


	WAIT_FOR_PROGRESS(bodyListIndex2, (bodyListSize + articulationListSize));

	a = 1;
	for(; a < params.velocityIterations; ++a)
	{
		for(PxU32 b = 0; b < nbPartitions; ++b)
		{
			WAIT_FOR_PROGRESS(constraintIndex2, targetConstraintIndex);

			maxNormalIndex += headersPerPartition[b];
			
			PxI32 nbSolved = 0;
			while(index < maxNormalIndex)
			{
				const PxI32 remainder = PxMin(maxNormalIndex - index, endIndexCount);
				SolveBlockParallel(constraintList, remainder, index, batchCount, cache, contactIter, gVTableSolveBlock, 
					normalIteration);
				index += remainder;
				endIndexCount -= remainder;
				nbSolved += remainder;
				if(endIndexCount == 0)
				{
					endIndexCount = UnrollCount;
					index = physx::shdfnd::atomicAdd(constraintIndex, UnrollCount) - UnrollCount;
				}
			}
			if(nbSolved)
			{
				Ps::memoryBarrier();
				physx::shdfnd::atomicAdd(constraintIndex2, nbSolved);
			}
			targetConstraintIndex += headersPerPartition[b]; //Increment target constraint index by batch count
		}
		++normalIteration;
	}

	ThresholdStreamElement* PX_RESTRICT thresholdStream = params.thresholdStream;
	PxU32 thresholdStreamLength = params.thresholdStreamLength;
	PxI32* outThresholdPairs = params.outThresholdPairs;

	cache.mSharedOutThresholdPairs = outThresholdPairs;
	cache.mSharedThresholdStream = thresholdStream;
	cache.mSharedThresholdStreamLength = thresholdStreamLength;

	//Last iteration - do writeback as well!
	cache.writeBackIteration = true;
	{
		for(PxU32 b = 0; b < nbPartitions; ++b)
		{
			WAIT_FOR_PROGRESS(constraintIndex2, targetConstraintIndex);

			maxNormalIndex += headersPerPartition[b];
			
			PxI32 nbSolved = 0;
			while(index < maxNormalIndex)
			{
				const PxI32 remainder = PxMin(maxNormalIndex - index, endIndexCount);

				SolveBlockParallel(constraintList, remainder, index, batchCount, cache, contactIter, gVTableSolveWriteBackBlock, 
					normalIteration);

				index += remainder;
				endIndexCount -= remainder;
				nbSolved += remainder;
				if(endIndexCount == 0)
				{
					endIndexCount = UnrollCount;
					index = physx::shdfnd::atomicAdd(constraintIndex, UnrollCount) - UnrollCount;
				}
			}
			if(nbSolved)
			{
				Ps::memoryBarrier();
				physx::shdfnd::atomicAdd(constraintIndex2, nbSolved);
			}
			targetConstraintIndex += headersPerPartition[b]; //Increment target constraint index by batch count
		}

		if(cache.mThresholdStreamIndex > 0)
		{
			//Write back to global buffer
			PxI32 threshIndex = physx::shdfnd::atomicAdd(outThresholdPairs, PxI32(cache.mThresholdStreamIndex)) - PxI32(cache.mThresholdStreamIndex);
			for(PxU32 b = 0; b < cache.mThresholdStreamIndex; ++b)
			{
				thresholdStream[b + threshIndex] = cache.mThresholdStream[b];
			}
			cache.mThresholdStreamIndex = 0;
		}

		++normalIteration;

	}

#if PX_PROFILE_SOLVE_STALLS

	
	PxU64 endTime = readTimer();
	PxReal totalTime = (PxReal)(endTime - startTime);
	PxReal stallTime = (PxReal)stallCount;
	PxReal stallRatio = stallTime/totalTime;
	if(0)//stallRatio > 0.2f)
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency( &frequency );
		printf("Warning -- percentage time stalled = %f; stalled for %f seconds; total Time took %f seconds\n", 
			stallRatio * 100.f, stallTime/(PxReal)frequency.QuadPart, totalTime/(PxReal)frequency.QuadPart);
	}
#endif

	return normalIteration * batchCount;

}


void SolverCoreGeneral::writeBackV
(const PxSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 /*constraintListSize*/, PxConstraintBatchHeader* batchHeaders, const PxU32 numBatches,
 ThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs,
 PxSolverBodyData* atomListData, WriteBackBlockMethod writeBackTable[]) const
{
	SolverContext cache;
	cache.solverBodyArray			= atomListData;
	cache.mThresholdStream			= thresholdStream;
	cache.mThresholdStreamLength	= thresholdStreamLength;
	cache.mThresholdStreamIndex		= 0;

	PxI32 outThreshIndex = 0;
	for(PxU32 j = 0; j < numBatches; ++j)
	{
		PxU8 type = *constraintList[batchHeaders[j].mStartIndex].constraint;
		writeBackTable[type](constraintList + batchHeaders[j].mStartIndex,
			batchHeaders[j].mStride, cache);
	}

	outThresholdPairs = PxU32(outThreshIndex);
}

void solveVBlock(SOLVEV_BLOCK_METHOD_ARGS)
{
	solverCore->solveV_Blocks(params);
}

}
}


//#endif
