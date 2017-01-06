/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code from the article "Radix Sort Revisited".
 *	\file		IceRevisitedRadix.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICERADIXSORT_BASE_H__
#define __ICERADIXSORT_BASE_H__

#include "PxSimpleTypes.h"
#include "ApexUsingNamespace.h"
#include <PsUserAllocated.h>

namespace nvidia
{
namespace fracture
{
namespace base
{

	class RadixSort : public UserAllocated
	{
		public:
		// Constructor/Destructor
								RadixSort();
								virtual ~RadixSort();
		// Sorting methods
				RadixSort&		Sort(const uint32_t* input, uint32_t nb, bool signedvalues=true);
				RadixSort&		Sort(const float* input, uint32_t nb);

		//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
		inline	uint32_t*			GetRanks()			const	{ return mRanks;		}

		//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
		inline	uint32_t*			GetRecyclable()		const	{ return mRanks2;		}

		// Stats
				uint32_t			GetUsedRam()		const;
		//! Returns the total number of calls to the radix sorter.
		inline	uint32_t			GetNbTotalCalls()	const	{ return mTotalCalls;	}
		//! Returns the number of premature exits due to temporal coherence.
		inline uint32_t			GetNbHits()			const	{ return mNbHits;		}

		private:

				uint32_t			mHistogram[256*4];			//!< Counters for each byte
				uint32_t			mOffset[256];				//!< Offsets (nearly a cumulative distribution function)
				uint32_t			mCurrentSize;				//!< Current size of the indices list

				uint32_t           mRanksSize;
				uint32_t*			mRanks;				//!< Two lists, swapped each pass
				uint32_t*			mRanks2;
		// Stats
				uint32_t			mTotalCalls;
				uint32_t			mNbHits;
		// Internal methods
				void			CheckResize(uint32_t nb);
				bool			Resize(uint32_t nb);
	};

}
}
}

#endif // __ICERADIXSORT_H__
#endif
