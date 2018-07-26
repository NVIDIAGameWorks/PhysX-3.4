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

#ifndef BP_BROADPHASE_MBP_COMMON_H
#define BP_BROADPHASE_MBP_COMMON_H

#include "PxPhysXConfig.h"
#include "BpBroadPhaseUpdate.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Bp
{

#define MBP_USE_WORDS
#define MBP_USE_NO_CMP_OVERLAP
#if PX_INTEL_FAMILY && !defined(PX_SIMD_DISABLED)
	#define MBP_SIMD_OVERLAP
#endif

#ifdef MBP_USE_WORDS
	typedef	PxU16	MBP_Index;
#else
	typedef	PxU32	MBP_Index;
#endif
	typedef	PxU32	MBP_ObjectIndex;	// PT: index in mMBP_Objects
	typedef	PxU32	MBP_Handle;			// PT: returned to MBP users, combination of index/flip-flop/static-bit

	struct IAABB : public Ps::UserAllocated
	{
		PX_FORCE_INLINE bool isInside(const IAABB& box) const
		{
			if(box.mMinX>mMinX)	return false;
			if(box.mMinY>mMinY)	return false;
			if(box.mMinZ>mMinZ)	return false;
			if(box.mMaxX<mMaxX)	return false;
			if(box.mMaxY<mMaxY)	return false;
			if(box.mMaxZ<mMaxZ)	return false;
			return true;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersects(const IAABB& a)	const
		{
			if(mMaxX < a.mMinX || a.mMaxX < mMinX
			|| mMaxY < a.mMinY || a.mMaxY < mMinY
			|| mMaxZ < a.mMinZ || a.mMaxZ < mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersectNoTouch(const IAABB& a)	const
		{
			if(mMaxX <= a.mMinX || a.mMaxX <= mMinX
			|| mMaxY <= a.mMinY || a.mMaxY <= mMinY
			|| mMaxZ <= a.mMinZ || a.mMaxZ <= mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}

		PX_FORCE_INLINE	void	initFrom2(const PxBounds3& box)
		{
			const PxU32* PX_RESTRICT binary = reinterpret_cast<const PxU32*>(&box.minimum.x);
			mMinX = encodeFloat(binary[0])>>1;
			mMinY = encodeFloat(binary[1])>>1;
			mMinZ = encodeFloat(binary[2])>>1;
			mMaxX = encodeFloat(binary[3])>>1;
			mMaxY = encodeFloat(binary[4])>>1;
			mMaxZ = encodeFloat(binary[5])>>1;
		}

		PX_FORCE_INLINE	void	decode(PxBounds3& box)	const
		{
			PxU32* PX_RESTRICT binary = reinterpret_cast<PxU32*>(&box.minimum.x);
			binary[0] = decodeFloat(mMinX<<1);
			binary[1] = decodeFloat(mMinY<<1);
			binary[2] = decodeFloat(mMinZ<<1);
			binary[3] = decodeFloat(mMaxX<<1);
			binary[4] = decodeFloat(mMaxY<<1);
			binary[5] = decodeFloat(mMaxZ<<1);
		}

		PX_FORCE_INLINE PxU32	getMin(PxU32 i)	const	{	return (&mMinX)[i];	}
		PX_FORCE_INLINE PxU32	getMax(PxU32 i)	const	{	return (&mMaxX)[i];	}

		PxU32 mMinX;
		PxU32 mMinY;
		PxU32 mMinZ;
		PxU32 mMaxX;
		PxU32 mMaxY;
		PxU32 mMaxZ;
	};

	struct SIMD_AABB : public Ps::UserAllocated
	{
		PX_FORCE_INLINE	void	initFrom(const PxBounds3& box)
		{
			const PxU32* PX_RESTRICT binary = reinterpret_cast<const PxU32*>(&box.minimum.x);
			mMinX = encodeFloat(binary[0]);
			mMinY = encodeFloat(binary[1]);
			mMinZ = encodeFloat(binary[2]);
			mMaxX = encodeFloat(binary[3]);
			mMaxY = encodeFloat(binary[4]);
			mMaxZ = encodeFloat(binary[5]);
		}

		PX_FORCE_INLINE	void	initFrom2(const PxBounds3& box)
		{
			const PxU32* PX_RESTRICT binary = reinterpret_cast<const PxU32*>(&box.minimum.x);
			mMinX = encodeFloat(binary[0])>>1;
			mMinY = encodeFloat(binary[1])>>1;
			mMinZ = encodeFloat(binary[2])>>1;
			mMaxX = encodeFloat(binary[3])>>1;
			mMaxY = encodeFloat(binary[4])>>1;
			mMaxZ = encodeFloat(binary[5])>>1;
		}

		PX_FORCE_INLINE	void	decode(PxBounds3& box)	const
		{
			PxU32* PX_RESTRICT binary = reinterpret_cast<PxU32*>(&box.minimum.x);
			binary[0] = decodeFloat(mMinX<<1);
			binary[1] = decodeFloat(mMinY<<1);
			binary[2] = decodeFloat(mMinZ<<1);
			binary[3] = decodeFloat(mMaxX<<1);
			binary[4] = decodeFloat(mMaxY<<1);
			binary[5] = decodeFloat(mMaxZ<<1);
		}

		PX_FORCE_INLINE bool isInside(const SIMD_AABB& box) const
		{
			if(box.mMinX>mMinX)	return false;
			if(box.mMinY>mMinY)	return false;
			if(box.mMinZ>mMinZ)	return false;
			if(box.mMaxX<mMaxX)	return false;
			if(box.mMaxY<mMaxY)	return false;
			if(box.mMaxZ<mMaxZ)	return false;
			return true;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersects(const SIMD_AABB& a)	const
		{
			if(mMaxX < a.mMinX || a.mMaxX < mMinX
			|| mMaxY < a.mMinY || a.mMaxY < mMinY
			|| mMaxZ < a.mMinZ || a.mMaxZ < mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}

		PX_FORCE_INLINE			Ps::IntBool		intersectNoTouch(const SIMD_AABB& a)	const
		{
			if(mMaxX <= a.mMinX || a.mMaxX <= mMinX
			|| mMaxY <= a.mMinY || a.mMaxY <= mMinY
			|| mMaxZ <= a.mMinZ || a.mMaxZ <= mMinZ
			)
				return Ps::IntFalse;
			return Ps::IntTrue;
		}

		PxU32 mMinX;
		PxU32 mMaxX;
		PxU32 mMinY;
		PxU32 mMinZ;
		PxU32 mMaxY;
		PxU32 mMaxZ;
	};

}
} // namespace physx

#endif // BP_BROADPHASE_MBP_COMMON_H
