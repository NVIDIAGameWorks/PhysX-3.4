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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.


#ifndef PX_COLLISION_DEFS_H
#define PX_COLLISION_DEFS_H

#include "PxPhysXConfig.h"
#include "foundation/PxVec3.h"
#include "foundation/PxMat33.h"
#include "GeomUtils/GuContactPoint.h"
#include "GeomUtils/GuContactBuffer.h"
#include "geometry/PxGeometry.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	/**
	\brief A structure to cache contact information produced by LL contact gen functions.
	*/
	struct PxCache
	{
		PxU8*		mCachedData;			//!< Cached data pointer. Allocated via PxCacheAllocator
		PxU16		mCachedSize;			//!< The total size of the cached data 
		PxU8		mPairData;				//!< Pair data information used and cached internally by some contact gen functions to accelerate performance.
		PxU8		mManifoldFlags;			//!< Manifold flags used to identify the format the cached data is stored in.

		PxCache() : mCachedData(NULL), mCachedSize(0), mPairData(0), mManifoldFlags(0)
		{
		}
	};


	/**
	A callback class to allocate memory to cache information used in contact generation.
	*/
	class PxCacheAllocator
	{
	public:
		/**
		\brief Allocates cache data for contact generation. This data is stored inside PxCache objects. The application can retain and provide this information for future contact generation passes
		for a given pair to improve contact generation performance. It is the application's responsibility to release this memory appropriately. If the memory is released, the application must ensure that
		this memory is no longer referenced by any PxCache objects passed to PxGenerateContacts.
		\param byteSize The size of the allocation in bytes
		\return the newly-allocated memory. The returned address must be 16-byte aligned.
		*/
		virtual PxU8* allocateCacheData(const PxU32 byteSize) = 0;

		virtual ~PxCacheAllocator() {}
	};

#if !PX_DOXYGEN
}
#endif

#endif

