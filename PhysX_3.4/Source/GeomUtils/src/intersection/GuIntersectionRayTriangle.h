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

#ifndef GU_INTERSECTION_RAY_TRIANGLE_H
#define GU_INTERSECTION_RAY_TRIANGLE_H

#include "foundation/PxVec3.h"
#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"

namespace physx
{

namespace Gu
{
	// PT: this is used for backface culling. It existed in Moller's original code already. Basically this is only to avoid dividing by zero.
	// This should not depend on what units are used, and neither should it depend on the size of triangles. A large triangle with the same
	// orientation as a small triangle should be backface culled the same way. A triangle whose orientation does not change should not suddenly
	// become culled or visible when we scale it.
	//
	// An absolute epsilon is fine here. The computation will work fine for small triangles, and large triangles will simply make 'det' larger,
	// more and more inaccurate, but it won't suddenly make it negative.
	//
	// Using FLT_EPSILON^2 ensures that triangles whose edges are smaller than FLT_EPSILON long are rejected. This epsilon makes the code work
	// for very small triangles, while still preventing divisions by too small values.
	#define GU_CULLING_EPSILON_RAY_TRIANGLE FLT_EPSILON*FLT_EPSILON

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Computes a ray-triangle intersection test.
	*	From Tomas Moeller's "Fast Minimum Storage Ray-Triangle Intersection"
	*	Could be optimized and cut into 2 methods (culled or not). Should make a batch one too to avoid the call overhead, or make it inline.
	*
	*	\param		orig	[in] ray origin
	*	\param		dir		[in] ray direction
	*	\param		vert0	[in] triangle vertex
	*	\param		vert1	[in] triangle vertex
	*	\param		vert2	[in] triangle vertex
	*	\param		at		[out] distance
	*	\param		au		[out] impact barycentric coordinate
	*	\param		av		[out] impact barycentric coordinate
	*	\param		cull	[in] true to use backface culling
	*	\param		enlarge [in] enlarge triangle by specified epsilon in UV space to avoid false near-edge rejections
	*	\return		true on overlap
	*	\note		u, v and t will remain unchanged if false is returned.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PX_FORCE_INLINE bool intersectRayTriangle(	const PxVec3& orig, const PxVec3& dir, 
												const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2, 
												PxReal& at, PxReal& au, PxReal& av,
												bool cull, float enlarge=0.0f)
	{
		// Find vectors for two edges sharing vert0
		const PxVec3 edge1 = vert1 - vert0;
		const PxVec3 edge2 = vert2 - vert0;

		// Begin calculating determinant - also used to calculate U parameter
		const PxVec3 pvec = dir.cross(edge2); // error ~ |v2-v0|

		// If determinant is near zero, ray lies in plane of triangle
		const PxReal det = edge1.dot(pvec); // error ~ |v2-v0|*|v1-v0|

		if(cull)
		{
			if(det<GU_CULLING_EPSILON_RAY_TRIANGLE)
				return false;

			// Calculate distance from vert0 to ray origin
			const PxVec3 tvec = orig - vert0;

			// Calculate U parameter and test bounds
			const PxReal u = tvec.dot(pvec);

			const PxReal enlargeCoeff = enlarge*det;
			const PxReal uvlimit = -enlargeCoeff;
			const PxReal uvlimit2 = det + enlargeCoeff;

			if(u<uvlimit || u>uvlimit2)
				return false;

			// Prepare to test V parameter
			const PxVec3 qvec = tvec.cross(edge1);

			// Calculate V parameter and test bounds
			const PxReal v = dir.dot(qvec);
			if(v<uvlimit || (u+v)>uvlimit2)
				return false;

			// Calculate t, scale parameters, ray intersects triangle
			const PxReal t = edge2.dot(qvec);

			const PxReal inv_det = 1.0f / det;
			at = t*inv_det;
			au = u*inv_det;
			av = v*inv_det;
		}
		else
		{
			// the non-culling branch
			if(PxAbs(det)<GU_CULLING_EPSILON_RAY_TRIANGLE)
				return false;

			const PxReal inv_det = 1.0f / det;

			// Calculate distance from vert0 to ray origin
			const PxVec3 tvec = orig - vert0; // error ~ |orig-v0|

			// Calculate U parameter and test bounds
			const PxReal u = tvec.dot(pvec) * inv_det;
			if(u<-enlarge || u>1.0f+enlarge)
				return false;

			// prepare to test V parameter
			const PxVec3 qvec = tvec.cross(edge1);

			// Calculate V parameter and test bounds
			const PxReal v = dir.dot(qvec) * inv_det;
			if(v<-enlarge || (u+v)>1.0f+enlarge)
				return false;

			// Calculate t, ray intersects triangle
			const PxReal t = edge2.dot(qvec) * inv_det;

			at = t;
			au = u;
			av = v;
		}
		return true;
	}

	/*	\note	u, v and t will remain unchanged if false is returned. */
	PX_FORCE_INLINE bool intersectRayTriangleCulling(	const PxVec3& orig, const PxVec3& dir, 
														const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2, 
														PxReal& t, PxReal& u, PxReal& v, 
														float enlarge=0.0f)
	{
		return intersectRayTriangle(orig, dir, vert0, vert1, vert2,  t, u, v, true, enlarge);
	}

	/*	\note	u, v and t will remain unchanged if false is returned. */
	PX_FORCE_INLINE bool intersectRayTriangleNoCulling(	const PxVec3& orig, const PxVec3& dir, 
														const PxVec3& vert0, const PxVec3& vert1, const PxVec3& vert2, 
														PxReal& t, PxReal& u, PxReal& v, 
														float enlarge=0.0f)
	{
		return intersectRayTriangle(orig, dir, vert0, vert1, vert2,  t, u, v, false, enlarge);
	}

} // namespace Gu

}

#endif
