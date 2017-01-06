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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "CmRadixSortBuffered.h"
#include "PsAllocator.h"

using namespace physx;
using namespace Cm;

RadixSortBuffered::RadixSortBuffered()
: RadixSort()
{
}

RadixSortBuffered::~RadixSortBuffered()
{
	// Release everything
	if(mDeleteRanks)
	{
		PX_FREE_AND_RESET(mRanks2);
		PX_FREE_AND_RESET(mRanks);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Resizes the inner lists.
 *	\param		nb	[in] new size (number of dwords)
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RadixSortBuffered::Resize(PxU32 nb)
{
	if(mDeleteRanks)
	{
		// Free previously used ram
		PX_FREE_AND_RESET(mRanks2);
		PX_FREE_AND_RESET(mRanks);

		// Get some fresh one
		mRanks	= reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nb, "RadixSortBuffered:mRanks"));
		mRanks2	= reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nb, "RadixSortBuffered:mRanks2"));
	}

	return true;
}

PX_INLINE void RadixSortBuffered::CheckResize(PxU32 nb)
{
	PxU32 CurSize = CURRENT_SIZE;
	if(nb!=CurSize)
	{
		if(nb>CurSize)	Resize(nb);
		mCurrentSize = nb;
		INVALIDATE_RANKS;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main sort routine.
 *	This one is for integer values. After the call, mRanks contains a list of indices in sorted order, i.e. in the order you may process your data.
 *	\param		input	[in] a list of integer values to sort
 *	\param		nb		[in] number of values to sort, must be < 2^31
 *	\param		hint	[in] RADIX_SIGNED to handle negative values, RADIX_UNSIGNED if you know your input buffer only contains positive values
 *	\return		Self-Reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSortBuffered& RadixSortBuffered::Sort(const PxU32* input, PxU32 nb, RadixHint hint)
{
	// Checkings
	if(!input || !nb || nb&0x80000000)	return *this;

	// Resize lists if needed
	CheckResize(nb);

	//Set histogram buffers.
	PxU32 histogram[1024];
	PxU32* links[256];
	mHistogram1024=histogram;
	mLinks256=links;

	RadixSort::Sort(input,nb,hint);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main sort routine.
 *	This one is for floating-point values. After the call, mRanks contains a list of indices in sorted order, i.e. in the order you may process your data.
 *	\param		input2			[in] a list of floating-point values to sort
 *	\param		nb				[in] number of values to sort, must be < 2^31
 *	\return		Self-Reference
 *	\warning	only sorts IEEE floating-point values
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSortBuffered& RadixSortBuffered::Sort(const float* input2, PxU32 nb)
{
	// Checkings
	if(!input2 || !nb || nb&0x80000000)	return *this;

	// Resize lists if needed
	CheckResize(nb);

	//Set histogram buffers.
	PxU32 histogram[1024];
	PxU32* links[256];
	mHistogram1024=histogram;
	mLinks256=links;

	RadixSort::Sort(input2,nb);
	return *this;
}

