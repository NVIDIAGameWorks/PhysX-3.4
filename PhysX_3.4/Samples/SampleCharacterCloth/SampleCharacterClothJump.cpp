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

#include "SampleCharacterClothJump.h"

using namespace physx;

static PxF32 gJumpGravity = -25.0f;

SampleCharacterClothJump::SampleCharacterClothJump() :
	mV          (0.0f),
	mTime	(0.0f),
	mJump		(false),
	mFreefall   (false)
{
}

void SampleCharacterClothJump::startJump(PxF32 v0)
{
	if (mJump) return;
	if (mFreefall) return;
	mV = v0;
	mJump = true;
}

void SampleCharacterClothJump::reset()
{
	// reset jump state at most every half a second
	// otherwise we might miss jump events
	if (mTime < 0.5f)
		return;

	mFreefall = false;
	mJump = false;
	mV = 0.0f;
	mTime = 0.0f;
}

void SampleCharacterClothJump::tick(PxF32 dtime)
{
	mTime += dtime;
	mV += gJumpGravity * dtime;
	const PxReal vlimit = -25.0f;
	if (mV < vlimit)
	{
		// limit velocity in freefall so that character does not fall at rocket speed.
		mV = vlimit;
		mFreefall = true;
		mJump = false;
	}
}

PxF32 SampleCharacterClothJump::getDisplacement(PxF32 dtime)
{
	return mV * dtime;
}

