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
#include "foundation/PxAssert.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "GuHillClimbing.h"
#include "GuBigConvexData2.h"

namespace physx
{

void localSearch(PxU32& id, const PxVec3& dir, const PxVec3* verts, const Gu::BigConvexRawData* val)
{
	// WARNING: there is a problem on x86 with a naive version of this code, where truncation
	// of values from 80 bits to 32 bits as they're stored in memory means that iteratively moving to 
	// an adjacent vertex of greater support can go into an infinite loop. So we use a version which 
	// never visits a vertex twice. Note - this might not be enough for GJK, since local
	// termination of the support function might not be enough to ensure convergence of GJK itself.

	// if we got here, we'd better have vertices and valencies
	PX_ASSERT(verts && val);

	class TinyBitMap
	{
	public:
		PxU32 m[8];
		PX_FORCE_INLINE TinyBitMap()			{ m[0] = m[1] = m[2] = m[3] = m[4] = m[5] = m[6] = m[7] = 0;	}
		PX_FORCE_INLINE void set(PxU8 v)		{ m[v>>5] |= 1<<(v&31);											}
		PX_FORCE_INLINE bool get(PxU8 v) const	{ return (m[v>>5] & 1<<(v&31)) != 0;							}
	};

	TinyBitMap visited;

	const Gu::Valency* Valencies	= val->mValencies;
	const PxU8* Adj					= val->mAdjacentVerts;

	PX_ASSERT(Valencies && Adj);

	// Get the initial value and the initial vertex
	float MaxVal	= dir.dot(verts[id]);
	PxU32 NextVtx	= id;

	do
	{
		PxU16 NbNeighbors = Valencies[NextVtx].mCount;
		const PxU8* Run	= Adj + Valencies[NextVtx].mOffset;
		id = NextVtx;
		while(NbNeighbors--)
		{
			const PxU8 Neighbor = *Run++;

			if(!visited.get(Neighbor))
			{
				visited.set(Neighbor);

				const float CurVal = dir.dot(verts[Neighbor]);

				if(CurVal>MaxVal)
				{
					MaxVal	= CurVal;
					NextVtx	= Neighbor;
				}
			}
		}
	} while(NextVtx!=id);
}

}
