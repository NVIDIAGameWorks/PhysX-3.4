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


#ifndef DY_THREADCONTEXT_H
#define DY_THREADCONTEXT_H

#include "foundation/PxTransform.h"
#include "PxvConfig.h"
#include "CmBitMap.h"
#include "CmMatrix34.h"
#include "PxcThreadCoherentCache.h"
#include "DyThresholdTable.h"
#include "PsAllocator.h"
#include "PsAllocator.h"
#include "GuContactBuffer.h"
#include "DySolverConstraintDesc.h"
#include "PxvDynamics.h"
#include "DyArticulation.h"
#include "DyFrictionPatchStreamPair.h"
#include "PxcConstraintBlockStream.h"
#include "DyCorrelationBuffer.h"

namespace physx
{
struct PxsIndexedContactManager;

namespace Dy
{

/*!
Cache information specific to the software implementation(non common).

See PxcgetThreadContext.

Not thread-safe, so remember to have one object per thread!

TODO! refactor this and rename(it is a general per thread cache). Move transform cache into its own class.
*/
class ThreadContext : 
	public PxcThreadCoherentCache<ThreadContext, PxcNpMemBlockPool>::EntryBase
{
	PX_NOCOPY(ThreadContext)
public:

#if PX_ENABLE_SIM_STATS
	struct ThreadSimStats
	{
		void clear()
		{

			numActiveConstraints = 0;
			numActiveDynamicBodies = 0;
			numActiveKinematicBodies = 0;
			numAxisSolverConstraints = 0;

		}

		PxU32 numActiveConstraints;
		PxU32 numActiveDynamicBodies;
		PxU32 numActiveKinematicBodies;
		PxU32 numAxisSolverConstraints;

	};
#endif

	//TODO: tune cache size based on number of active objects.
	ThreadContext(PxcNpMemBlockPool* memBlockPool);
	void reset();
	void resizeArrays(PxU32 frictionConstraintDescCount, PxU32 articulationCount);

	PX_FORCE_INLINE	Ps::Array<ArticulationSolverDesc>&		getArticulations()								{ return mArticulations;					}


#if PX_ENABLE_SIM_STATS
	PX_FORCE_INLINE ThreadSimStats& getSimStats()
	{
		return mThreadSimStats;
	}
#endif

	Gu::ContactBuffer mContactBuffer;

		// temporary buffer for correlation
	PX_ALIGN(16, CorrelationBuffer			mCorrelationBuffer); 

	FrictionPatchStreamPair		mFrictionPatchStreamPair;	// patch streams

	PxsConstraintBlockManager		mConstraintBlockManager;	// for when this thread context is "lead" on an island
	PxcConstraintBlockStream 		mConstraintBlockStream;		// constraint block pool


	// this stuff is just used for reformatting the solver data. Hopefully we should have a more
	// sane format for this when the dust settles - so it's just temporary. If we keep this around
	// here we should move these from public to private

	PxU32 mNumDifferentBodyConstraints;
	PxU32 mNumDifferentBodyFrictionConstraints;
	PxU32 mNumSelfConstraints;
	PxU32 mNumSelfFrictionConstraints;
	PxU32 mNumSelfConstraintBlocks;
	PxU32 mNumSelfConstraintFrictionBlocks;

	Ps::Array<PxU32>					mConstraintsPerPartition;
	Ps::Array<PxU32>					mFrictionConstraintsPerPartition;
	Ps::Array<PxU32>					mPartitionNormalizationBitmap;
	PxsBodyCore**						mBodyCoreArray;
	PxsRigidBody**						mRigidBodyArray;
	Articulation**						mArticulationArray;
	Cm::SpatialVector*					motionVelocityArray;
	PxU32*								bodyRemapTable;
	PxU32*								mNodeIndexArray;

	//Constraint info for normal constraint sovler
	PxSolverConstraintDesc*			contactConstraintDescArray;
	PxU32								contactDescArraySize;
	PxSolverConstraintDesc*			orderedContactConstraints;
	PxConstraintBatchHeader*			contactConstraintBatchHeaders;
	PxU32								numContactConstraintBatches;

	//Constraint info for partitioning
	PxSolverConstraintDesc*			tempConstraintDescArray;

	//Additional constraint info for 1d/2d friction model
	Ps::Array<PxSolverConstraintDesc>	frictionConstraintDescArray;
	Ps::Array<PxConstraintBatchHeader> frictionConstraintBatchHeaders;

	//Info for tracking compound contact managers (temporary data - could use scratch memory!)
	Ps::Array<CompoundContactManager> compoundConstraints;

	//Used for sorting constraints. Temporary, could use scratch memory
	Ps::Array<const PxsIndexedContactManager*> orderedContactList;
	Ps::Array<const PxsIndexedContactManager*> tempContactList;
	Ps::Array<PxU32>					sortIndexArray;

	PxU32								numDifferentBodyBatchHeaders;
	PxU32								numSelfConstraintBatchHeaders;

	
	PxU32								mOrderedContactDescCount;
	PxU32								mOrderedFrictionDescCount;

	PxU32								mConstraintSize;

	PxU32 mAxisConstraintCount;
	SelfConstraintBlock* mSelfConstraintBlocks;
	
	SelfConstraintBlock* mSelfConstraintFrictionBlocks;

	PxU32 mMaxPartitions;
	PxU32 mMaxFrictionPartitions;
	PxU32 mMaxSolverPositionIterations;
	PxU32 mMaxSolverVelocityIterations;
	PxU32 mMaxArticulationLength;
	PxU32 mMaxArticulationSolverLength;
	
	PxSolverConstraintDesc* mContactDescPtr;
	PxSolverConstraintDesc* mStartContactDescPtr;
	PxSolverConstraintDesc* mFrictionDescPtr;

private:

	Ps::Array<ArticulationSolverDesc>	mArticulations;

#if PX_ENABLE_SIM_STATS
	ThreadSimStats				mThreadSimStats;
#endif

	public:

};

}

}

#endif //DY_THREADCONTEXT_H
