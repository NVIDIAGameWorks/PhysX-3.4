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


#ifndef PX_PHYSICS_SCP_CONTACTREPORTBUFFER
#define PX_PHYSICS_SCP_CONTACTREPORTBUFFER

#include "foundation/Px.h"

namespace physx
{
	namespace Sc
	{
		class ContactReportBuffer
		{
		public:
			PX_FORCE_INLINE ContactReportBuffer(PxU32 initialSize, bool noResizeAllowed)
				: mBuffer(NULL)
				 ,mCurrentBufferIndex(0)
				 ,mCurrentBufferSize(initialSize)
				 ,mDefaultBufferSize(initialSize)
				 ,mLastBufferIndex(0)
				 ,mAllocationLocked(noResizeAllowed)
			{
				mBuffer = allocateBuffer(initialSize);
				PX_ASSERT(mBuffer);
			}

			~ContactReportBuffer()
			{
				PX_FREE(mBuffer);
			}

			PX_FORCE_INLINE void					reset();
			PX_FORCE_INLINE void					flush();

			PX_FORCE_INLINE PxU8*					allocateNotThreadSafe(PxU32 size, PxU32& index, PxU32 alignment= 16);
			PX_FORCE_INLINE PxU8*					reallocateNotThreadSafe(PxU32 size, PxU32& index, PxU32 alignment= 16, PxU32 lastIndex = 0xFFFFFFFF);
			PX_FORCE_INLINE	PxU8*					getData(const PxU32& index) const { return mBuffer+index; }

			PX_FORCE_INLINE PxU32					getDefaultBufferSize() const {return mDefaultBufferSize;}

		private:
			PX_FORCE_INLINE PxU8* allocateBuffer(PxU32 size);

		private:
			PxU8*			mBuffer;
			PxU32			mCurrentBufferIndex;
			PxU32			mCurrentBufferSize;
			PxU32			mDefaultBufferSize;
			PxU32			mLastBufferIndex;
			bool			mAllocationLocked;
		};

	} // namespace Sc

	//////////////////////////////////////////////////////////////////////////

	PX_FORCE_INLINE void Sc::ContactReportBuffer::reset()
	{
		mCurrentBufferIndex = 0;	
		mLastBufferIndex = 0xFFFFFFFF;
	}

	//////////////////////////////////////////////////////////////////////////

	void Sc::ContactReportBuffer::flush()
	{
		mCurrentBufferIndex = 0;
		mLastBufferIndex = 0xFFFFFFFF;

		if(mCurrentBufferSize != mDefaultBufferSize)
		{
			PX_FREE(mBuffer);

			mBuffer = allocateBuffer(mDefaultBufferSize);
			PX_ASSERT(mBuffer);

			mCurrentBufferSize = mDefaultBufferSize;			
		}
	}

	//////////////////////////////////////////////////////////////////////////

	PxU8* Sc::ContactReportBuffer::allocateNotThreadSafe(PxU32 size, PxU32& index ,PxU32 alignment/* =16 */)
	{
		PX_ASSERT(shdfnd::isPowerOfTwo(alignment));
		
		// padding for alignment
		PxU32 pad = ((mCurrentBufferIndex+alignment-1)&~(alignment-1)) - mCurrentBufferIndex;

		index = mCurrentBufferIndex + pad;

		if (index + size > mCurrentBufferSize)
		{			
			if(mAllocationLocked)
				return NULL;

			PxU32 oldBufferSize = mCurrentBufferSize;
			while(index + size > mCurrentBufferSize)
			{
				mCurrentBufferSize *= 2;
			}
			
			PxU8* tempBuffer = allocateBuffer(mCurrentBufferSize);

			PxMemCopy(tempBuffer,mBuffer,oldBufferSize);

			PX_FREE(mBuffer);

			mBuffer = tempBuffer;
		}

		
		PxU8* ptr = mBuffer + index;
		mLastBufferIndex = index;
		PX_ASSERT((reinterpret_cast<size_t>(ptr)&(alignment-1)) == 0);	
		mCurrentBufferIndex += size + pad;
		return ptr;
	}

	//////////////////////////////////////////////////////////////////////////

	PxU8* Sc::ContactReportBuffer::reallocateNotThreadSafe(PxU32 size, PxU32& index ,PxU32 alignment/* =16 */, PxU32 lastIndex)
	{		
		if(lastIndex != mLastBufferIndex)
		{
			return allocateNotThreadSafe(size,index,alignment);
		}
		else
		{
			mCurrentBufferIndex = mLastBufferIndex;
			return allocateNotThreadSafe(size,index,alignment);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	PX_FORCE_INLINE PxU8* Sc::ContactReportBuffer::allocateBuffer(PxU32 size)
	{
		return (static_cast<PxU8*>(PX_ALLOC(size, "ContactReportBuffer")));
	}

} // namespace physx

#endif // PX_PHYSICS_SCP_CONTACTREPORTBUFFER
