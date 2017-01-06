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

#include "CmPhysXCommon.h"
#include "BpBroadPhaseSapAux.h"
#include "PsFoundation.h"

namespace physx
{

namespace Bp
{

PX_FORCE_INLINE void PxBpHandleSwap(BpHandle& a, BpHandle& b)													
{ 
	const BpHandle c = a; a = b; b = c;		
}

PX_FORCE_INLINE void Sort(BpHandle& id0, BpHandle& id1)										
{ 
	if(id0>id1)	PxBpHandleSwap(id0, id1);						
}

PX_FORCE_INLINE bool DifferentPair(const BroadPhasePair& p, BpHandle id0, BpHandle id1)	
{ 
	return (id0!=p.mVolA) || (id1!=p.mVolB);						
}

PX_FORCE_INLINE int Hash32Bits_1(int key)
{
	key += ~(key << 15);
	key ^=  (key >> 10);
	key +=  (key << 3);
	key ^=  (key >> 6);
	key += ~(key << 11);
	key ^=  (key >> 16);
	return key;
}

PX_FORCE_INLINE PxU32 Hash(BpHandle id0, BpHandle id1)								
{ 
	return PxU32(Hash32Bits_1( int(PxU32(id0)|(PxU32(id1)<<16)) ));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SapPairManager::SapPairManager() :
	mHashTable				(NULL),
	mNext					(NULL),
	mHashSize				(0),
	mHashCapacity			(0),
	mMinAllowedHashCapacity	(0),
	mActivePairs			(NULL),
	mActivePairStates		(NULL),
	mNbActivePairs			(0),
	mActivePairsCapacity	(0),
	mMask					(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SapPairManager::~SapPairManager()
{
	PX_ASSERT(NULL==mHashTable);
	PX_ASSERT(NULL==mNext);
	PX_ASSERT(NULL==mActivePairs);
	PX_ASSERT(NULL==mActivePairStates);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void SapPairManager::init(const PxU32 size)
{
	mHashTable=reinterpret_cast<BpHandle*>(PX_ALLOC(ALIGN_SIZE_16(sizeof(BpHandle)*size), "BpHandle"));
	mNext=reinterpret_cast<BpHandle*>(PX_ALLOC(ALIGN_SIZE_16(sizeof(BpHandle)*size), "BpHandle"));
	mActivePairs=reinterpret_cast<BroadPhasePair*>(PX_ALLOC(ALIGN_SIZE_16(sizeof(BroadPhasePair)*size), "BroadPhasePair"));
	mActivePairStates=reinterpret_cast<PxU8*>(PX_ALLOC(ALIGN_SIZE_16(sizeof(PxU8)*size), "BroadPhaseContextSap ActivePairStates"));
	mHashCapacity=size;
	mMinAllowedHashCapacity = size;
	mActivePairsCapacity=size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SapPairManager::release()
{
	PX_FREE(mHashTable);
	PX_FREE(mNext);
	PX_FREE(mActivePairs);
	PX_FREE(mActivePairStates);
	mHashTable				= NULL;
	mNext					= NULL;
	mActivePairs			= NULL;
	mActivePairStates		= NULL;
	mNext					= 0;
	mHashSize				= 0;
	mHashCapacity			= 0;
	mMinAllowedHashCapacity	= 0;
	mNbActivePairs			= 0;
	mActivePairsCapacity	= 0;
	mMask					= 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const BroadPhasePair* SapPairManager::FindPair(BpHandle id0, BpHandle id1) const
{
	if(0==mHashSize) return NULL;	// Nothing has been allocated yet

	// Order the ids
	Sort(id0, id1);

	// Compute hash value for this pair
	PxU32 HashValue = Hash(id0, id1) & mMask;
	PX_ASSERT(HashValue<mHashCapacity);

	// Look for it in the table
	PX_ASSERT(HashValue<mHashCapacity);
	PxU32 Offset = mHashTable[HashValue];
	PX_ASSERT(BP_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	while(Offset!=BP_INVALID_BP_HANDLE && DifferentPair(mActivePairs[Offset], id0, id1))
	{
		PX_ASSERT(mActivePairs[Offset].mVolA!=BP_INVALID_BP_HANDLE);
		PX_ASSERT(Offset<mHashCapacity);
		Offset = mNext[Offset];		// Better to have a separate array for this
		PX_ASSERT(BP_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	}
	if(Offset==BP_INVALID_BP_HANDLE)	return NULL;
	PX_ASSERT(Offset<mNbActivePairs);
	// Match mActivePairs[Offset] => the pair is persistent
	PX_ASSERT(Offset<mActivePairsCapacity);
	return &mActivePairs[Offset];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Internal version saving hash computation
PX_FORCE_INLINE BroadPhasePair* SapPairManager::FindPair(BpHandle id0, BpHandle id1, PxU32 hash_value) const
{
	if(0==mHashSize) return NULL;	// Nothing has been allocated yet

	// Look for it in the table
	PX_ASSERT(hash_value<mHashCapacity);
	PxU32 Offset = mHashTable[hash_value];
	PX_ASSERT(BP_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	while(Offset!=BP_INVALID_BP_HANDLE && DifferentPair(mActivePairs[Offset], id0, id1))
	{
		PX_ASSERT(mActivePairs[Offset].mVolA!=BP_INVALID_BP_HANDLE);
		PX_ASSERT(Offset<mHashCapacity);
		Offset = mNext[Offset];		// Better to have a separate array for this
		PX_ASSERT(BP_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	}
	if(Offset==BP_INVALID_BP_HANDLE)	return NULL;
	PX_ASSERT(Offset<mNbActivePairs);
	// Match mActivePairs[Offset] => the pair is persistent
	PX_ASSERT(Offset<mActivePairsCapacity);
	return &mActivePairs[Offset];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const BroadPhasePair* SapPairManager::AddPair(BpHandle id0, BpHandle id1, const PxU8 state)
{
	if(MAX_BP_HANDLE == mNbActivePairs)
	{
		PX_WARN_ONCE(MAX_BP_PAIRS_MESSAGE);
		return NULL;
	}

	// Order the ids
	Sort(id0, id1);

	PxU32 HashValue = Hash(id0, id1) & mMask;

	BroadPhasePair* P = FindPair(id0, id1, HashValue);
	if(P)
	{
		return P;	// Persistent pair
	}

	// This is a new pair
	if(mNbActivePairs >= mHashSize)
	{
		// Get more entries
		mHashSize = Ps::nextPowerOfTwo(mNbActivePairs+1);
		mMask = mHashSize-1;

		reallocPairs(mHashSize>mHashCapacity);

		// Recompute hash value with new hash size
		HashValue = Hash(id0, id1) & mMask;
	}

	PX_ASSERT(mNbActivePairs<mActivePairsCapacity);
	BroadPhasePair* p = &mActivePairs[mNbActivePairs];
	p->mVolA		= id0;	// ### CMOVs would be nice here
	p->mVolB		= id1;
	mActivePairStates[mNbActivePairs]=state;

	PX_ASSERT(mNbActivePairs<mHashSize);
	PX_ASSERT(mNbActivePairs<mHashCapacity);
	PX_ASSERT(HashValue<mHashCapacity);
	mNext[mNbActivePairs] = mHashTable[HashValue];
	mHashTable[HashValue] = BpHandle(mNbActivePairs++);
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SapPairManager::RemovePair(BpHandle /*id0*/, BpHandle /*id1*/, PxU32 hash_value, PxU32 pair_index)
{
	// Walk the hash table to fix mNext
	{
		PX_ASSERT(hash_value<mHashCapacity);
		PxU32 Offset = mHashTable[hash_value];
		PX_ASSERT(Offset!=BP_INVALID_BP_HANDLE);

		PxU32 Previous=BP_INVALID_BP_HANDLE;
		while(Offset!=pair_index)
		{
			Previous = Offset;
			PX_ASSERT(Offset<mHashCapacity);
			Offset = mNext[Offset];
		}

		// Let us go/jump us
		if(Previous!=BP_INVALID_BP_HANDLE)
		{
			PX_ASSERT(Previous<mHashCapacity);
			PX_ASSERT(pair_index<mHashCapacity);
			PX_ASSERT(mNext[Previous]==pair_index);
			mNext[Previous] = mNext[pair_index];
		}
		// else we were the first
		else
		{
			PX_ASSERT(hash_value<mHashCapacity);
			PX_ASSERT(pair_index<mHashCapacity);
			mHashTable[hash_value] = mNext[pair_index];
		}
	}
	// we're now free to reuse mNext[PairIndex] without breaking the list

#if PX_DEBUG
	PX_ASSERT(pair_index<mHashCapacity);
	mNext[pair_index]=BP_INVALID_BP_HANDLE;
#endif
	// Invalidate entry

	// Fill holes
	{
		// 1) Remove last pair
		const PxU32 LastPairIndex = mNbActivePairs-1;
		if(LastPairIndex==pair_index)
		{
			mNbActivePairs--;
		}
		else
		{
			PX_ASSERT(LastPairIndex<mActivePairsCapacity);
			const BroadPhasePair* Last = &mActivePairs[LastPairIndex];
			const PxU32 LastHashValue = Hash(Last->mVolA, Last->mVolB) & mMask;

			// Walk the hash table to fix mNext
			PX_ASSERT(LastHashValue<mHashCapacity);
			PxU32 Offset = mHashTable[LastHashValue];
			PX_ASSERT(Offset!=BP_INVALID_BP_HANDLE);

			PxU32 Previous=BP_INVALID_BP_HANDLE;
			while(Offset!=LastPairIndex)
			{
				Previous = Offset;
				PX_ASSERT(Offset<mHashCapacity);
				Offset = mNext[Offset];
			}

			// Let us go/jump us
			if(Previous!=BP_INVALID_BP_HANDLE)
			{
				PX_ASSERT(Previous<mHashCapacity);
				PX_ASSERT(LastPairIndex<mHashCapacity);
				PX_ASSERT(mNext[Previous]==LastPairIndex);
				mNext[Previous] = mNext[LastPairIndex];
			}
			// else we were the first
			else
			{
				PX_ASSERT(LastHashValue<mHashCapacity);
				PX_ASSERT(LastPairIndex<mHashCapacity);
				mHashTable[LastHashValue] = mNext[LastPairIndex];
			}
			// we're now free to reuse mNext[LastPairIndex] without breaking the list

#if PX_DEBUG
			PX_ASSERT(LastPairIndex<mHashCapacity);
			mNext[LastPairIndex]=BP_INVALID_BP_HANDLE;
#endif

			// Don't invalidate entry since we're going to shrink the array

			// 2) Re-insert in free slot
			PX_ASSERT(pair_index<mActivePairsCapacity);
			PX_ASSERT(LastPairIndex<mActivePairsCapacity);
			mActivePairs[pair_index] = mActivePairs[LastPairIndex];
			mActivePairStates[pair_index] = mActivePairStates[LastPairIndex];
#if PX_DEBUG
			PX_ASSERT(pair_index<mHashCapacity);
			PX_ASSERT(mNext[pair_index]==BP_INVALID_BP_HANDLE);
#endif
			PX_ASSERT(pair_index<mHashCapacity);
			PX_ASSERT(LastHashValue<mHashCapacity);
			mNext[pair_index] = mHashTable[LastHashValue];
			mHashTable[LastHashValue] = BpHandle(pair_index);

			mNbActivePairs--;
		}
	}
}

bool SapPairManager::RemovePair(BpHandle id0, BpHandle id1)
{
	// Order the ids
	Sort(id0, id1);

	const PxU32 HashValue = Hash(id0, id1) & mMask;
	const BroadPhasePair* P = FindPair(id0, id1, HashValue);
	if(!P)	return false;
	PX_ASSERT(P->mVolA==id0);
	PX_ASSERT(P->mVolB==id1);

	RemovePair(id0, id1, HashValue, GetPairIndex(P));

	shrinkMemory();

	return true;
}

bool SapPairManager::RemovePairs(const Cm::BitMap& removedAABBs)
{
	PxU32 i=0;
	while(i<mNbActivePairs)
	{
		const BpHandle id0 = mActivePairs[i].mVolA;
		const BpHandle id1 = mActivePairs[i].mVolB;
		if(removedAABBs.test(id0) || removedAABBs.test(id1))
		{
			const PxU32 HashValue = Hash(id0, id1) & mMask;
			RemovePair(id0, id1, HashValue, i);
		}
		else i++;
	}
	return true;
}

void SapPairManager::shrinkMemory()
{
	//Compute the hash size given the current number of active pairs.
	const PxU32 correctHashSize = Ps::nextPowerOfTwo(mNbActivePairs);

	//If we have the correct hash size then no action required.
	if(correctHashSize==mHashSize || (correctHashSize < mMinAllowedHashCapacity && mHashSize == mMinAllowedHashCapacity))	
		return;

	//The hash size can be reduced so take action.
	//Don't let the hash size fall below a threshold value.
	PxU32 newHashSize = correctHashSize;
	if(newHashSize < mMinAllowedHashCapacity)
	{
		newHashSize = mMinAllowedHashCapacity;
	}
	mHashSize = newHashSize;
	mMask = newHashSize-1;

	reallocPairs( (newHashSize > mMinAllowedHashCapacity) || (mHashSize <= (mHashCapacity >> 2)) || (mHashSize <= (mActivePairsCapacity >> 2)));
}

void SapPairManager::reallocPairs(const bool allocRequired)
{
	if(allocRequired)
	{
		PX_FREE(mHashTable);
		mHashCapacity=mHashSize;
		mActivePairsCapacity=mHashSize;
		mHashTable = reinterpret_cast<BpHandle*>(PX_ALLOC(mHashSize*sizeof(BpHandle), "BpHandle"));

		for(PxU32 i=0;i<mHashSize;i++)	
		{
			mHashTable[i] = BP_INVALID_BP_HANDLE;
		}

		// Get some bytes for new entries
		BroadPhasePair* NewPairs	= reinterpret_cast<BroadPhasePair*>(PX_ALLOC(mHashSize * sizeof(BroadPhasePair), "BroadPhasePair"));	PX_ASSERT(NewPairs);
		BpHandle* NewNext			= reinterpret_cast<BpHandle*>(PX_ALLOC(mHashSize * sizeof(BpHandle), "BpHandle"));						PX_ASSERT(NewNext);
		PxU8* NewPairStates			= reinterpret_cast<PxU8*>(PX_ALLOC(mHashSize * sizeof(PxU8), "SapPairStates"));							PX_ASSERT(NewPairStates);

		// Copy old data if needed
		if(mNbActivePairs) 
		{
			PxMemCopy(NewPairs, mActivePairs, mNbActivePairs*sizeof(BroadPhasePair));
			PxMemCopy(NewPairStates, mActivePairStates, mNbActivePairs*sizeof(PxU8));
		}

		// ### check it's actually needed... probably only for pairs whose hash value was cut by the and
		// yeah, since Hash(id0, id1) is a constant
		// However it might not be needed to recompute them => only less efficient but still ok
		for(PxU32 i=0;i<mNbActivePairs;i++)
		{
			const PxU32 HashValue = Hash(mActivePairs[i].mVolA, mActivePairs[i].mVolB) & mMask;	// New hash value with new mask
			NewNext[i] = mHashTable[HashValue];
			PX_ASSERT(HashValue<mHashCapacity);
			mHashTable[HashValue] = BpHandle(i);
		}

		// Delete old data
		PX_FREE(mNext);
		PX_FREE(mActivePairs);
		PX_FREE(mActivePairStates);

		// Assign new pointer
		mActivePairs = NewPairs;
		mActivePairStates = NewPairStates;
		mNext = NewNext;
	}
	else
	{
		for(PxU32 i=0;i<mHashSize;i++)	
		{
			mHashTable[i] = BP_INVALID_BP_HANDLE;
		}

		// ### check it's actually needed... probably only for pairs whose hash value was cut by the and
		// yeah, since Hash(id0, id1) is a constant
		// However it might not be needed to recompute them => only less efficient but still ok
		for(PxU32 i=0;i<mNbActivePairs;i++)
		{
			const PxU32 HashValue = Hash(mActivePairs[i].mVolA, mActivePairs[i].mVolB) & mMask;	// New hash value with new mask
			mNext[i] = mHashTable[HashValue];
			PX_ASSERT(HashValue<mHashCapacity);
			mHashTable[HashValue] = BpHandle(i);
		}
	}
}

void resizeCreatedDeleted(BroadPhasePair*& pairs, PxU32& maxNumPairs)
{
	PX_ASSERT(pairs);
	PX_ASSERT(maxNumPairs>0);
	const PxU32 newMaxNumPairs=2*maxNumPairs;
	BroadPhasePair* newPairs=reinterpret_cast<BroadPhasePair*>(PX_ALLOC(sizeof(BroadPhasePair)*newMaxNumPairs, "BroadPhasePair"));
	PxMemCopy(newPairs, pairs, sizeof(BroadPhasePair)*maxNumPairs);
	PX_FREE(pairs);
	pairs=newPairs;
	maxNumPairs=newMaxNumPairs;
}

void ComputeCreatedDeletedPairsLists
(const BpHandle* PX_RESTRICT boxGroups, 
 const BpHandle* PX_RESTRICT dataArray, const PxU32 dataArraySize,
 PxcScratchAllocator* scratchAllocator,
 BroadPhasePairReport*& createdPairsList, PxU32& numCreatedPairs, PxU32& maxNumCreatedPairs,
 BroadPhasePairReport*& deletedPairsList, PxU32& numDeletedPairs, PxU32& maxNumDeletedPairs,
 PxU32& numActualDeletedPairs,
 SapPairManager& pairManager)
{
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
	PX_UNUSED(boxGroups);
#endif

	for(PxU32 i=0;i<dataArraySize;i++)
	{
		const PxU32 ID = dataArray[i];
		PX_ASSERT(ID<pairManager.mNbActivePairs);

		const BroadPhasePair* PX_RESTRICT UP = pairManager.mActivePairs + ID;
		PX_ASSERT(pairManager.IsInArray(UP));

		if(pairManager.IsRemoved(UP))
		{
			if(!pairManager.IsNew(UP))
			{
				// No need to call "ClearInArray" in this case, since the pair will get removed anyway
				if(numDeletedPairs==maxNumDeletedPairs)
				{
					BroadPhasePairReport* newDeletedPairsList = reinterpret_cast<BroadPhasePairReport*>(scratchAllocator->alloc(sizeof(BroadPhasePairReport)*2*maxNumDeletedPairs, true));
					PxMemCopy(newDeletedPairsList, deletedPairsList, sizeof(BroadPhasePairReport)*maxNumDeletedPairs);
					scratchAllocator->free(deletedPairsList);
					deletedPairsList = newDeletedPairsList;
					maxNumDeletedPairs = 2*maxNumDeletedPairs;
				}

				PX_ASSERT(numDeletedPairs<maxNumDeletedPairs);
				//PX_ASSERT((uintptr_t)UP->mUserData != 0xcdcdcdcd);
				deletedPairsList[numDeletedPairs]=BroadPhasePairReport(UP->mVolA,UP->mVolB, UP->mUserData, ID);
				numDeletedPairs++;
			}
		}
		else
		{
			pairManager.ClearInArray(UP);
			// Add => already there... Might want to create user data, though
			if(pairManager.IsNew(UP))
			{
#if !BP_SAP_TEST_GROUP_ID_CREATEUPDATE
				if(boxGroups[UP->mVolA]!=boxGroups[UP->mVolB])
#endif
				{
					if(numCreatedPairs==maxNumCreatedPairs)
					{
						BroadPhasePairReport* newCreatedPairsList = reinterpret_cast<BroadPhasePairReport*>(scratchAllocator->alloc(sizeof(BroadPhasePairReport)*2*maxNumCreatedPairs, true));
						PxMemCopy(newCreatedPairsList, createdPairsList, sizeof(BroadPhasePairReport)*maxNumCreatedPairs);
						scratchAllocator->free(createdPairsList);
						createdPairsList = newCreatedPairsList;
						maxNumCreatedPairs = 2*maxNumCreatedPairs;
					}

					PX_ASSERT(numCreatedPairs<maxNumCreatedPairs);
					createdPairsList[numCreatedPairs]=BroadPhasePairReport(UP->mVolA,UP->mVolB, UP->mUserData, ID);
					numCreatedPairs++;
				}
				pairManager.ClearNew(UP);
			}
		}
	}

	//Record pairs that are to be deleted because they were simultaneously created and removed 
	//from different axis sorts.
	numActualDeletedPairs=numDeletedPairs;
	for(PxU32 i=0;i<dataArraySize;i++)
	{
		const PxU32 ID = dataArray[i];
		PX_ASSERT(ID<pairManager.mNbActivePairs);
		const BroadPhasePair* PX_RESTRICT UP = pairManager.mActivePairs + ID;
		if(pairManager.IsRemoved(UP) && pairManager.IsNew(UP))
		{
			PX_ASSERT(pairManager.IsInArray(UP));

			if(numActualDeletedPairs==maxNumDeletedPairs)
			{
				BroadPhasePairReport* newDeletedPairsList = reinterpret_cast<BroadPhasePairReport*>(scratchAllocator->alloc(sizeof(BroadPhasePairReport)*2*maxNumDeletedPairs, true));
				PxMemCopy(newDeletedPairsList, deletedPairsList, sizeof(BroadPhasePairReport)*maxNumDeletedPairs);
				scratchAllocator->free(deletedPairsList);
				deletedPairsList = newDeletedPairsList;
				maxNumDeletedPairs = 2*maxNumDeletedPairs;
			}

			PX_ASSERT(numActualDeletedPairs<=maxNumDeletedPairs);
			deletedPairsList[numActualDeletedPairs]=BroadPhasePairReport(UP->mVolA,UP->mVolB, NULL, ID); //KS - should we even get here????
			numActualDeletedPairs++;
		}
	}

//	// #### try batch removal here
//	for(PxU32 i=0;i<numActualDeletedPairs;i++)
//	{
//		const BpHandle id0 = deletedPairsList[i].mVolA;
//		const BpHandle id1 = deletedPairsList[i].mVolB;
//#if PX_DEBUG
//		const bool Status = pairManager.RemovePair(id0, id1);
//		PX_ASSERT(Status);
//#else
//		pairManager.RemovePair(id0, id1);
//#endif
//	}

	//Only report deleted pairs from different groups.
#if !BP_SAP_TEST_GROUP_ID_CREATEUPDATE
	for(PxU32 i=0;i<numDeletedPairs;i++)
	{
		const PxU32 id0 = deletedPairsList[i].mVolA;
		const PxU32 id1 = deletedPairsList[i].mVolB;
		if(boxGroups[id0]==boxGroups[id1])
		{
			while((numDeletedPairs-1) > i && boxGroups[deletedPairsList[numDeletedPairs-1].mVolA] == boxGroups[deletedPairsList[numDeletedPairs-1].mVolB])
			{
				numDeletedPairs--;
			}
			deletedPairsList[i]=deletedPairsList[numDeletedPairs-1];
			numDeletedPairs--;
		}
	}
#endif
}

void DeletePairsLists(const PxU32 numActualDeletedPairs, BroadPhasePairReport* deletedPairsList, SapPairManager& pairManager)
{
	// #### try batch removal here
	for(PxU32 i=0;i<numActualDeletedPairs;i++)
	{
		const BpHandle id0 = deletedPairsList[i].mVolA;
		const BpHandle id1 = deletedPairsList[i].mVolB;
#if PX_DEBUG
		const bool Status = pairManager.RemovePair(id0, id1);
		PX_ASSERT(Status);
#else
		pairManager.RemovePair(id0, id1);
#endif
	}
}

#include "BpBroadPhase.h"
PX_COMPILE_TIME_ASSERT(FilterGroup::eSTATICS==0);
void ComputeSortedLists
(Cm::BitMap* PX_RESTRICT bitmap, 
 const PxU32 insertAABBStart, const PxU32 insertAABBEnd, const BpHandle* PX_RESTRICT createdAABBs,
 SapBox1D** PX_RESTRICT asapBoxes, const BpHandle* PX_RESTRICT asapBoxGroupIds, 
 BpHandle* PX_RESTRICT asapEndPointDatas, const PxU32 numSortedEndPoints, 
 const Gu::Axes& axes,
 BpHandle* PX_RESTRICT newBoxIndicesSorted, PxU32& newBoxIndicesCount, BpHandle* PX_RESTRICT oldBoxIndicesSorted, PxU32& oldBoxIndicesCount,
 bool& allNewBoxesStatics, bool& allOldBoxesStatics)
{
	const PxU32 axis0=axes.mAxis0;
	const PxU32 axis1=axes.mAxis1;
	const PxU32 axis2=axes.mAxis2;

	//Set the bitmap for new box ids and compute the aabb (of the sorted handles/indices and not of the values) that bounds all new boxes.
	
	PxU32 globalAABBMinX=PX_MAX_U32;
	PxU32 globalAABBMinY=PX_MAX_U32;
	PxU32 globalAABBMinZ=PX_MAX_U32;
	PxU32 globalAABBMaxX=0;
	PxU32 globalAABBMaxY=0;
	PxU32 globalAABBMaxZ=0;
	
	for(PxU32 i=insertAABBStart;i<insertAABBEnd;i++)
	{
		const PxU32 boxId=createdAABBs[i];
		bitmap->set(boxId);

		globalAABBMinX = PxMin(globalAABBMinX, PxU32(asapBoxes[axis0][boxId].mMinMax[0]));
		globalAABBMinY = PxMin(globalAABBMinY, PxU32(asapBoxes[axis1][boxId].mMinMax[0]));
		globalAABBMinZ = PxMin(globalAABBMinZ, PxU32(asapBoxes[axis2][boxId].mMinMax[0]));
		globalAABBMaxX = PxMax(globalAABBMaxX, PxU32(asapBoxes[axis0][boxId].mMinMax[1]));
		globalAABBMaxY = PxMax(globalAABBMaxY, PxU32(asapBoxes[axis1][boxId].mMinMax[1]));
		globalAABBMaxZ = PxMax(globalAABBMaxZ, PxU32(asapBoxes[axis2][boxId].mMinMax[1]));
	}

	PxU32 oldStaticCount=0;
	PxU32 newStaticCount=0;

	//Assign the sorted end pts to the appropriate arrays.
	for(PxU32 i=1;i<numSortedEndPoints-1;i++)
	{
		//Make sure we haven't encountered a sentinel - 
		//they should only be at each end of the array.
		PX_ASSERT(!isSentinel(asapEndPointDatas[i]));
		PX_ASSERT(!isSentinel(asapEndPointDatas[i]));
		PX_ASSERT(!isSentinel(asapEndPointDatas[i]));

		if(!isMax(asapEndPointDatas[i]))
		{
			const BpHandle boxId=BpHandle(getOwner(asapEndPointDatas[i]));
			if(!bitmap->test(boxId))
			{
				if(Intersect3D(
					globalAABBMinX, globalAABBMaxX, globalAABBMinY, globalAABBMaxY, globalAABBMinZ, globalAABBMaxZ,
					asapBoxes[axis0][boxId].mMinMax[0],asapBoxes[axis0][boxId].mMinMax[1],asapBoxes[axis1][boxId].mMinMax[0],asapBoxes[axis1][boxId].mMinMax[1],asapBoxes[axis2][boxId].mMinMax[0],asapBoxes[axis2][boxId].mMinMax[1]))
				{
					oldBoxIndicesSorted[oldBoxIndicesCount]=boxId;
					oldBoxIndicesCount++;
					oldStaticCount+=asapBoxGroupIds[boxId];
				}
			}
			else 
			{
				newBoxIndicesSorted[newBoxIndicesCount]=boxId;
				newBoxIndicesCount++;
				newStaticCount+=asapBoxGroupIds[boxId];
			}
		}
	}

	allOldBoxesStatics = oldStaticCount ? false : true;
	allNewBoxesStatics = newStaticCount ? false : true;

	//Make sure that we've found the correct number of boxes.
	PX_ASSERT(newBoxIndicesCount==(insertAABBEnd-insertAABBStart));
	PX_ASSERT(oldBoxIndicesCount<=((numSortedEndPoints-NUM_SENTINELS)/2));
}

void performBoxPruningNewNew
(const Gu::Axes& axes,
 const BpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount, const bool allNewBoxesStatics,
 BpHandle* PX_RESTRICT minPosList0,
 SapBox1D** PX_RESTRICT asapBoxes, const BpHandle* PX_RESTRICT asapBoxGroupIds, 
 PxcScratchAllocator* scratchAllocator,
 SapPairManager& pairManager, BpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	// Checkings
	if(!newBoxIndicesCount)	return;

	// Catch axes
	const PxU32 Axis0 = axes.mAxis0;
	const PxU32 Axis1 = axes.mAxis1;
	const PxU32 Axis2 = axes.mAxis2;

	// 1) Build main list using the primary axis
	for(PxU32 i=0;i<newBoxIndicesCount;i++)	
	{
		const BpHandle boxId = newBoxIndicesSorted[i];
		minPosList0[i] = asapBoxes[Axis0][boxId].mMinMax[0];
	}

	if(allNewBoxesStatics) return;

	// 2) Prune the list

	const PxU32 LastSortedIndex = newBoxIndicesCount;
	PxU32 RunningIndex = 0;
	PxU32 SortedIndex = 0;

	while(RunningIndex<LastSortedIndex && SortedIndex<LastSortedIndex)
	{
		const PxU32 Index0 = SortedIndex++;
		const BpHandle boxId0 = newBoxIndicesSorted[Index0];
		const BpHandle Limit = asapBoxes[Axis0][boxId0].mMinMax[1];

		while(RunningIndex<LastSortedIndex && minPosList0[RunningIndex++]<minPosList0[Index0]);

		if(RunningIndex<LastSortedIndex)
		{
			PxU32 RunningIndex2 = RunningIndex;

			PxU32 Index1;
			while(RunningIndex2<LastSortedIndex && minPosList0[Index1 = RunningIndex2++] <= Limit)
			{
				const BpHandle boxId1 = newBoxIndicesSorted[Index1];
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
				if(asapBoxGroupIds[boxId0]!=asapBoxGroupIds[boxId1])
#endif
				{
					if(Intersect2D(
						asapBoxes[Axis1][boxId0].mMinMax[0],asapBoxes[Axis1][boxId0].mMinMax[1],asapBoxes[Axis2][boxId0].mMinMax[0],asapBoxes[Axis2][boxId0].mMinMax[1],
						asapBoxes[Axis1][boxId1].mMinMax[0],asapBoxes[Axis1][boxId1].mMinMax[1],asapBoxes[Axis2][boxId1].mMinMax[0],asapBoxes[Axis2][boxId1].mMinMax[1]))
					{
						AddPair(boxId0, boxId1, scratchAllocator, pairManager, dataArray, dataArraySize, dataArrayCapacity);
					}
				}
			}
		}
	}
}

void performBoxPruningNewOld
(const Gu::Axes& axes,
 const BpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount, const BpHandle* PX_RESTRICT oldBoxIndicesSorted, const PxU32 oldBoxIndicesCount,
 BpHandle* PX_RESTRICT minPosListNew,  BpHandle* PX_RESTRICT minPosListOld,
 SapBox1D** PX_RESTRICT asapBoxes, const BpHandle* PX_RESTRICT asapBoxGroupIds,
 PxcScratchAllocator* scratchAllocator, 
 SapPairManager& pairManager, BpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	// Checkings
	if(!newBoxIndicesCount || !oldBoxIndicesCount)	return;

	// Catch axes
	const PxU32 Axis0 = axes.mAxis0;
	const PxU32 Axis1 = axes.mAxis1;
	const PxU32 Axis2 = axes.mAxis2;

	BpHandle* PX_RESTRICT minPosList0=minPosListNew;
	BpHandle* PX_RESTRICT minPosList1=minPosListOld;

	// 1) Build main lists using the primary axis
	for(PxU32 i=0;i<newBoxIndicesCount;i++)	
	{
		const BpHandle boxId=newBoxIndicesSorted[i];
		minPosList0[i] = asapBoxes[Axis0][boxId].mMinMax[0];
	}
	for(PxU32 i=0;i<oldBoxIndicesCount;i++)	
	{
		const BpHandle boxId=oldBoxIndicesSorted[i];
		minPosList1[i] = asapBoxes[Axis0][boxId].mMinMax[0];
	}

	// 3) Prune the lists
	const PxU32 LastSortedIndex0 = newBoxIndicesCount;
	const PxU32 LastSortedIndex1 = oldBoxIndicesCount;
	PxU32 RunningIndex0 = 0;
	PxU32 RunningIndex1 = 0;
	PxU32 SortedIndex1 = 0;
	PxU32 SortedIndex0 = 0;


	while(RunningIndex1<LastSortedIndex1 && SortedIndex0<LastSortedIndex0)
	{
		const PxU32 Index0 = SortedIndex0++;
		const BpHandle boxId0 = newBoxIndicesSorted[Index0];
		const BpHandle Limit = asapBoxes[Axis0][boxId0].mMinMax[1];//Box0.mMaxIndex[Axis0];

		while(RunningIndex1<LastSortedIndex1 && minPosList1[RunningIndex1]<minPosList0[Index0])
			RunningIndex1++;

		PxU32 RunningIndex2_1 = RunningIndex1;

		PxU32 Index1;
		while(RunningIndex2_1<LastSortedIndex1 && minPosList1[Index1 = RunningIndex2_1++] <= Limit)
		{
			const BpHandle boxId1 = oldBoxIndicesSorted[Index1];
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
			if(asapBoxGroupIds[boxId0]!=asapBoxGroupIds[boxId1])
#endif
			{
				if(Intersect2D(
					asapBoxes[Axis1][boxId0].mMinMax[0],asapBoxes[Axis1][boxId0].mMinMax[1],asapBoxes[Axis2][boxId0].mMinMax[0],asapBoxes[Axis2][boxId0].mMinMax[1],
					asapBoxes[Axis1][boxId1].mMinMax[0],asapBoxes[Axis1][boxId1].mMinMax[1],asapBoxes[Axis2][boxId1].mMinMax[0],asapBoxes[Axis2][boxId1].mMinMax[1]))
				{
					AddPair(boxId0, boxId1, scratchAllocator, pairManager, dataArray, dataArraySize, dataArrayCapacity);
				}
			}
		}
	}

	////

	while(RunningIndex0<LastSortedIndex0 && SortedIndex1<LastSortedIndex1)
	{
		const PxU32 Index0 = SortedIndex1++;
		const BpHandle boxId0 = oldBoxIndicesSorted[Index0];
		const BpHandle Limit = asapBoxes[Axis0][boxId0].mMinMax[1];

		while(RunningIndex0<LastSortedIndex0 && minPosList0[RunningIndex0]<=minPosList1[Index0])
			RunningIndex0++;

		PxU32 RunningIndex2_0 = RunningIndex0;

		PxU32 Index1;
		while(RunningIndex2_0<LastSortedIndex0 && minPosList0[Index1 = RunningIndex2_0++] <= Limit)
		{
			const BpHandle boxId1 = newBoxIndicesSorted[Index1];
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
			if(asapBoxGroupIds[boxId0]!=asapBoxGroupIds[boxId1])
#endif
			{
				if(Intersect2D(
					asapBoxes[Axis1][boxId0].mMinMax[0],asapBoxes[Axis1][boxId0].mMinMax[1],asapBoxes[Axis2][boxId0].mMinMax[0],asapBoxes[Axis2][boxId0].mMinMax[1],
					asapBoxes[Axis1][boxId1].mMinMax[0],asapBoxes[Axis1][boxId1].mMinMax[1],asapBoxes[Axis2][boxId1].mMinMax[0],asapBoxes[Axis2][boxId1].mMinMax[1]))
				{
					AddPair(boxId0, boxId1, scratchAllocator, pairManager, dataArray, dataArraySize, dataArrayCapacity);
				}
			}
		}
	}
}

} //namespace Bp

} //namespace physx

