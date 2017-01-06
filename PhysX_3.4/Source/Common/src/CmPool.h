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


#ifndef CM_POOL_H
#define CM_POOL_H

#include "PsSort.h"
#include "PsMutex.h"
#include "PsBasicTemplates.h"

#include "CmBitMap.h"
#include "CmPhysXCommon.h"

namespace physx
{
namespace Cm
{

/*!
Allocator for pools of data structures
Also decodes indices (which can be computed from handles) into objects. To make this
faster, the EltsPerSlab must be a power of two
*/
template <class T, class ArgumentType> 
class PoolList : public Ps::AllocatorTraits<T>::Type
{
	typedef typename Ps::AllocatorTraits<T>::Type Alloc;
	PX_NOCOPY(PoolList)
public:
	PX_INLINE PoolList(const Alloc& alloc, ArgumentType* argument, PxU32 eltsPerSlab, PxU32 maxSlabs)
		: Alloc(alloc),
		mEltsPerSlab(eltsPerSlab), 
		mMaxSlabs(maxSlabs), 
		mSlabCount(0),
		mFreeList(0), 
		mFreeCount(0), 
		mSlabs(reinterpret_cast<T**>(Alloc::allocate(maxSlabs * sizeof(T*), __FILE__, __LINE__))),
		mArgument(argument)
	{
		PX_ASSERT(mEltsPerSlab>0);
		// either maxSlabs = 1 (non-resizable pool), or elts per slab must be a power of two
		PX_ASSERT((maxSlabs==1) || ((maxSlabs < 8192) && (mEltsPerSlab & (mEltsPerSlab-1))) == 0);		
		mLog2EltsPerSlab = 0;

		if(mMaxSlabs>1)
		{
			for(mLog2EltsPerSlab=0; mEltsPerSlab!=PxU32(1<<mLog2EltsPerSlab); mLog2EltsPerSlab++)
				;
		}
	}

	PX_INLINE ~PoolList()
	{
		destroy();
	}

	PX_INLINE void destroy()
	{
		// Run all destructors
		for(PxU32 i=0;i<mSlabCount;i++)
		{
			PX_ASSERT(mSlabs);
			T* slab = mSlabs[i];
			for(PxU32 j=0;j<mEltsPerSlab;j++)
			{
				slab[j].~T();
			}
		}

		//Deallocate
		for(PxU32 i=0;i<mSlabCount;i++)
		{
			PX_FREE(mSlabs[i]);
			mSlabs[i] = NULL;
		}
		mSlabCount = 0;

		if(mFreeList)
			PX_FREE(mFreeList);
		mFreeList = NULL;
		if(mSlabs)
		{
			PX_FREE(mSlabs);
			mSlabs = NULL;
		}
	}

	PxU32 preallocate(const PxU32 nbRequired, T** elements)
	{
		//(1) Allocate and pull out an array of X elements

		PxU32 nbToAllocate = nbRequired > mFreeCount ? nbRequired - mFreeCount : 0;

		PxU32 nbElements = nbRequired - nbToAllocate;

		PxMemCopy(elements, mFreeList + (mFreeCount - nbElements), sizeof(T*) * nbElements);
		//PxU32 originalFreeCount = mFreeCount;
		mFreeCount -= nbElements;

		if (nbToAllocate)
		{
			PX_ASSERT(mFreeCount == 0);

			PxU32 nbSlabs = (nbToAllocate + mEltsPerSlab - 1) / mEltsPerSlab; //The number of slabs we need to allocate...

			if (mSlabCount + nbSlabs >= mMaxSlabs)
				return nbElements; //Return only nbFromFree because we're not going to allocate any slabs. Seriously, we need to nuke this "maxSlabs" stuff ASAP!

			//allocate our slabs...

			PxU32 freeCount = mFreeCount;

			for (PxU32 i = 0; i < nbSlabs; ++i)
			{

				//KS - would be great to allocate this using a single allocation but it will make releasing slabs fail later :(
				T * mAddr = reinterpret_cast<T*>(Alloc::allocate(mEltsPerSlab * sizeof(T), __FILE__, __LINE__));
				if (!mAddr)
					return nbElements; //Allocation failed so only return the set of elements we could allocate from the free list

				mSlabs[mSlabCount++] = mAddr;

				// Make sure the usage bitmap is up-to-size
				if (mUseBitmap.size() < mSlabCount*mEltsPerSlab)
				{
					mUseBitmap.resize(2 * mSlabCount*mEltsPerSlab); //set last element as not used
					if (mFreeList)
						PX_FREE(mFreeList);
					mFreeList = reinterpret_cast<T**>(Alloc::allocate(2 * mSlabCount * mEltsPerSlab * sizeof(T*), __FILE__, __LINE__));
				}

				PxU32 baseIndex = (mSlabCount-1) * mEltsPerSlab;

				//Now add all these to the mFreeList and elements...
				PxI32 idx = PxI32(mEltsPerSlab - 1);

				for (; idx >= PxI32(nbToAllocate); --idx)
				{
					mFreeList[freeCount++] = new(mAddr + idx) T(mArgument, baseIndex + idx);
				}

				PxU32 origElements = nbElements;
				T** writeIdx = elements + nbElements;
				for (; idx >= 0; --idx)
				{
					writeIdx[idx] = new(mAddr + idx) T(mArgument, baseIndex + idx);
					nbElements++;
				}

				nbToAllocate -= (nbElements - origElements);
			}

			mFreeCount = freeCount;
		}
		
		PX_ASSERT(nbElements == nbRequired);

		for (PxU32 a = 0; a < nbElements; ++a)
		{
			mUseBitmap.set(elements[a]->getIndex());
		}

		return nbRequired;
	}

	// TODO: would be nice to add templated construct/destroy methods like ObjectPool

	PX_INLINE T* get()
	{
		if(mFreeCount == 0 && !extend())
			return 0;
		T* element = mFreeList[--mFreeCount];
		mUseBitmap.set(element->getIndex());
		return element;
	}

	PX_INLINE void put(T* element)
	{
		PxU32 i = element->getIndex();
		mUseBitmap.reset(i);
		mFreeList[mFreeCount++] = element;
	}

	/*
		WARNING: Unlike findByIndexFast below, this method is NOT safe to use if another thread 
		is concurrently updating the pool (e.g. through put/get/extend/getIterator), since the
		safety boundedTest uses mSlabCount and mUseBitmap.
	*/
	PX_FORCE_INLINE T* findByIndex(PxU32 index) const
	{
		if(index>=mSlabCount*mEltsPerSlab || !(mUseBitmap.boundedTest(index)))
			return 0;
		return mMaxSlabs==1 ? mSlabs[0]+index : mSlabs[index>>mLog2EltsPerSlab] + (index&(mEltsPerSlab-1));
	}

	/*
		This call is safe to do while other threads update the pool.
	*/
	PX_FORCE_INLINE T* findByIndexFast(PxU32 index) const
	{
		PX_ASSERT(mMaxSlabs != 1);
		return mSlabs[index>>mLog2EltsPerSlab] + (index&(mEltsPerSlab-1));
	}

	bool extend()
	{
		if(mSlabCount == mMaxSlabs)
			return false;
		T * mAddr = reinterpret_cast<T*>(Alloc::allocate(mEltsPerSlab * sizeof(T), __FILE__, __LINE__));
		if(!mAddr)
			return false;
		mSlabs[mSlabCount++] = mAddr;



		// Make sure the usage bitmap is up-to-size
		if(mUseBitmap.size() < mSlabCount*mEltsPerSlab)
		{
			mUseBitmap.resize(2*mSlabCount*mEltsPerSlab); //set last element as not used
			if(mFreeList)
				PX_FREE(mFreeList);
			mFreeList = reinterpret_cast<T**>(Alloc::allocate(2*mSlabCount * mEltsPerSlab * sizeof(T*), __FILE__, __LINE__));
		}
	
		// Add to free list in descending order so that lowest indices get allocated first - 
		// the FW context code currently *relies* on this behavior to grab the zero-index volume
		// which can't be allocated to the user. TODO: fix this

		PxU32 baseIndex = (mSlabCount-1) * mEltsPerSlab;
		PxU32 freeCount = mFreeCount;
		for(PxI32 i=PxI32(mEltsPerSlab-1);i>=0;i--)
			mFreeList[freeCount++] = new(mAddr+i) T(mArgument, baseIndex+ i);

		mFreeCount = freeCount;

		return true;
	}

	PX_INLINE PxU32 getMaxUsedIndex()	const
	{
		return mUseBitmap.findLast();
	}

	PX_INLINE BitMap::Iterator getIterator() const
	{
		return BitMap::Iterator(mUseBitmap);
	}

private:
	const PxU32				mEltsPerSlab;
	const PxU32				mMaxSlabs;
	PxU32					mSlabCount;
	PxU32					mLog2EltsPerSlab;
	T**						mFreeList;
	PxU32					mFreeCount;
	T**						mSlabs;
	ArgumentType*			mArgument;
	BitMap					mUseBitmap;
};


}
}

#endif
