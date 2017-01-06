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

#include "GuContactBuffer.h"
#include "GuDistanceSegmentSegment.h"
#include "GuContactMethodImpl.h"
#include "GuInternal.h"
#include "GuGeometryUnion.h"

using namespace physx;

namespace physx
{
namespace Gu
{
bool contactCapsuleCapsule(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(renderOutput);
	PX_UNUSED(cache);

	const PxCapsuleGeometry& capsuleGeom0 = shape0.get<const PxCapsuleGeometry>();
	const PxCapsuleGeometry& capsuleGeom1 = shape1.get<const PxCapsuleGeometry>();

	// PT: get capsules in local space
	PxVec3 dir[2];
	Segment segment[2];
	{
		const PxVec3 capsuleLocalSegment0 = getCapsuleHalfHeightVector(transform0, capsuleGeom0);
		const PxVec3 capsuleLocalSegment1 = getCapsuleHalfHeightVector(transform1, capsuleGeom1);

		const PxVec3 delta = transform1.p - transform0.p;
		segment[0].p0 = capsuleLocalSegment0;
		segment[0].p1 = -capsuleLocalSegment0;
		dir[0] = -capsuleLocalSegment0*2.0f;
		segment[1].p0 = capsuleLocalSegment1 + delta;
		segment[1].p1 = -capsuleLocalSegment1 + delta;
		dir[1] = -capsuleLocalSegment1*2.0f;
	}

	// PT: compute distance between capsules' segments
	PxReal s,t;
	const PxReal squareDist = distanceSegmentSegmentSquared(segment[0], segment[1], &s, &t);
	const PxReal radiusSum = capsuleGeom0.radius + capsuleGeom1.radius;
	const PxReal inflatedSum = radiusSum + params.mContactDistance;
	const PxReal inflatedSumSquared = inflatedSum*inflatedSum;

	if(squareDist >= inflatedSumSquared)
		return false;

	// PT: TODO: optimize this away
	PxReal segLen[2];
	segLen[0] = dir[0].magnitude();
	segLen[1] = dir[1].magnitude();

	if (segLen[0]) dir[0] *= 1.0f / segLen[0];
	if (segLen[1]) dir[1] *= 1.0f / segLen[1];

	if (PxAbs(dir[0].dot(dir[1])) > 0.9998f)	//almost parallel, ca. 1 degree difference --> generate two contact points at ends
	{
		PxU32 numCons = 0;

		PxReal segLenEps[2];
		segLenEps[0] = segLen[0] * 0.001f;//0.1% error is ok.
		segLenEps[1] = segLen[1] * 0.001f;
			
		//project the two end points of each onto the axis of the other and take those 4 points.
		//we could also generate a single normal at the single closest point, but this would be 'unstable'.

		for (PxU32 destShapeIndex = 0; destShapeIndex < 2; destShapeIndex ++)
		{
			for (PxU32 startEnd = 0; startEnd < 2; startEnd ++)
			{
				const PxU32 srcShapeIndex = 1-destShapeIndex;
				//project start/end of srcShapeIndex onto destShapeIndex.
				PxVec3 pos[2];
				pos[destShapeIndex] = startEnd ? segment[srcShapeIndex].p1 : segment[srcShapeIndex].p0;
				const PxReal p = dir[destShapeIndex].dot(pos[destShapeIndex] - segment[destShapeIndex].p0);
				if (p >= -segLenEps[destShapeIndex] && p <= (segLen[destShapeIndex] + segLenEps[destShapeIndex]))
				{
					pos[srcShapeIndex] = p * dir[destShapeIndex] + segment[destShapeIndex].p0;

					PxVec3 normal = pos[1] - pos[0];

					const PxReal normalLenSq = normal.magnitudeSquared();
					if (normalLenSq > 1e-6f && normalLenSq < inflatedSumSquared)
					{
						const PxReal distance = PxSqrt(normalLenSq);
						normal *= 1.0f/distance;
						PxVec3 point = pos[1] - normal * (srcShapeIndex ? capsuleGeom1 : capsuleGeom0).radius;
						point += transform0.p;
						contactBuffer.contact(point, normal, distance - radiusSum);
						numCons++;
					}					
				}
			}
		}

		if (numCons)	//if we did not have contacts, then we may have the case where they are parallel, but are stacked end to end, in which case the old code will generate good contacts.
			return true;
	}

	// Collision response
	PxVec3 pos1 = segment[0].getPointAt(s);
	PxVec3 pos2 = segment[1].getPointAt(t);

	PxVec3 normal = pos1 - pos2;

	const PxReal normalLenSq = normal.magnitudeSquared();
	if (normalLenSq < 1e-6f)
	{
		// PT: TODO: revisit this. "FW" sounds old.
		// Zero normal -> pick the direction of segment 0.
		// Not always accurate but consistent with FW.
		if (segLen[0] > 1e-6f)
			normal = dir[0];
		else 
			normal = PxVec3(1.0f, 0.0f, 0.0f);
	}
	else
	{
		normal *= PxRecipSqrt(normalLenSq);
	}
	
	pos1 += transform0.p;
	contactBuffer.contact(pos1 - normal * capsuleGeom0.radius, normal, PxSqrt(squareDist) - radiusSum);
	return true;
}
}//Gu
}//physx
