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


#include "DyThreadContext.h"
#include "PsBitUtils.h"

namespace physx
{
namespace Dy
{

ThreadContext::ThreadContext(PxcNpMemBlockPool* memBlockPool):
	mFrictionPatchStreamPair(*memBlockPool),
	mConstraintBlockManager	(*memBlockPool),
	mConstraintBlockStream	(*memBlockPool),
	mNumDifferentBodyConstraints(0),
	mNumSelfConstraints(0),
	mNumSelfConstraintBlocks(0),
	mConstraintsPerPartition(PX_DEBUG_EXP("ThreadContext::mConstraintsPerPartition")),
	mFrictionConstraintsPerPartition(PX_DEBUG_EXP("ThreadContext::frictionsConstraintsPerPartition")),
	mPartitionNormalizationBitmap(PX_DEBUG_EXP("ThreadContext::mPartitionNormalizationBitmap")),
	frictionConstraintDescArray(PX_DEBUG_EXP("ThreadContext::solverFrictionConstraintArray")),
	frictionConstraintBatchHeaders(PX_DEBUG_EXP("ThreadContext::frictionConstraintBatchHeaders")),
	compoundConstraints(PX_DEBUG_EXP("ThreadContext::compoundConstraints")),
	orderedContactList(PX_DEBUG_EXP("ThreadContext::orderedContactList")),
	tempContactList(PX_DEBUG_EXP("ThreadContext::tempContactList")),
	sortIndexArray(PX_DEBUG_EXP("ThreadContext::sortIndexArray")),
	mConstraintSize			(0),
	mAxisConstraintCount(0),
	mSelfConstraintBlocks(NULL),
	mMaxPartitions(0),
	mMaxSolverPositionIterations(0),
	mMaxSolverVelocityIterations(0),
	mMaxArticulationLength(0),
	mContactDescPtr(NULL),
	mFrictionDescPtr(NULL),
	mArticulations(PX_DEBUG_EXP("ThreadContext::articulations"))
	
{
#if PX_ENABLE_SIM_STATS
	mThreadSimStats.clear();
#endif
	//Defaulted to have space for 16384 bodies
	mPartitionNormalizationBitmap.reserve(512); 
	//Defaulted to have space for 128 partitions (should be more-than-enough)
	mConstraintsPerPartition.reserve(128);
}

void ThreadContext::resizeArrays(PxU32 frictionConstraintDescCount, PxU32 articulationCount)
{
	// resize resizes smaller arrays to the exact target size, which can generate a lot of churn
	frictionConstraintDescArray.forceSize_Unsafe(0);
	frictionConstraintDescArray.reserve((frictionConstraintDescCount+63)&~63);

	mArticulations.forceSize_Unsafe(0);
	mArticulations.reserve(PxMax<PxU32>(Ps::nextPowerOfTwo(articulationCount), 16));
	mArticulations.forceSize_Unsafe(articulationCount);

	mContactDescPtr = contactConstraintDescArray;
	mFrictionDescPtr = frictionConstraintDescArray.begin();
}

void ThreadContext::reset()
{
	// TODO: move these to the PxcNpThreadContext
	mFrictionPatchStreamPair.reset();
	mConstraintBlockStream.reset();

	mContactDescPtr = contactConstraintDescArray;
	mFrictionDescPtr = frictionConstraintDescArray.begin();

	mAxisConstraintCount = 0;
	mMaxSolverPositionIterations = 0;
	mMaxSolverVelocityIterations = 0;
	mNumDifferentBodyConstraints = 0;
	mNumSelfConstraints = 0;
	mSelfConstraintBlocks = NULL;
	mNumSelfConstraintBlocks = 0;
	mConstraintSize = 0;
}

}
} 
