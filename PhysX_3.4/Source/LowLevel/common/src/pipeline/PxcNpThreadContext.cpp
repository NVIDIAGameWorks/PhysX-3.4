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

#include "PxcConstraintBlockStream.h"
#include "PxcNpThreadContext.h"

using namespace physx;

PxcNpThreadContext::PxcNpThreadContext(PxcNpContext* params) : 
	mRenderOutput						(params->mRenderBuffer),
	mContactBlockStream					(params->mNpMemBlockPool),
	mNpCacheStreamPair					(params->mNpMemBlockPool),
	mNarrowPhaseParams					(0.0f, params->mMeshContactMargin, params->mToleranceLength),
	mPCM								(false),
	mContactCache						(false),
	mCreateContactStream				(params->mCreateContactStream),
	mCreateAveragePoint					(false),
#if PX_ENABLE_SIM_STATS
	mCompressedCacheSize				(0),
	mNbDiscreteContactPairsWithCacheHits(0),
	mNbDiscreteContactPairsWithContacts	(0),
#endif
	mMaxPatches							(0),
	mTotalCompressedCacheSize			(0),
	mContactStreamPool					(params->mContactStreamPool),
	mPatchStreamPool					(params->mPatchStreamPool),
	mForceAndIndiceStreamPool			(params->mForceAndIndiceStreamPool),
	mMaterialManager(params->mMaterialManager),
	mLocalNewTouchCount					(0), 
	mLocalLostTouchCount				(0),
	mLocalFoundPatchCount				(0),	
	mLocalLostPatchCount				(0)
{
#if PX_ENABLE_SIM_STATS
	clearStats();
#endif
}

PxcNpThreadContext::~PxcNpThreadContext()
{
}

#if PX_ENABLE_SIM_STATS
void PxcNpThreadContext::clearStats()
{
	PxMemSet(mDiscreteContactPairs, 0, sizeof(mDiscreteContactPairs));
	PxMemSet(mModifiedContactPairs, 0, sizeof(mModifiedContactPairs));
	mCompressedCacheSize					= 0;
	mNbDiscreteContactPairsWithCacheHits	= 0;
	mNbDiscreteContactPairsWithContacts		= 0;
}
#endif

void PxcNpThreadContext::reset(PxU32 cmCount)
{
	mContactBlockStream.reset();
	mNpCacheStreamPair.reset();

	mLocalChangeTouch.clear();
	mLocalChangeTouch.resize(cmCount);
	mLocalPatchCountChange.clear();
	mLocalPatchCountChange.resize(cmCount);
	mLocalNewTouchCount = 0;
	mLocalLostTouchCount = 0;
	mLocalFoundPatchCount = 0;
	mLocalLostPatchCount = 0;
}
