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

#include "foundation/PxMath.h"
#include "CookingUtils.h"
#include "CmRadixSortBuffered.h"
#include "PsAllocator.h"
#include "PsFPU.h"

using namespace physx;
using namespace Cm;

ReducedVertexCloud::ReducedVertexCloud(const PxVec3* verts, PxU32 nb_verts) : mNbRVerts(0), mRVerts(NULL), mXRef(NULL)
{
	mVerts		= verts;
	mNbVerts	= nb_verts;
}

ReducedVertexCloud::~ReducedVertexCloud()
{
	Clean();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Frees used ram.
*	\return		Self-reference
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ReducedVertexCloud& ReducedVertexCloud::Clean()
{
	PX_DELETE_POD(mXRef);
	PX_FREE_AND_RESET(mRVerts);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Reduction method. Use this to create a minimal vertex cloud.
*	\param		rc		[out] result structure
*	\return		true if success
*	\warning	This is not about welding nearby vertices, here we look for real redundant ones.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ReducedVertexCloud::Reduce(REDUCEDCLOUD* rc)
{
	Clean();

	mXRef = PX_NEW(PxU32)[mNbVerts];

	float* f = PX_NEW_TEMP(float)[mNbVerts];

	for(PxU32 i=0;i<mNbVerts;i++)
		f[i] = mVerts[i].x;

	RadixSortBuffered Radix;
	Radix.Sort(reinterpret_cast<const PxU32*>(f), mNbVerts, RADIX_UNSIGNED);

	for(PxU32 i=0;i<mNbVerts;i++)
		f[i] = mVerts[i].y;
	Radix.Sort(reinterpret_cast<const PxU32*>(f), mNbVerts, RADIX_UNSIGNED);

	for(PxU32 i=0;i<mNbVerts;i++)
		f[i] = mVerts[i].z;
	const PxU32* Sorted = Radix.Sort(reinterpret_cast<const PxU32*>(f), mNbVerts, RADIX_UNSIGNED).GetRanks();

	PX_DELETE_POD(f);

	mNbRVerts = 0;
	const PxU32 Junk[] = {PX_INVALID_U32, PX_INVALID_U32, PX_INVALID_U32};
	const PxU32* Previous = Junk;
	mRVerts = reinterpret_cast<PxVec3*>(PX_ALLOC(sizeof(PxVec3) * mNbVerts, "PxVec3"));
	PxU32 Nb = mNbVerts;
	while(Nb--)
	{
		const PxU32 Vertex = *Sorted++;	// Vertex number

		const PxU32* current = reinterpret_cast<const PxU32*>(&mVerts[Vertex]);
		if(current[0]!=Previous[0] || current[1]!=Previous[1] || current[2]!=Previous[2])
			mRVerts[mNbRVerts++] = mVerts[Vertex];

		Previous = current;

		mXRef[Vertex] = mNbRVerts-1;
	}

	if(rc)
	{
		rc->CrossRef	= mXRef;
		rc->NbRVerts	= mNbRVerts;
		rc->RVerts		= mRVerts;
	}
	return true;
}
