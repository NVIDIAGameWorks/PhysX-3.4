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

#ifndef CM_RADIX_SORT_H
#define CM_RADIX_SORT_H

#include "PxPhysXCommonConfig.h"

namespace physx
{
namespace Cm
{

	enum RadixHint
	{
		RADIX_SIGNED,		//!< Input values are signed
		RADIX_UNSIGNED,		//!< Input values are unsigned

		RADIX_FORCE_DWORD = 0x7fffffff
	};

#define INVALIDATE_RANKS	mCurrentSize|=0x80000000
#define VALIDATE_RANKS		mCurrentSize&=0x7fffffff
#define CURRENT_SIZE		(mCurrentSize&0x7fffffff)
#define INVALID_RANKS		(mCurrentSize&0x80000000)

	class PX_PHYSX_COMMON_API RadixSort
	{
		public:
										RadixSort();
		virtual							~RadixSort();
		// Sorting methods
						RadixSort&		Sort(const PxU32* input, PxU32 nb, RadixHint hint=RADIX_SIGNED);
						RadixSort&		Sort(const float* input, PxU32 nb);

		//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
		PX_FORCE_INLINE	const PxU32*	GetRanks()			const	{ return mRanks;		}

		//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
		PX_FORCE_INLINE	PxU32*			GetRecyclable()		const	{ return mRanks2;		}

		//! Returns the total number of calls to the radix sorter.
		PX_FORCE_INLINE	PxU32			GetNbTotalCalls()	const	{ return mTotalCalls;	}
		//! Returns the number of eraly exits due to temporal coherence.
		PX_FORCE_INLINE	PxU32			GetNbHits()			const	{ return mNbHits;		}

						bool			SetBuffers(PxU32* ranks0, PxU32* ranks1, PxU32* histogram1024, PxU32** links256);
		private:
										RadixSort(const RadixSort& object);
										RadixSort& operator=(const RadixSort& object);
		protected:
						PxU32			mCurrentSize;		//!< Current size of the indices list
						PxU32*			mRanks;				//!< Two lists, swapped each pass
						PxU32*			mRanks2;
						PxU32*			mHistogram1024;
						PxU32**			mLinks256;
		// Stats
						PxU32			mTotalCalls;		//!< Total number of calls to the sort routine
						PxU32			mNbHits;			//!< Number of early exits due to coherence

						// Stack-radix
						bool			mDeleteRanks;		//!<
	};

	#define StackRadixSort(name, ranks0, ranks1)	\
		RadixSort name;								\
		PxU32 histogramBuffer[1024];				\
		PxU32* linksBuffer[256];					\
		name.SetBuffers(ranks0, ranks1, histogramBuffer, linksBuffer);
}

}

#endif // CM_RADIX_SORT_H
