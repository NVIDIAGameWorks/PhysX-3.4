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

#include "PxTkRandom.h"
#include "foundation/PxQuat.h"

using namespace physx;
using namespace PxToolkit;

static RandomR250 gRandomR250(0x95d6739b);

PxVec3 BasicRandom::unitRandomPt()
{
	PxVec3 v;
	do
	{
		v.x = randomFloat();
		v.y = randomFloat();
		v.z = randomFloat();
	}
	while(v.normalize()<1e-6f);
	return v;
}

PxQuat BasicRandom::unitRandomQuat()
{
	PxQuat v;
	do
	{
		v.x = randomFloat();
		v.y = randomFloat();
		v.z = randomFloat();
		v.w = randomFloat();
	}
	while(v.normalize()<1e-6f);

	return v;
}


void BasicRandom::unitRandomPt(PxVec3& v)
{
	v = unitRandomPt();
}
void BasicRandom::unitRandomQuat(PxQuat& v)
{
	v = unitRandomQuat();
}


void PxToolkit::SetSeed(PxU32 seed)
{
	gRandomR250.setSeed(seed);
}

PxU32 PxToolkit::Rand()
{
	return gRandomR250.randI() & TEST_MAX_RAND;
}


RandomR250::RandomR250(PxI32 s)
{
	setSeed(s);
}

void RandomR250::setSeed(PxI32 s)
{
	BasicRandom lcg(s);
	mIndex = 0;

	PxI32 j;
	for (j = 0; j < 250; j++)        // fill r250 buffer with bit values
		mBuffer[j] = lcg.randomize();

	for (j = 0; j < 250; j++)        // set some MSBs to 1
		if ( lcg.randomize() > 0x40000000L )
			mBuffer[j] |= 0x80000000L;

	PxU32 msb  = 0x80000000;           // turn on diagonal bit
	PxU32 mask = 0xffffffff;           // turn off the leftmost bits

	for (j = 0; j < 32; j++)
	{
		const PxI32 k = 7 * j + 3;   // select a word to operate on
		mBuffer[k] &= mask;           // turn off bits left of the diagonal 
		mBuffer[k] |= msb;            // turn on the diagonal bit 
		mask >>= 1;
		msb  >>= 1;
	}
}

PxU32 RandomR250::randI()
{
	PxI32 j;

	// wrap pointer around 
	if ( mIndex >= 147 ) j = mIndex - 147;   
	else                 j = mIndex + 103;

	const PxU32 new_rand = mBuffer[ mIndex ] ^ mBuffer[ j ];
	mBuffer[ mIndex ] = new_rand;

	// increment pointer for next time
	if ( mIndex >= 249 ) mIndex = 0;
	else                 mIndex++;

	return new_rand >> 1;
}

PxReal RandomR250::randUnit()
{
	PxU32 mask = (1<<23)-1;
	return PxF32(randI()&(mask))/PxF32(mask);
}
