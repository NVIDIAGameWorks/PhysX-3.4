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


#ifndef BP_BROADPHASE_SAP_H
#define BP_BROADPHASE_SAP_H

#include "BpBroadPhase.h"
#include "BpBroadPhaseSapAux.h"
#include "CmPool.h"
#include "CmPhysXCommon.h"
#include "BpSAPTasks.h"
#include "PsUserAllocated.h"

namespace physx
{

// Forward declarations
class PxcScratchAllocator;
class PxcScratchAllocator;

namespace Gu
{
	class Axes;
}

namespace Bp
{

class SapEndPoint;
class IntegerAABB;

class BroadPhaseBatchUpdateWorkTask: public Cm::Task
{
public:

	BroadPhaseBatchUpdateWorkTask(PxU64 contextId=0) :
		Cm::Task(contextId),
		mSap(NULL),
		mAxis(0xffffffff),
		mPairs(NULL),
		mPairsSize(0),
		mPairsCapacity(0)
	{
	}

	virtual void runInternal();

	virtual const char* getName() const { return "BpBroadphaseSap.batchUpdate"; }

	void set(class BroadPhaseSap* sap, const PxU32 axis) {mSap = sap; mAxis = axis;}

	BroadPhasePair* getPairs() const {return mPairs;}
	PxU32 getPairsSize() const {return mPairsSize;}
	PxU32 getPairsCapacity() const {return mPairsCapacity;}

	void setPairs(BroadPhasePair* pairs, const PxU32 pairsCapacity) {mPairs = pairs; mPairsCapacity = pairsCapacity;}

	void setNumPairs(const PxU32 pairsSize) {mPairsSize=pairsSize;}

private:

	class BroadPhaseSap* mSap;
	PxU32 mAxis;

	BroadPhasePair* mPairs;
	PxU32 mPairsSize;
	PxU32 mPairsCapacity;
};

//KS - TODO, this could be reduced to U16 in smaller scenes
struct BroadPhaseActivityPocket
{
	PxU32 mStartIndex;
	PxU32 mEndIndex;
};


class BroadPhaseSap : public BroadPhase, public Ps::UserAllocated
{
	PX_NOCOPY(BroadPhaseSap)
public:

	friend class BroadPhaseBatchUpdateWorkTask;
	friend class SapUpdateWorkTask;
	friend class SapPostUpdateWorkTask;

										BroadPhaseSap(const PxU32 maxNbBroadPhaseOverlaps, const PxU32 maxNbStaticShapes, const PxU32 maxNbDynamicShapes, PxU64 contextID);
	virtual								~BroadPhaseSap();
	virtual	void						destroy();

	virtual	PxBroadPhaseType::Enum		getType()					const	{ return PxBroadPhaseType::eSAP;	}

	virtual	void						update(const PxU32 numCpuTasks, PxcScratchAllocator* scratchAllocator, const BroadPhaseUpdateData& updateData, physx::PxBaseTask* continuation, physx::PxBaseTask* narrowPhaseUnblockTask);
	virtual void						fetchBroadPhaseResults(physx::PxBaseTask*) {}

	virtual PxU32						getNbCreatedPairs()		const		{ return mCreatedPairsSize;		}
	virtual BroadPhasePairReport*		getCreatedPairs()					{ return mCreatedPairsArray;	}
	virtual PxU32						getNbDeletedPairs()		const		{ return mDeletedPairsSize;		}
	virtual BroadPhasePairReport*		getDeletedPairs()					{ return mDeletedPairsArray;	}

	virtual void						resizeBuffers();
	virtual void						freeBuffers();

	virtual void						shiftOrigin(const PxVec3& shift);
	//~BroadPhase

#if PX_CHECKED
	virtual bool						isValid(const BroadPhaseUpdateData& updateData) const;
#endif

	virtual BroadPhasePair*				getBroadPhasePairs() const  {return mPairs.mActivePairs;}

	virtual void						deletePairs();

private:

			PxcScratchAllocator*		mScratchAllocator;

			SapUpdateWorkTask			mSapUpdateWorkTask;
			SapPostUpdateWorkTask		mSapPostUpdateWorkTask;

	//Data passed in from updateV.
			const BpHandle*				mCreated;				
			PxU32						mCreatedSize;			
			const BpHandle*				mRemoved;				
			PxU32						mRemovedSize;				
			const BpHandle*				mUpdated;				
			PxU32						mUpdatedSize;				
			const PxBounds3*			mBoxBoundsMinMax;			
			const BpHandle*				mBoxGroups;
			const PxReal*				mContactDistance;
			PxU32						mBoxesCapacity;


	//Boxes.
			SapBox1D*					mBoxEndPts[3];			//Position of box min/max in sorted arrays of end pts (needs to have mBoxesCapacity).

	//End pts (endpts of boxes sorted along each axis).
			ValType*					mEndPointValues[3];		//Sorted arrays of min and max box coords
			BpHandle*					mEndPointDatas[3];		//Corresponding owner id and isMin/isMax for each entry in the sorted arrays of min and max box coords.

			PxU8*						mBoxesUpdated;	
			BpHandle*					mSortedUpdateElements;	
			BroadPhaseActivityPocket*	mActivityPockets;
			BpHandle*					mListNext;
			BpHandle*					mListPrev;

			PxU32						mBoxesSize;				//Number of sorted boxes + number of unsorted (new) boxes
			PxU32						mBoxesSizePrev;			//Number of sorted boxes 
			PxU32						mEndPointsCapacity;		//Capacity of sorted arrays. 

	//Default maximum number of overlap pairs 
			PxU32						mDefaultPairsCapacity;

	//Box-box overlap pairs created or removed each update.
			BpHandle*					mData;
			PxU32						mDataSize;
			PxU32						mDataCapacity;

	//All current box-box overlap pairs.
			SapPairManager				mPairs;

	//Created and deleted overlap pairs reported back through api.
			BroadPhasePairReport*		mCreatedPairsArray;
			PxU32						mCreatedPairsSize;
			PxU32						mCreatedPairsCapacity;
			BroadPhasePairReport*		mDeletedPairsArray;
			PxU32						mDeletedPairsSize;
			PxU32						mDeletedPairsCapacity;
			PxU32						mActualDeletedPairSize;

			bool						setUpdateData(const BroadPhaseUpdateData& updateData);
			void						update(physx::PxBaseTask* continuation);
			void						postUpdate(physx::PxBaseTask* continuation);

	//Batch create/remove/update.
			void						batchCreate();
			void						batchRemove();
			void						batchUpdate();

			void						batchUpdate(const PxU32 Axis, BroadPhasePair*& pairs, PxU32& pairsSize, PxU32& pairsCapacity);

			void						batchUpdateFewUpdates(const PxU32 Axis, BroadPhasePair*& pairs, PxU32& pairsSize, PxU32& pairsCapacity);

			void						performBoxPruning(const Gu::Axes axes);

			BroadPhaseBatchUpdateWorkTask mBatchUpdateTasks[3];

			PxU64						mContextID;
#if PX_DEBUG
			bool						isSelfOrdered() const;
			bool						isSelfConsistent() const;
#endif
};

} //namespace Bp

} //namespace physx

#endif //BP_BROADPHASE_SAP_H
