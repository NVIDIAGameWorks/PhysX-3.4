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

#include "foundation/PxVec3.h"
#include "foundation/PxMemory.h"
#include "MeshCleaner.h"
#include "PsAllocator.h"
#include "PsBitUtils.h"

#ifndef PX_COOKING
#error Do not include anymore!
#endif

using namespace physx;

struct Indices
{
	PxU32 mRef[3];

	PX_FORCE_INLINE bool operator!=(const Indices&v) const	{ return mRef[0] != v.mRef[0] || mRef[1] != v.mRef[1] || mRef[2] != v.mRef[2]; }
};

static PX_FORCE_INLINE PxU32 getHashValue(const PxVec3& v)
{
	const PxU32* h = reinterpret_cast<const PxU32*>(&v.x);
	const PxU32 f = (h[0]+h[1]*11-(h[2]*17)) & 0x7fffffff;	// avoid problems with +-0
	return (f>>22)^(f>>12)^(f);
}

static PX_FORCE_INLINE PxU32 getHashValue(const Indices& v)
{
//	const PxU32* h = v.mRef;
//	const PxU32 f = (h[0]+h[1]*11-(h[2]*17)) & 0x7fffffff;	// avoid problems with +-0
//	return (f>>22)^(f>>12)^(f);

	PxU32 a = v.mRef[0];
	PxU32 b = v.mRef[1];
	PxU32 c = v.mRef[2];
	a=a-b;  a=a-c;  a=a^(c >> 13);
	b=b-c;  b=b-a;  b=b^(a << 8); 
	c=c-a;  c=c-b;  c=c^(b >> 13);
	a=a-b;  a=a-c;  a=a^(c >> 12);
	b=b-c;  b=b-a;  b=b^(a << 16);
	c=c-a;  c=c-b;  c=c^(b >> 5);
	a=a-b;  a=a-c;  a=a^(c >> 3);
	b=b-c;  b=b-a;  b=b^(a << 10);
	c=c-a;  c=c-b;  c=c^(b >> 15);
	return c;
}

MeshCleaner::MeshCleaner(PxU32 nbVerts, const PxVec3* srcVerts, PxU32 nbTris, const PxU32* srcIndices, PxF32 meshWeldTolerance)
{
	PxVec3* cleanVerts = reinterpret_cast<PxVec3*>(PX_ALLOC(sizeof(PxVec3)*nbVerts, "MeshCleaner"));
	PX_ASSERT(cleanVerts);

	PxU32* indices = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nbTris*3, "MeshCleaner"));

	PxU32* remapTriangles = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nbTris, "MeshCleaner"));

	PxU32* vertexIndices = NULL;
	if(meshWeldTolerance!=0.0f)
	{
		vertexIndices = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nbVerts, "MeshCleaner"));
		const PxF32 weldTolerance = 1.0f / meshWeldTolerance;
		// snap to grid
		for(PxU32 i=0; i<nbVerts; i++)
		{
			vertexIndices[i] = i;
			cleanVerts[i] = PxVec3(	PxFloor(srcVerts[i].x*weldTolerance + 0.5f),
									PxFloor(srcVerts[i].y*weldTolerance + 0.5f),
									PxFloor(srcVerts[i].z*weldTolerance + 0.5f));
		}
	}
	else
	{
		PxMemCopy(cleanVerts, srcVerts, nbVerts*sizeof(PxVec3));
	}

	const PxU32 maxNbElems = PxMax(nbTris, nbVerts);
	const PxU32 hashSize = shdfnd::nextPowerOfTwo(maxNbElems);
	const PxU32 hashMask = hashSize-1;
	PxU32* hashTable = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*(hashSize + maxNbElems), "MeshCleaner"));
	PX_ASSERT(hashTable);
	memset(hashTable, 0xff, hashSize * sizeof(PxU32));
	PxU32* const next = hashTable + hashSize;

	PxU32* remapVerts = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*nbVerts, "MeshCleaner"));
	memset(remapVerts, 0xff, nbVerts * sizeof(PxU32));

	for(PxU32 i=0;i<nbTris*3;i++)
	{
		const PxU32 vref = srcIndices[i];
		if(vref<nbVerts)
			remapVerts[vref] = 0;
	}

	PxU32 nbCleanedVerts = 0;
	for(PxU32 i=0;i<nbVerts;i++)
	{
		if(remapVerts[i]==0xffffffff)
			continue;

		const PxVec3& v = cleanVerts[i];
		const PxU32 hashValue = getHashValue(v) & hashMask;
		PxU32 offset = hashTable[hashValue];

		while(offset!=0xffffffff && cleanVerts[offset]!=v)
			offset = next[offset];

		if(offset==0xffffffff)
		{
			remapVerts[i] = nbCleanedVerts;
			cleanVerts[nbCleanedVerts] = v;
			if(vertexIndices)
				vertexIndices[nbCleanedVerts] = i;
			next[nbCleanedVerts] = hashTable[hashValue];
			hashTable[hashValue] = nbCleanedVerts++;
		}
		else remapVerts[i] = offset;
	}

	PxU32 nbCleanedTris = 0;
	for(PxU32 i=0;i<nbTris;i++)
	{
		PxU32 vref0 = *srcIndices++;
		PxU32 vref1 = *srcIndices++;
		PxU32 vref2 = *srcIndices++;
		if(vref0>=nbVerts || vref1>=nbVerts || vref2>=nbVerts)
			continue;

		// PT: you can still get zero-area faces when the 3 vertices are perfectly aligned
		const PxVec3& p0 = srcVerts[vref0];
		const PxVec3& p1 = srcVerts[vref1];
		const PxVec3& p2 = srcVerts[vref2];
		const float area2 = ((p0 - p1).cross(p0 - p2)).magnitudeSquared();
		if(area2==0.0f)
			continue;

		vref0 = remapVerts[vref0];
		vref1 = remapVerts[vref1];
		vref2 = remapVerts[vref2];
		if(vref0==vref1 || vref1==vref2 || vref2==vref0)
			continue;

		indices[nbCleanedTris*3+0] = vref0;
		indices[nbCleanedTris*3+1] = vref1;
		indices[nbCleanedTris*3+2] = vref2;
		remapTriangles[nbCleanedTris] = i;
		nbCleanedTris++;
	}
	PX_FREE(remapVerts);

	PxU32 nbToGo = nbCleanedTris;
	nbCleanedTris = 0;
	memset(hashTable, 0xff, hashSize * sizeof(PxU32));

	Indices* const I = reinterpret_cast<Indices*>(indices);
	bool idtRemap = true;
	for(PxU32 i=0;i<nbToGo;i++)
	{
		const Indices& v = I[i];
		const PxU32 hashValue = getHashValue(v) & hashMask;
		PxU32 offset = hashTable[hashValue];

		while(offset!=0xffffffff && I[offset]!=v)
			offset = next[offset];

		if(offset==0xffffffff)
		{
			const PxU32 originalIndex = remapTriangles[i];
			PX_ASSERT(nbCleanedTris<=i);
			remapTriangles[nbCleanedTris] = originalIndex;
			if(originalIndex!=nbCleanedTris)
				idtRemap = false;
			I[nbCleanedTris] = v;
			next[nbCleanedTris] = hashTable[hashValue];
			hashTable[hashValue] = nbCleanedTris++;
		}
	}
	PX_FREE(hashTable);

	if(vertexIndices)
	{
		for(PxU32 i=0;i<nbCleanedVerts;i++)
			cleanVerts[i] = srcVerts[vertexIndices[i]];
		PX_FREE(vertexIndices);
	}
	mNbVerts	= nbCleanedVerts;
	mNbTris		= nbCleanedTris;
	mVerts		= cleanVerts;
	mIndices	= indices;
	if(idtRemap)
	{
		PX_FREE(remapTriangles);
		mRemap	= NULL;
	}
	else
	{
		mRemap	= remapTriangles;
	}
}

MeshCleaner::~MeshCleaner()
{
	PX_FREE_AND_RESET(mRemap);
	PX_FREE_AND_RESET(mIndices);
	PX_FREE_AND_RESET(mVerts);
}
