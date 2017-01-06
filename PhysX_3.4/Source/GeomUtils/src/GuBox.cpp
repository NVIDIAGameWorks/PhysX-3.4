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


#include "PsIntrinsics.h"
#include "GuBoxConversion.h"
#include "GuCapsule.h"
#include "GuInternal.h"
#include "CmMatrix34.h"
#include "PsMathUtils.h"

using namespace physx;

void Gu::Box::create(const Gu::Capsule& capsule)
{
	// Box center = center of the two LSS's endpoints
	center = capsule.computeCenter();

	// Box orientation
	const PxVec3 dir = capsule.p1 - capsule.p0;
	const float d = dir.magnitude();
	if(d!=0.0f)
	{
		rot.column0 = dir / d;
		Ps::computeBasis(rot.column0, rot.column1, rot.column2);
	}
	else
		rot = PxMat33(PxIdentity);

	// Box extents
	extents.x = capsule.radius + (d * 0.5f);
	extents.y = capsule.radius;
	extents.z = capsule.radius;
}


/**
Returns edges.
\return		24 indices (12 edges) indexing the list returned by ComputePoints()
*/
const PxU8* Gu::getBoxEdges()
{
	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	static PxU8 Indices[] = {
		0, 1,	1, 2,	2, 3,	3, 0,
		7, 6,	6, 5,	5, 4,	4, 7,
		1, 5,	6, 2,
		3, 7,	4, 0
	};
	return Indices;
}


void Gu::computeOBBPoints(PxVec3* PX_RESTRICT pts, const PxVec3& center, const PxVec3& extents, const PxVec3& base0, const PxVec3& base1, const PxVec3& base2)
{
	PX_ASSERT(pts);

	// "Rotated extents"
	const PxVec3 axis0 = base0 * extents.x;
	const PxVec3 axis1 = base1 * extents.y;
	const PxVec3 axis2 = base2 * extents.z;

	//     7+------+6			0 = ---
	//     /|     /|			1 = +--
	//    / |    / |			2 = ++-
	//   / 4+---/--+5			3 = -+-
	// 3+------+2 /    y   z	4 = --+
	//  | /    | /     |  /		5 = +-+
	//  |/     |/      |/		6 = +++
	// 0+------+1      *---x	7 = -++

	// Original code: 24 vector ops
	/*	pts[0] = box.center - Axis0 - Axis1 - Axis2;
	pts[1] = box.center + Axis0 - Axis1 - Axis2;
	pts[2] = box.center + Axis0 + Axis1 - Axis2;
	pts[3] = box.center - Axis0 + Axis1 - Axis2;
	pts[4] = box.center - Axis0 - Axis1 + Axis2;
	pts[5] = box.center + Axis0 - Axis1 + Axis2;
	pts[6] = box.center + Axis0 + Axis1 + Axis2;
	pts[7] = box.center - Axis0 + Axis1 + Axis2;*/

	// Rewritten: 12 vector ops
	pts[0] = pts[3] = pts[4] = pts[7] = center - axis0;
	pts[1] = pts[2] = pts[5] = pts[6] = center + axis0;

	PxVec3 tmp = axis1 + axis2;
	pts[0] -= tmp;
	pts[1] -= tmp;
	pts[6] += tmp;
	pts[7] += tmp;

	tmp = axis1 - axis2;
	pts[2] += tmp;
	pts[3] += tmp;
	pts[4] -= tmp;
	pts[5] -= tmp;
}

