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

#ifndef PX_TOOLKIT_RANDOM_H
#define PX_TOOLKIT_RANDOM_H

#include "common/PxPhysXCommonConfig.h"
#define TEST_MAX_RAND 0xffff

namespace PxToolkit
{
	using namespace physx;

	class BasicRandom
	{
		public:
									BasicRandom(PxU32 seed=0)	: mRnd(seed)	{}
									~BasicRandom()								{}

		PX_FORCE_INLINE	void		setSeed(PxU32 seed)			{ mRnd = seed;											}
		PX_FORCE_INLINE	PxU32		getCurrentValue()	const	{ return mRnd;											}
						PxU32		randomize()					{ mRnd = mRnd * 2147001325 + 715136305; return mRnd;	}

		PX_FORCE_INLINE	PxU32		rand()						{ return randomize() & 0xffff;							}
		PX_FORCE_INLINE	PxU32		rand32()					{ return randomize() & 0xffffffff;						}

						PxF32		randLegacy(PxF32 a, PxF32 b)
									{
										const PxF32 r = static_cast<PxF32>(rand())/(static_cast<PxF32>(0x7fff)+1.0f);
										return r*(b-a) + a;
									}

						PxI32		randLegacy(PxI32 a, PxI32 b)
									{
										return a + static_cast<PxI32>(rand()%(b-a));
									}

						PxF32		rand(PxF32 a, PxF32 b)
									{
										const PxF32 r = rand32()/(static_cast<PxF32>(0xffffffff));
										return r*(b-a) + a;
									}

						PxI32		rand(PxI32 a, PxI32 b)
									{
										return a + static_cast<PxI32>(rand32()%(b-a));
									}

						PxF32		randomFloat()
									{
										return rand()/(static_cast<PxF32>(0xffff)) - 0.5f;
									}									
						PxF32		randomFloat32()
									{
										return rand32()/(static_cast<PxF32>(0xffffffff)) - 0.5f;
									}

						PxF32		randomFloat32(PxReal a, PxReal b) { return rand32()/PxF32(0xffffffff)*(b-a)+a; }
						void		unitRandomPt(physx::PxVec3& v);	
						void		unitRandomQuat(physx::PxQuat& v);

						PxVec3		unitRandomPt();
						PxQuat		unitRandomQuat();

	private:
						PxU32		mRnd;
	};

	//--------------------------------------
	// Fast, very good random numbers
	//
	// Period = 2^249
	//
	// Kirkpatrick, S., and E. Stoll, 1981; A Very Fast Shift-Register
	//       Sequence Random Number Generator, Journal of Computational Physics,
	// V. 40.
	//
	// Maier, W.L., 1991; A Fast Pseudo Random Number Generator,
	// Dr. Dobb's Journal, May, pp. 152 - 157

	class RandomR250
	{
	public:
		RandomR250(PxI32 s); 

		void	setSeed(PxI32 s);
		PxU32	randI();
		PxReal	randUnit();

		PxReal rand(PxReal lower, PxReal upper)
		{
			return lower + randUnit() * (upper - lower);
		}

	private:
		PxU32	mBuffer[250];
		PxI32	mIndex;
	};

	void SetSeed(PxU32 seed);
	PxU32 Rand();

	PX_INLINE PxF32 Rand(PxF32 a,PxF32 b)
	{
		const PxF32 r = static_cast<PxF32>(Rand())/(static_cast<PxF32>(TEST_MAX_RAND));
		return r*(b-a) + a;
	}

	PX_INLINE PxF32 RandLegacy(PxF32 a,PxF32 b)
	{
		const PxF32 r = static_cast<PxF32>(Rand())/(static_cast<PxF32>(0x7fff)+1.0f);
		return r*(b-a) + a;
	} 

	//returns numbers from [a, b-1] 
	PX_INLINE PxI32 Rand(PxI32 a,PxI32 b)
	{
		return a + static_cast<PxI32>(Rand()%(b-a));
	}
}

#endif
