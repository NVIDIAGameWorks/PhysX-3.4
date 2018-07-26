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

#ifndef GU_GEOM_UTILS_INTERNAL_H
#define GU_GEOM_UTILS_INTERNAL_H

#include "CmPhysXCommon.h"
#include "GuCapsule.h"
#include "PxCapsuleGeometry.h"
#include "PxBoxGeometry.h"
#include "PsMathUtils.h"
#include "PsUtilities.h"

#define GU_EPSILON_SAME_DISTANCE 1e-3f

namespace physx
{
namespace Gu
{
	class Box;

	// PT: TODO: now that the Gu files are not exposed to users anymore, we should move back capsule-related functions
	// to GuCapsule.h, etc

	PX_PHYSX_COMMON_API const PxU8*	getBoxEdges();

	PX_PHYSX_COMMON_API void		computeBoxPoints(const PxBounds3& bounds, PxVec3* PX_RESTRICT pts);
	PX_PHYSX_COMMON_API void		computeBoundsAroundVertices(PxBounds3& bounds, PxU32 nbVerts, const PxVec3* PX_RESTRICT verts);

						void		computeBoxAroundCapsule(const Capsule& capsule, Box& box);

						PxPlane		getPlane(const PxTransform& pose);

	PX_FORCE_INLINE		PxVec3		getCapsuleHalfHeightVector(const PxTransform& transform, const PxCapsuleGeometry& capsuleGeom)
									{
										return transform.q.getBasisVector0() * capsuleGeom.halfHeight;
									}

	PX_FORCE_INLINE		void		getCapsuleSegment(const PxTransform& transform, const PxCapsuleGeometry& capsuleGeom, Gu::Segment& segment)
									{
										const PxVec3 tmp = getCapsuleHalfHeightVector(transform, capsuleGeom);
										segment.p0 = transform.p + tmp;
										segment.p1 = transform.p - tmp;
									}

	PX_FORCE_INLINE		void		getCapsule(Gu::Capsule& capsule, const PxCapsuleGeometry& capsuleGeom, const PxTransform& pose)
									{
										getCapsuleSegment(pose, capsuleGeom, capsule);
										capsule.radius = capsuleGeom.radius;
									}

						void		computeSweptBox(Gu::Box& box, const PxVec3& extents, const PxVec3& center, const PxMat33& rot, const PxVec3& unitDir, const PxReal distance);

	/**
	*	PT: computes "alignment value" used to select the "best" triangle in case of identical impact distances (for sweeps).
	*	This simply computes how much a triangle is aligned with a given sweep direction.
	*	Captured in a function to make sure it is always computed correctly, i.e. working for double-sided triangles.
	*
	*	\param		triNormal	[in] triangle's normal
	*	\param		unitDir		[in] sweep direction (normalized)
	*	\return		alignment value in [-1.0f, 0.0f]. -1.0f for fully aligned, 0.0f for fully orthogonal.
	*/
	PX_FORCE_INLINE		PxReal		computeAlignmentValue(const PxVec3& triNormal, const PxVec3& unitDir)
	{
		// PT: initial dot product gives the angle between the two, with "best" triangles getting a +1 or -1 score
		// depending on their winding. We take the absolute value to ignore the impact of winding. We negate the result
		// to make the function compatible with the initial code, which assumed single-sided triangles and expected -1
		// for best triangles.
		return -PxAbs(triNormal.dot(unitDir));
	}

	/**
	*	PT: sweeps: determines if a newly touched triangle is "better" than best one so far.
	*	In this context "better" means either clearly smaller impact distance, or a similar impact
	*	distance but a normal more aligned with the sweep direction.
	*
	*	\param		triImpactDistance	[in] new triangle's impact distance
	*	\param		triAlignmentValue	[in] new triangle's alignment value (as computed by computeAlignmentValue)
	*	\param		bestImpactDistance	[in] current best triangle's impact distance
	*	\param		bestAlignmentValue	[in] current best triangle's alignment value (as computed by computeAlignmentValue)
	*   \param		maxDistance			[in] maximum distance of the query, hit cannot be longer than this maxDistance
	*	\param		distEpsilon			[in] tris have "similar" impact distances if the difference is smaller than 2*distEpsilon
	*	\return		true if new triangle is better
	*/
	PX_FORCE_INLINE		bool		keepTriangle(	float triImpactDistance, float triAlignmentValue,
										float bestImpactDistance, float bestAlignmentValue, float maxDistance,
										float distEpsilon)
	{
		// Reject triangle if further than the maxDistance
		if(triImpactDistance > maxDistance)
			return false;

		// PT: make it a relative epsilon to make sure it still works with large distances
		distEpsilon *= PxMax(1.0f, PxMax(triImpactDistance, bestImpactDistance));

		// If new distance is more than epsilon closer than old distance
		if(triImpactDistance < bestImpactDistance - distEpsilon)
			return true;

		// If new distance is no more than epsilon farther than oldDistance and "face is more opposing than previous"
		if(triImpactDistance < bestImpactDistance+distEpsilon && triAlignmentValue < bestAlignmentValue)
			return true;

		// If alignment value is the same, but the new triangle is closer than the best distance
		if(triAlignmentValue == bestAlignmentValue && triImpactDistance < bestImpactDistance)
			return true;

		// If initial overlap happens, keep the triangle
		if(triImpactDistance == 0.0f)
			return true;

		return false;
	}

	#define StoreBounds(bounds, minV, maxV)	\
		V4StoreU(minV, &bounds.minimum.x);	\
		PX_ALIGN(16, PxVec4) max4;			\
		V4StoreA(maxV, &max4.x);			\
		bounds.maximum = PxVec3(max4.x, max4.y, max4.z);

}  // namespace Gu

}

#endif
