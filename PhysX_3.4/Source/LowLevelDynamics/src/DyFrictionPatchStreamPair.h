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



#ifndef PXC_FRICTIONPATCHPOOL_H
#define PXC_FRICTIONPATCHPOOL_H

#include "foundation/PxSimpleTypes.h"
#include "PxvConfig.h"
#include "PsMutex.h"
#include "PsArray.h"

// Each narrow phase thread has an input stream of friction patches from the
// previous frame and an output stream of friction patches which will be
// saved for next frame. The patches persist for exactly one frame at which
// point they get thrown away.


// There is a stream pair per thread. A contact callback reserves space
// for its friction patches and gets a cookie in return that can stash
// for next frame. Cookies are valid for one frame only.
//
// note that all friction patches reserved are guaranteed to be contiguous;
// this might turn out to be a bit inefficient if we often have a large
// number of friction patches

#include "PxcNpMemBlockPool.h"

namespace physx
{

class FrictionPatchStreamPair
{
public:
	FrictionPatchStreamPair(PxcNpMemBlockPool& blockPool);

	// reserve can fail and return null. Read should never fail
	template<class FrictionPatch>
	FrictionPatch*		reserve(const PxU32 size);

	template<class FrictionPatch>
	const FrictionPatch* findInputPatches(const PxU8* ptr) const;
	void					reset();

	PxcNpMemBlockPool& getBlockPool() { return mBlockPool;}
private:
	PxcNpMemBlockPool&	mBlockPool;
	PxcNpMemBlock*		mBlock;
	PxU32				mUsed;

	FrictionPatchStreamPair& operator=(const FrictionPatchStreamPair&);
};

PX_FORCE_INLINE FrictionPatchStreamPair::FrictionPatchStreamPair(PxcNpMemBlockPool& blockPool):
  mBlockPool(blockPool), mBlock(NULL), mUsed(0)
{
}

PX_FORCE_INLINE void FrictionPatchStreamPair::reset()
{
	mBlock = NULL;
	mUsed = 0;
}

// reserve can fail and return null. Read should never fail
template <class FrictionPatch>
FrictionPatch* FrictionPatchStreamPair::reserve(const PxU32 size)
{
	if(size>PxcNpMemBlock::SIZE)
	{
		return reinterpret_cast<FrictionPatch*>(-1);
	}

	PX_ASSERT(size <= PxcNpMemBlock::SIZE);

	FrictionPatch* ptr = NULL;

	if(mBlock == NULL || mUsed + size > PxcNpMemBlock::SIZE)
	{
		mBlock = mBlockPool.acquireFrictionBlock();
		mUsed = 0;
	}

	if(mBlock)
	{
		ptr = reinterpret_cast<FrictionPatch*>(mBlock->data+mUsed);
		mUsed += size;
	}

	return ptr;
}

template <class FrictionPatch>
const FrictionPatch* FrictionPatchStreamPair::findInputPatches(const PxU8* ptr) const
{
	return reinterpret_cast<const FrictionPatch*>(ptr);
}

}

#endif
