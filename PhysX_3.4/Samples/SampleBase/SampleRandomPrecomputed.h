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

#ifndef SAMPLE_RANDOM_PRECOMPUTED_H
#define SAMPLE_RANDOM_PRECOMPUTED_H

#include "common/PxPhysXCommonConfig.h"
#include "foundation/PxAssert.h"

using namespace physx;

class PhysXSample;

class SampleRandomPrecomputed
{
public:

	SampleRandomPrecomputed(PhysXSample& app);
	~SampleRandomPrecomputed();

	PxF32 getRandom() const {return mPrecomputedRandomSequence[(mPrecomputedRandomSequenceCount++) % PRECOMPUTED_RANDOM_SEQUENCE_SIZE];}
	PxF32 getRandomInRange(const PxF32 a, const PxF32 b)
	{
		PX_ASSERT(b>a);
		return a + (b-a)*getRandom();
	}

private:

	enum
	{
		PRECOMPUTED_RANDOM_SEQUENCE_SIZE=65536
	};
	PxF32* mPrecomputedRandomSequence;
	mutable PxU32 mPrecomputedRandomSequenceCount;
};

#endif //SAMPLE_RANDOM_PRECOMPUTED_H
