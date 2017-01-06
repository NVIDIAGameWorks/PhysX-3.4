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

#ifndef PXS_TRANSFORM_CACHE_H
#define PXS_TRANSFORM_CACHE_H

#include "CmPhysXCommon.h"
#include "CmIDPool.h"
#include "CmBitMap.h"
#include "PsUserAllocated.h"
#include "PsAllocator.h"

#define PX_DEFAULT_CACHE_SIZE 512

namespace physx
{
	struct PxsTransformFlag
	{
		enum Flags
		{
			eFROZEN = (1 << 0)
		};
	};

	struct PX_ALIGN_PREFIX(16) PxsCachedTransform
	{
		PxTransform transform;
		PxU32 flags;

		PX_FORCE_INLINE PxU32 isFrozen() const { return flags & PxsTransformFlag::eFROZEN; }
	}
	PX_ALIGN_SUFFIX(16);


	class PxsTransformCache : public Ps::UserAllocated
	{
		typedef PxU32 RefCountType;

	public:
		PxsTransformCache(Ps::VirtualAllocatorCallback& allocatorCallback) : mTransformCache(Ps::VirtualAllocator(&allocatorCallback)), mHasAnythingChanged(true)
		{
			/*mTransformCache.reserve(PX_DEFAULT_CACHE_SIZE);
			mTransformCache.forceSize_Unsafe(PX_DEFAULT_CACHE_SIZE);*/
			mUsedSize = 0;
		}

		void initEntry(PxU32 index)
		{
			PxU32 oldCapacity = mTransformCache.capacity();
			if (index >= oldCapacity)
			{
				PxU32 newCapacity = Ps::nextPowerOfTwo(index);
				mTransformCache.reserve(newCapacity);
				mTransformCache.forceSize_Unsafe(newCapacity);
			}
			mUsedSize = PxMax(mUsedSize, index + 1u);
		}


		PX_FORCE_INLINE void setTransformCache(const PxTransform& transform, const PxU32 flags, const PxU32 index)
		{
			mTransformCache[index].transform = transform;
			mTransformCache[index].flags = flags;
			mHasAnythingChanged = true;
		}

		PX_FORCE_INLINE const PxsCachedTransform& getTransformCache(const PxU32 index) const
		{
			return mTransformCache[index];
		}


		PX_FORCE_INLINE PxsCachedTransform& getTransformCache(const PxU32 index)
		{
			return mTransformCache[index];
		}

		PX_FORCE_INLINE void shiftTransforms(const PxVec3& shift)
		{
			for (PxU32 i = 0; i < mTransformCache.capacity(); i++)
			{
				mTransformCache[i].transform.p += shift;
			}
			mHasAnythingChanged = true;
		}

		PX_FORCE_INLINE PxU32 getTotalSize() const
		{
			return mUsedSize;
		}

		PX_FORCE_INLINE const PxsCachedTransform* getTransforms() const
		{
			return mTransformCache.begin();
		}

		PX_FORCE_INLINE PxsCachedTransform* getTransforms()
		{
			return mTransformCache.begin();
		}

		PX_FORCE_INLINE Ps::Array<PxsCachedTransform, Ps::VirtualAllocator>* getCachedTransformArray()
		{
			return &mTransformCache;
		}

		PX_FORCE_INLINE	void resetChangedState()	{ mHasAnythingChanged = false;	}
		PX_FORCE_INLINE	void setChangedState()		{ mHasAnythingChanged = true;	}
		PX_FORCE_INLINE	bool hasChanged()	const	{ return mHasAnythingChanged;	}

	private:
		Ps::Array<PxsCachedTransform, Ps::VirtualAllocator>	mTransformCache;
		PxU32												mUsedSize;
		bool												mHasAnythingChanged;
	};
}

#endif
