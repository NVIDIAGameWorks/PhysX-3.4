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

#include "SampleRandomPrecomputed.h"
#include "PhysXSample.h"
#include "SampleAllocator.h"
#include "PxTkStream.h"

using namespace PxToolkit;

#define WRITE_SEQUENCE 0

PX_FORCE_INLINE PxF32 flip(const PxF32* v)
{
	const PxU8* b = (const PxU8*)v;
	PxF32 f;
	PxU8* bf = (PxU8*)&f;
	bf[0] = b[3];
	bf[1] = b[2];
	bf[2] = b[1];
	bf[3] = b[0];
	return f;
}

SampleRandomPrecomputed::SampleRandomPrecomputed(PhysXSample& app)
: mPrecomputedRandomSequence(NULL),
  mPrecomputedRandomSequenceCount(0)
{
	mPrecomputedRandomSequence = (PxF32*)SAMPLE_ALLOC(sizeof(PxF32)*(PRECOMPUTED_RANDOM_SEQUENCE_SIZE+1));

#if WRITE_SEQUENCE
	char buffer[256];
	const char* filename = getSampleOutputDirManager().getFilePath("SampleBaseRandomSequence", buffer, false);
	const PxF32 denom = (1.0f / float(RAND_MAX));
	for(PxU32 i=0;i<PRECOMPUTED_RANDOM_SEQUENCE_SIZE;i++)
	{
		mPrecomputedRandomSequence[i] = float(rand()) * denom;
	}
	mPrecomputedRandomSequence[PRECOMPUTED_RANDOM_SEQUENCE_SIZE]=1.0f;
	PxDefaultFileOutputStream stream(filename);
	stream.write(mPrecomputedRandomSequence,sizeof(PxF32)*(PRECOMPUTED_RANDOM_SEQUENCE_SIZE+1));

#else
	const char* filename = getSampleMediaFilename("SampleBaseRandomSequence");
	PxDefaultFileInputData stream(filename);
	if(!stream.isValid())
		app.fatalError("SampleBaseRandomSequence file not found");
	stream.read(mPrecomputedRandomSequence,sizeof(PxF32)*(PRECOMPUTED_RANDOM_SEQUENCE_SIZE+1));

	const bool mismatch = (1.0f != mPrecomputedRandomSequence[PRECOMPUTED_RANDOM_SEQUENCE_SIZE]);
	if(mismatch)
	{
		for(PxU32 i=0;i<PRECOMPUTED_RANDOM_SEQUENCE_SIZE;i++)
		{
			mPrecomputedRandomSequence[i]=flip(&mPrecomputedRandomSequence[i]);
		}
	}

#endif
}

SampleRandomPrecomputed::~SampleRandomPrecomputed()
{
	SAMPLE_FREE(mPrecomputedRandomSequence);
	mPrecomputedRandomSequenceCount=0;
}
