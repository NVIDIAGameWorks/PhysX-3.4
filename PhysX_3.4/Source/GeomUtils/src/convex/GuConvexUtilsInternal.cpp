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

#include "GuConvexUtilsInternal.h"

#include "foundation/PxBounds3.h"
#include "CmScaling.h"
#include "GuBoxConversion.h"
#include "PxConvexMeshGeometry.h"
#include "GuConvexMesh.h"

using namespace physx;
using namespace Gu;

void Gu::computeHullOBB(Box& hullOBB, const PxBounds3& hullAABB, float offset, 
						const Cm::Matrix34& convexPose, 
						const Cm::Matrix34& meshPose, const Cm::FastVertex2ShapeScaling& meshScaling, bool idtScaleMesh)
{
	// transform bounds = mesh space
	Cm::Matrix34 m0to1 = meshPose.transformTranspose(convexPose);

	hullOBB.extents = hullAABB.getExtents() + PxVec3(offset);
	hullOBB.center = m0to1.transform(hullAABB.getCenter());
	hullOBB.rot = m0to1.m;
	
	if(!idtScaleMesh)
		meshScaling.transformQueryBounds(hullOBB.center, hullOBB.extents, hullOBB.rot);
}

void Gu::computeVertexSpaceOBB(Box& dst, const Box& src, const PxTransform& meshPose, const PxMeshScale& meshScale)
{
	// AP scaffold failure in x64 debug in GuConvexUtilsInternal.cpp
	//PX_ASSERT("Performance warning - this path shouldn't execute for identity mesh scale." && !meshScale.isIdentity());

	dst = transform(meshScale.getInverse() * Cm::Matrix34(meshPose.getInverse()), src);
}

void Gu::computeOBBAroundConvex(
	Box& obb, const PxConvexMeshGeometry& convexGeom, const PxConvexMesh* cm, const PxTransform& convexPose)
{
	const CenterExtents& aabb = static_cast<const Gu::ConvexMesh*>(cm)->getLocalBoundsFast();

	if(convexGeom.scale.isIdentity())
	{
		const PxMat33 m(convexPose.q);
		obb = Gu::Box(m.transform(aabb.mCenter) + convexPose.p, aabb.mExtents, m);
	}
	else
	{
		obb = transform(Cm::Matrix34(convexPose) * convexGeom.scale.toMat33(), Box(aabb.mCenter, aabb.mExtents, PxMat33(PxIdentity)));
	}
}
