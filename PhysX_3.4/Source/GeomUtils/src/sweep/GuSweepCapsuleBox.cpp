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

#include "foundation/PxBounds3.h"
#include "foundation/PxTransform.h"
#include "GuSweepCapsuleBox.h"
#include "GuSweepSphereTriangle.h"
#include "GuCapsule.h"
#include "PxTriangle.h"
#include "GuDistanceSegmentBox.h"
#include "GuInternal.h"
#include "PsAlloca.h"
#include "GuSIMDHelpers.h"

using namespace physx;
using namespace Gu;

namespace
{
/**
*	Returns triangles.
*	\return		36 indices (12 triangles) indexing the list returned by ComputePoints()
*/
static const PxU8* getBoxTriangles()
{
	static PxU8 Indices[] = {
		0,2,1,	0,3,2,
		1,6,5,	1,2,6,
		5,7,4,	5,6,7,
		4,3,0,	4,7,3,
		3,6,2,	3,7,6,
		5,0,1,	5,4,0
	};
	return Indices;
}
}

#define OUTPUT_TRI(t, p0, p1, p2){	\
t->verts[0] = p0;					\
t->verts[1] = p1;					\
t->verts[2] = p2;					\
t++;}

#define OUTPUT_TRI2(t, p0, p1, p2, d){		\
t->verts[0] = p0;							\
t->verts[1] = p1;							\
t->verts[2] = p2;							\
t->denormalizedNormal(denormalizedNormal);	\
if((denormalizedNormal.dot(d))>0.0f) {		\
PxVec3 Tmp = t->verts[1];					\
t->verts[1] = t->verts[2];					\
t->verts[2] = Tmp;							\
}											\
t++; *ids++ = i; }

static PxU32 extrudeMesh(	PxU32 nbTris, const PxTriangle* triangles,
							const PxVec3& extrusionDir, PxTriangle* tris, PxU32* ids, const PxVec3& dir)
{
	const PxU32* base = ids;

	for(PxU32 i=0; i<nbTris; i++)
	{
		const PxTriangle& currentTriangle = triangles[i];

		// Create triangle normal
		PxVec3 denormalizedNormal;
		currentTriangle.denormalizedNormal(denormalizedNormal);

		// Backface culling
		const bool culled = (denormalizedNormal.dot(dir)) > 0.0f;
		if(culled)	continue;

		PxVec3 p0 = currentTriangle.verts[0];
		PxVec3 p1 = currentTriangle.verts[1];
		PxVec3 p2 = currentTriangle.verts[2];

		PxVec3 p0b = p0 + extrusionDir;
		PxVec3 p1b = p1 + extrusionDir;
		PxVec3 p2b = p2 + extrusionDir;

		p0 -= extrusionDir;
		p1 -= extrusionDir;
		p2 -= extrusionDir;

		if(denormalizedNormal.dot(extrusionDir) >= 0.0f)	OUTPUT_TRI(tris, p0b, p1b, p2b)
		else												OUTPUT_TRI(tris, p0, p1, p2)
		*ids++ = i;

		// ### it's probably useless to extrude all the shared edges !!!!!
		//if(CurrentFlags & TriangleCollisionFlag::eACTIVE_EDGE12)
		{
			OUTPUT_TRI2(tris, p1, p1b, p2b, dir)
			OUTPUT_TRI2(tris, p1, p2b, p2, dir)
		}
		//if(CurrentFlags & TriangleCollisionFlag::eACTIVE_EDGE20)
		{
			OUTPUT_TRI2(tris, p0, p2, p2b, dir)
			OUTPUT_TRI2(tris, p0, p2b, p0b, dir)
		}
		//if(CurrentFlags & TriangleCollisionFlag::eACTIVE_EDGE01)
		{
			OUTPUT_TRI2(tris, p0b, p1b, p1, dir)
			OUTPUT_TRI2(tris, p0b, p1, p0, dir)
		}
	}
	return PxU32(ids-base);
}

static PxU32 extrudeBox(const PxBounds3& localBox, const PxTransform* world, const PxVec3& extrusionDir, PxTriangle* tris, const PxVec3& dir)
{
	// Handle the box as a mesh

	PxTriangle boxTris[12];

	PxVec3 p[8];
	computeBoxPoints(localBox, p);

	const PxU8* PX_RESTRICT indices = getBoxTriangles();

	for(PxU32 i=0; i<12; i++)
	{
		const PxU8 VRef0 = indices[i*3+0];
		const PxU8 VRef1 = indices[i*3+1];
		const PxU8 VRef2 = indices[i*3+2];

		PxVec3 p0 = p[VRef0];
		PxVec3 p1 = p[VRef1];
		PxVec3 p2 = p[VRef2];
		if(world)
		{
			p0 = world->transform(p0);
			p1 = world->transform(p1);
			p2 = world->transform(p2);
		}

		boxTris[i].verts[0] = p0;
		boxTris[i].verts[1] = p1;
		boxTris[i].verts[2] = p2;
	}
	PxU32 fakeIDs[12*7];
	return extrudeMesh(12, boxTris, extrusionDir, tris, fakeIDs, dir);
}

//
// The problem of testing a swept capsule against a box is transformed into sweeping a sphere (lying at the center
// of the capsule) against the extruded triangles of the box. The box triangles are extruded along the
// capsule segment axis.
//
bool Gu::sweepCapsuleBox(const Capsule& capsule, const PxTransform& boxWorldPose, const PxVec3& boxDim, const PxVec3& dir, PxReal length, PxVec3& hit, PxReal& min_dist, PxVec3& normal, PxHitFlags hitFlags)
{
	if(!(hitFlags & PxHitFlag::eASSUME_NO_INITIAL_OVERLAP))
	{
		// PT: test if shapes initially overlap
		if(distanceSegmentBoxSquared(capsule.p0, capsule.p1, boxWorldPose.p, boxDim, PxMat33Padded(boxWorldPose.q)) < capsule.radius*capsule.radius)
		{
			min_dist	= 0.0f;
			normal		= -dir;
			return true;
		}
	}

	// Extrusion dir = capsule segment
	const PxVec3 extrusionDir = (capsule.p1 - capsule.p0)*0.5f;

	// Extrude box
	PxReal MinDist = length;
	bool Status = false;
	{
		const PxBounds3 aabb(-boxDim, boxDim);

		PX_ALLOCA(triangles, PxTriangle, 12*7);
		const PxU32 nbTris = extrudeBox(aabb, &boxWorldPose, extrusionDir, triangles, dir);
		PX_ASSERT(nbTris<=12*7);

		// Sweep sphere vs extruded box
		PxSweepHit h;	// PT: TODO: ctor!
		PxVec3 bestNormal;
		if(sweepSphereTriangles(nbTris, triangles, capsule.computeCenter(), capsule.radius, dir, length, NULL, h, bestNormal, false, false, false, false))
		{
			hit = h.position;
			MinDist = h.distance;
			normal = h.normal;
			Status = true;
		}
	}

	min_dist = MinDist;
	return Status;
}
