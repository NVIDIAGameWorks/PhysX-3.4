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

#ifndef GU_INTERSECTION_TRIANGLE_BOX_H
#define GU_INTERSECTION_TRIANGLE_BOX_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "foundation/PxMat33.h"

namespace physx
{
namespace Gu
{
	class Box;
	class BoxPadded;

	/**
	Tests if a triangle overlaps a box (AABB). This is the reference non-SIMD code.

	\param center	[in] the box center
	\param extents	[in] the box extents
	\param p0		[in] triangle's first point
	\param p1		[in] triangle's second point
	\param p2		[in] triangle's third point
	\return	true if triangle overlaps box
	*/
	PX_PHYSX_COMMON_API Ps::IntBool intersectTriangleBox_ReferenceCode(const PxVec3& center, const PxVec3& extents, const PxVec3& p0, const PxVec3& p1, const PxVec3& p2);

	/**
	Tests if a triangle overlaps a box (AABB). This is the optimized SIMD code.

	WARNING: the function has various SIMD requirements, left to the calling code:
	- function will load 4 bytes after 'center'. Make sure it's safe to load from there.
	- function will load 4 bytes after 'extents'. Make sure it's safe to load from there.
	- function will load 4 bytes after 'p0'. Make sure it's safe to load from there.
	- function will load 4 bytes after 'p1'. Make sure it's safe to load from there.
	- function will load 4 bytes after 'p2'. Make sure it's safe to load from there.
	If you can't guarantee these requirements, please use the non-SIMD reference code instead.

	\param center	[in] the box center. 
	\param extents	[in] the box extents
	\param p0		[in] triangle's first point
	\param p1		[in] triangle's second point
	\param p2		[in] triangle's third point
	\return	true if triangle overlaps box
	*/
	PX_PHYSX_COMMON_API Ps::IntBool intersectTriangleBox_Unsafe(const PxVec3& center, const PxVec3& extents, const PxVec3& p0, const PxVec3& p1, const PxVec3& p2);

	/**
	Tests if a triangle overlaps a box (OBB).

	There are currently no SIMD-related requirements for p0, p1, p2.

	\param box		[in] the box
	\param p0		[in] triangle's first point
	\param p1		[in] triangle's second point
	\param p2		[in] triangle's third point
	\return	true if triangle overlaps box
	*/
	PX_PHYSX_COMMON_API Ps::IntBool intersectTriangleBox(const BoxPadded& box, const PxVec3& p0, const PxVec3& p1, const PxVec3& p2);
} // namespace Gu
}

#endif
