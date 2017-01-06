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


#ifndef BP_BROADPHASE_SAP_AUX_H
#define BP_BROADPHASE_SAP_AUX_H

#include "foundation/PxAssert.h"
#include "CmPhysXCommon.h"
#include "PsIntrinsics.h"
#include "PsUserAllocated.h"
#include "BpBroadPhase.h"
#include "BpBroadPhaseUpdate.h"
#include "CmBitMap.h"
#include "GuAxes.h"
#include "PxcScratchAllocator.h"

namespace physx
{

namespace Bp
{

#define NUM_SENTINELS 2

#define BP_SAP_USE_PREFETCH 1//prefetch in batchUpdate

#define BP_SAP_USE_OVERLAP_TEST_ON_REMOVES	1// "Useless" but faster overall because seriously reduces number of calls (from ~10000 to ~3 sometimes!)

//Set 1 to test for group ids in batchCreate/batchUpdate so we can avoid group id test in ComputeCreatedDeletedPairsLists
//Set 0 to neglect group id test in batchCreate/batchUpdate and delay test until ComputeCreatedDeletedPairsLists
#define BP_SAP_TEST_GROUP_ID_CREATEUPDATE 1

#if PX_USE_16_BIT_HANDLES
#define MAX_BP_HANDLE			0xffff
#define PX_REMOVED_BP_HANDLE	0xfffd
#define MAX_BP_PAIRS_MESSAGE "Only 65536 broadphase pairs are supported.  This limit has been exceeded and some pairs will be dropped \n"
#else
#define MAX_BP_HANDLE			0x3fffffff
#define PX_REMOVED_BP_HANDLE	0x3ffffffd
#define MAX_BP_PAIRS_MESSAGE "Only 4294967296 broadphase pairs are supported.  This limit has been exceeded and some pairs will be dropped \n"
#endif

PX_FORCE_INLINE	void setMinSentinel(ValType& v, BpHandle& d)
{
	v = 0x00000000;//0x00800000;  //0x00800000 is -FLT_MAX but setting it to 0 means we don't crash when we get a value outside the float range.
	d = (BP_INVALID_BP_HANDLE & ~1);
}

PX_FORCE_INLINE	void setMaxSentinel(ValType& v, BpHandle& d)
{										
	v = 0xffffffff;//0xff7fffff;  //0xff7fffff is +FLT_MAX but setting it to 0xffffffff means we don't crash when we get a value outside the float range.
	d = BP_INVALID_BP_HANDLE;
}

PX_FORCE_INLINE	void setData(BpHandle& d, PxU32 owner_box_id, const bool is_max)
{
	//v=v;
	d = BpHandle(owner_box_id<<1);
	if(is_max)	d |= 1;
}

PX_FORCE_INLINE	bool isSentinel(const BpHandle& d)	
{ 
	return (d&~1)==(BP_INVALID_BP_HANDLE & ~1);	
}

PX_FORCE_INLINE	BpHandle isMax(const BpHandle& d) 	
{
	return BpHandle(d & 1);		
}

PX_FORCE_INLINE	BpHandle getOwner(const BpHandle& d) 	
{ 
	return BpHandle(d>>1);		
}

class SapBox1D
{
public:

	PX_FORCE_INLINE					SapBox1D()	{}
	PX_FORCE_INLINE					~SapBox1D()	{}

	BpHandle		mMinMax[2];//mMinMax[0]=min, mMinMax[1]=max
};

class SapPairManager
{
public:
	SapPairManager();
	~SapPairManager();

	void						init(const PxU32 size);
	void						release();

	void						shrinkMemory();

	const BroadPhasePair*	AddPair		(BpHandle id0, BpHandle id1, const PxU8 state);
	bool						RemovePair	(BpHandle id0, BpHandle id1);
	bool						RemovePairs	(const Cm::BitMap& removedAABBs);
	const BroadPhasePair*	FindPair	(BpHandle id0, BpHandle id1)	const;

	PX_FORCE_INLINE	PxU32		GetPairIndex(const BroadPhasePair* PX_RESTRICT pair)	const
	{
		return (PxU32((size_t(pair) - size_t(mActivePairs)))/sizeof(BroadPhasePair));
	}

	BpHandle*			mHashTable;
	BpHandle*			mNext;
	PxU32				mHashSize;
	PxU32				mHashCapacity;
	PxU32				mMinAllowedHashCapacity;
	BroadPhasePair*		mActivePairs;
	PxU8*				mActivePairStates;
	PxU32				mNbActivePairs;
	PxU32				mActivePairsCapacity;
	PxU32				mMask;

	BroadPhasePair*	FindPair	(BpHandle id0, BpHandle id1, PxU32 hash_value) const;
	void				RemovePair	(BpHandle id0, BpHandle id1, PxU32 hash_value, PxU32 pair_index);
	void				reallocPairs(const bool allocRequired);

	enum
	{
		PAIR_INARRAY=1,
		PAIR_REMOVED=2,
		PAIR_NEW=4,
		PAIR_UNKNOWN=8
	};

	PX_FORCE_INLINE bool IsInArray(const BroadPhasePair* PX_RESTRICT pair) const 
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_INARRAY ? true : false;
	}
	PX_FORCE_INLINE bool IsRemoved(const BroadPhasePair* PX_RESTRICT pair) const 
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_REMOVED ? true : false;
	}
	PX_FORCE_INLINE bool IsNew(const BroadPhasePair* PX_RESTRICT pair) const
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_NEW ? true : false;
	}
	PX_FORCE_INLINE bool IsUnknown(const BroadPhasePair* PX_RESTRICT pair) const
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_UNKNOWN ? true : false;
	}

	PX_FORCE_INLINE void ClearState(const BroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs]=0;
	}

	PX_FORCE_INLINE void SetInArray(const BroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] |= PAIR_INARRAY;
	}
	PX_FORCE_INLINE void SetRemoved(const BroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] |= PAIR_REMOVED;
	}
	PX_FORCE_INLINE void SetNew(const BroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] |= PAIR_NEW;
	}
	PX_FORCE_INLINE void ClearInArray(const BroadPhasePair* PX_RESTRICT pair)
	{		
		mActivePairStates[pair-mActivePairs] &= ~PAIR_INARRAY;
	}
	PX_FORCE_INLINE void ClearRemoved(const BroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] &= ~PAIR_REMOVED;
	}
	PX_FORCE_INLINE void ClearNew(const BroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] &= ~PAIR_NEW;
	}
};

PX_FORCE_INLINE void AddData(const PxU32 data, PxcScratchAllocator* scratchAllocator, BpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	if(dataArraySize==dataArrayCapacity)
	{
		BpHandle* newDataArray = reinterpret_cast<BpHandle*>(scratchAllocator->alloc(sizeof(BpHandle)*dataArrayCapacity*2, true));
		PxMemCopy(newDataArray, dataArray, dataArrayCapacity*sizeof(BpHandle));
		scratchAllocator->free(dataArray);
		dataArray = newDataArray;
		dataArrayCapacity = dataArrayCapacity*2;
	}
	PX_UNUSED(scratchAllocator);
	PX_UNUSED(dataArrayCapacity);
	PX_ASSERT(dataArraySize<dataArrayCapacity);
	dataArray[dataArraySize]=BpHandle(data);
	dataArraySize++;
}

PX_FORCE_INLINE	bool AddPair
(const BpHandle id0, const BpHandle id1, 
 PxcScratchAllocator* scratchAllocator,
 SapPairManager& pairManager, BpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	const BroadPhasePair* UP = reinterpret_cast<const BroadPhasePair*>(pairManager.AddPair(id0, id1, SapPairManager::PAIR_UNKNOWN));

	//If the hash table has reached its limit then we're unable to add a new pair.
	if(NULL==UP)
	{
		return false;
	}

	PX_ASSERT(UP);
	if(pairManager.IsUnknown(UP))
	{
		pairManager.ClearState(UP);
		pairManager.SetInArray(UP);
		AddData(pairManager.GetPairIndex(UP), scratchAllocator, dataArray, dataArraySize, dataArrayCapacity);
		pairManager.SetNew(UP);
	}
	pairManager.ClearRemoved(UP);

	return true;
}

PX_FORCE_INLINE	void RemovePair
(BpHandle id0, BpHandle id1, 
 PxcScratchAllocator* scratchAllocator,
 SapPairManager& pairManager,  BpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	const BroadPhasePair* UP = reinterpret_cast<const BroadPhasePair*>(pairManager.FindPair(id0, id1));
	if(UP)
	{
		if(!pairManager.IsInArray(UP))
		{
			pairManager.SetInArray(UP);
			AddData(pairManager.GetPairIndex(UP), scratchAllocator, dataArray, dataArraySize, dataArrayCapacity);
		}
		pairManager.SetRemoved(UP);
	}
}

PX_FORCE_INLINE void InsertEndPoints
(const ValType* PX_RESTRICT newEndPointValues, const BpHandle* PX_RESTRICT newEndPointDatas, PxU32 numNewEndPoints,
 ValType* PX_RESTRICT endPointValues, BpHandle* PX_RESTRICT endPointDatas, const PxU32 numEndPoints, 
 SapBox1D* PX_RESTRICT boxes)
{
	ValType* const BaseEPValue = endPointValues;
	BpHandle* const BaseEPData = endPointDatas;

	const PxU32 OldSize = numEndPoints-NUM_SENTINELS;
	const PxU32 NewSize = numEndPoints-NUM_SENTINELS+numNewEndPoints;

	BaseEPValue[NewSize + 1] = BaseEPValue[OldSize + 1];
	BaseEPData[NewSize + 1] = BaseEPData[OldSize + 1];

	PxI32 WriteIdx = PxI32(NewSize);
	PxU32 CurrInsIdx = 0;

	//const SapValType* FirstValue = &BaseEPValue[0];
	const BpHandle* FirstData = &BaseEPData[0];
	const ValType* CurrentValue = &BaseEPValue[OldSize];
	const BpHandle* CurrentData = &BaseEPData[OldSize];
	while(CurrentData>=FirstData)
	{
		const ValType& SrcValue = *CurrentValue;
		const BpHandle& SrcData = *CurrentData;
		const ValType& InsValue = newEndPointValues[CurrInsIdx];
		const BpHandle& InsData = newEndPointDatas[CurrInsIdx];

		// We need to make sure we insert maxs before mins to handle exactly equal endpoints correctly
		const bool ShouldInsert = isMax(InsData) ? (SrcValue <= InsValue) : (SrcValue < InsValue);

		const ValType& MovedValue = ShouldInsert ? InsValue : SrcValue;
		const BpHandle& MovedData = ShouldInsert ? InsData : SrcData;
		BaseEPValue[WriteIdx] = MovedValue;
		BaseEPData[WriteIdx] = MovedData;
		boxes[getOwner(MovedData)].mMinMax[isMax(MovedData)] = BpHandle(WriteIdx--);

		if(ShouldInsert)
		{
			CurrInsIdx++;
			if(CurrInsIdx >= numNewEndPoints)
				break;//we just inserted the last endpoint
		}
		else
		{
			CurrentValue--;
			CurrentData--;
		}
	}
}

void ComputeCreatedDeletedPairsLists
(const BpHandle* PX_RESTRICT boxGroups, 
 const BpHandle* PX_RESTRICT dataArray, const PxU32 dataArraySize,
 PxcScratchAllocator* scratchAllocator,
 BroadPhasePairReport* & createdPairsList, PxU32& numCreatedPairs, PxU32& maxNumCreatdPairs,
 BroadPhasePairReport* & deletedPairsList, PxU32& numDeletedPairs, PxU32& maxNumDeletedPairs,
 PxU32&numActualDeletedPairs,
 SapPairManager& pairManager);

void DeletePairsLists(const PxU32 numActualDeletedPairs, BroadPhasePairReport* deletedPairsList, SapPairManager& pairManager);

void ComputeSortedLists
(Cm::BitMap* PX_RESTRICT bitmap, 
 const PxU32 insertAABBStart, const PxU32 insertAABBEnd, const BpHandle* PX_RESTRICT createdAABBs,
 SapBox1D** PX_RESTRICT asapBoxes, const BpHandle* PX_RESTRICT asapBoxGroupIds, 
 BpHandle* PX_RESTRICT asapEndPointDatas, const PxU32 numSortedEndPoints, 
 const Gu::Axes& axes,
 BpHandle* PX_RESTRICT newBoxIndicesSorted, PxU32& newBoxIndicesCount, BpHandle* PX_RESTRICT oldBoxIndicesSorted, PxU32& oldBoxIndicesCount,
 bool& allNewBoxesStatics, bool& allOldBoxesStatics);

void performBoxPruningNewNew
(const Gu::Axes& axes,
 const BpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount,  const bool allNewBoxesStatics,
 BpHandle* PX_RESTRICT minPosList0,
 SapBox1D** PX_RESTRICT asapBoxes, const BpHandle* PX_RESTRICT asapBoxGroupIds, 
 PxcScratchAllocator* scratchAllocator,
 SapPairManager& pairManager, BpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity);

void performBoxPruningNewOld
(const Gu::Axes& axes,
 const BpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount, const BpHandle* PX_RESTRICT oldBoxIndicesSorted, const PxU32 oldBoxIndicesCount,
 BpHandle* PX_RESTRICT minPosListNew,  BpHandle* PX_RESTRICT minPosListOld,
 SapBox1D** PX_RESTRICT asapBoxes, const BpHandle* PX_RESTRICT asapBoxGroupIds,
 PxcScratchAllocator* scratchAllocator,
 SapPairManager& pairManager, BpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity);

PX_FORCE_INLINE bool Intersect2D(SapBox1D** PX_RESTRICT c, SapBox1D** PX_RESTRICT b, const PxU32 axis1, const PxU32 axis2)
{
	return (b[axis1]->mMinMax[1] >= c[axis1]->mMinMax[0] && c[axis1]->mMinMax[1] >= b[axis1]->mMinMax[0] &&
		    b[axis2]->mMinMax[1] >= c[axis2]->mMinMax[0] && c[axis2]->mMinMax[1] >= b[axis2]->mMinMax[0]);
}

PX_FORCE_INLINE bool Intersect3D(SapBox1D** PX_RESTRICT c, SapBox1D** PX_RESTRICT b)
{
	return (b[0]->mMinMax[1] >= c[0]->mMinMax[0] && c[0]->mMinMax[1] >= b[0]->mMinMax[0] &&
		    b[1]->mMinMax[1] >= c[1]->mMinMax[0] && c[1]->mMinMax[1] >= b[1]->mMinMax[0] &&
			b[2]->mMinMax[1] >= c[2]->mMinMax[0] && c[2]->mMinMax[1] >= b[2]->mMinMax[0]);       
}

PX_FORCE_INLINE bool Intersect1D_Min(const ValType aMin, const ValType /*aMax*/, SapBox1D* PX_RESTRICT b, const ValType* PX_RESTRICT endPointValues)
{
	const ValType& endPointValue=endPointValues[b->mMinMax[1]];
	return (endPointValue >= aMin);
}

PX_FORCE_INLINE bool Intersect1D_Max(const ValType /*aMin*/, const ValType aMax, SapBox1D* PX_RESTRICT b, const ValType* PX_RESTRICT endPointValues)
{
	const ValType& endPointValue=endPointValues[b->mMinMax[0]];
	return (endPointValue < aMax);
}

PX_FORCE_INLINE bool Intersect2D
(const ValType bDir1Min, const ValType bDir1Max, const ValType bDir2Min, const ValType bDir2Max,
 const ValType cDir1Min, const ValType cDir1Max, const ValType cDir2Min, const ValType cDir2Max)
{
	return (bDir1Max >= cDir1Min && cDir1Max >= bDir1Min && 
			bDir2Max >= cDir2Min && cDir2Max >= bDir2Min);        
}

PX_FORCE_INLINE bool Intersect2D_Handle
(const BpHandle bDir1Min, const BpHandle bDir1Max, const BpHandle bDir2Min, const BpHandle bDir2Max,
 const BpHandle cDir1Min, const BpHandle cDir1Max, const BpHandle cDir2Min, const BpHandle cDir2Max)
{
	return (bDir1Max > cDir1Min && cDir1Max > bDir1Min && 
			bDir2Max > cDir2Min && cDir2Max > bDir2Min);        
}

PX_FORCE_INLINE bool Intersect3D
(const ValType bDir1Min, const ValType bDir1Max, const ValType bDir2Min, const ValType bDir2Max, const ValType bDir3Min, const ValType bDir3Max,
 const ValType cDir1Min, const ValType cDir1Max, const ValType cDir2Min, const ValType cDir2Max, const ValType cDir3Min, const ValType cDir3Max)
{
	return (bDir1Max >= cDir1Min && cDir1Max >= bDir1Min && 
			bDir2Max >= cDir2Min && cDir2Max >= bDir2Min &&
			bDir3Max >= cDir3Min && cDir3Max >= bDir3Min);       
}

} //namespace Bp

} //namespace physx

#endif //BP_BROADPHASE_SAP_AUX_H
