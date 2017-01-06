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



#ifndef PXC_CONSTRAINTBLOCKPOOL_H
#define PXC_CONSTRAINTBLOCKPOOL_H

#include "PxvConfig.h"
#include "PsArray.h"
#include "PsMutex.h"
#include "PxcNpMemBlockPool.h"

namespace physx
{

class PxsConstraintBlockManager
{
public:
	PxsConstraintBlockManager(PxcNpMemBlockPool & blockPool):
		mBlockPool(blockPool)
	{
	}


	PX_FORCE_INLINE	void reset()
	{
		mBlockPool.releaseConstraintBlocks(mTrackingArray);
	}


	PxcNpMemBlockArray			mTrackingArray;
	PxcNpMemBlockPool&			mBlockPool;

private:
	PxsConstraintBlockManager& operator=(const PxsConstraintBlockManager&);
};

class PxcConstraintBlockStream
{
	PX_NOCOPY(PxcConstraintBlockStream)
public:
	PxcConstraintBlockStream(PxcNpMemBlockPool & blockPool):
		mBlockPool(blockPool),
		mBlock(NULL),
		mUsed(0)
	{
	}

	PX_FORCE_INLINE	PxU8* reserve(PxU32 size, PxsConstraintBlockManager& manager)
										{
											size = (size+15)&~15;
											if(size>PxcNpMemBlock::SIZE)
												return mBlockPool.acquireExceptionalConstraintMemory(size);

											if(mBlock == NULL || size+mUsed>PxcNpMemBlock::SIZE)
											{
												mBlock = mBlockPool.acquireConstraintBlock(manager.mTrackingArray);
												PX_ASSERT(0==mBlock || mBlock->data == reinterpret_cast<PxU8*>(mBlock));
												mUsed = size;
												return reinterpret_cast<PxU8*>(mBlock);
											}
											PX_ASSERT(mBlock && mBlock->data == reinterpret_cast<PxU8*>(mBlock));
											PxU8* PX_RESTRICT result = mBlock->data+mUsed;
											mUsed += size;
											return result;
										}

	PX_FORCE_INLINE	void				reset()
										{
											mBlock = NULL;
											mUsed = 0;
										}

	PX_FORCE_INLINE PxcNpMemBlockPool&	getMemBlockPool()
	{
		return mBlockPool;
	}

private:
			PxcNpMemBlockPool&			mBlockPool;
			PxcNpMemBlock*				mBlock;	// current constraint block
			PxU32						mUsed;	// number of bytes used in constraint block
			//Tracking peak allocations
			PxU32						mPeakUsed;
};

class PxcContactBlockStream
{
	PX_NOCOPY(PxcContactBlockStream)
public:
	PxcContactBlockStream(PxcNpMemBlockPool & blockPool):
		mBlockPool(blockPool),
		mBlock(NULL),
		mUsed(0)
	{
	}

	PX_FORCE_INLINE	PxU8* reserve(PxU32 size)
										{
											size = (size+15)&~15;

											if(size>PxcNpMemBlock::SIZE)
												return mBlockPool.acquireExceptionalConstraintMemory(size);

											PX_ASSERT(size <= PxcNpMemBlock::SIZE);

											if(mBlock == NULL || size+mUsed>PxcNpMemBlock::SIZE)
											{
												mBlock = mBlockPool.acquireContactBlock();
												PX_ASSERT(0==mBlock || mBlock->data == reinterpret_cast<PxU8*>(mBlock));
												mUsed = size;
												return reinterpret_cast<PxU8*>(mBlock);
											}
											PX_ASSERT(mBlock && mBlock->data == reinterpret_cast<PxU8*>(mBlock));
											PxU8* PX_RESTRICT result = mBlock->data+mUsed;
											mUsed += size;
											return result;
										}

	PX_FORCE_INLINE	void				reset()
										{
											mBlock = NULL;
											mUsed = 0;
										}

	PX_FORCE_INLINE PxcNpMemBlockPool&	getMemBlockPool()
	{
		return mBlockPool;
	}

private:
			PxcNpMemBlockPool&			mBlockPool;
			PxcNpMemBlock*				mBlock;	// current constraint block
			PxU32						mUsed;	// number of bytes used in constraint block
};

}

#endif
