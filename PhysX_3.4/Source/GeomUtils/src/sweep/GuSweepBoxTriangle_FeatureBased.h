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

#ifndef GU_SWEEP_BOX_TRIANGLE_FEATURE_BASED_H
#define GU_SWEEP_BOX_TRIANGLE_FEATURE_BASED_H

#include "foundation/PxVec3.h"
#include "foundation/PxPlane.h"
#include "CmPhysXCommon.h"

namespace physx
{
	class PxTriangle;
	
	namespace Gu
	{
		/**
		Sweeps a box against a triangle, using a 'feature-based' approach.

		This is currently only used for computing the box-sweep impact data, in a second pass,
		after the best triangle has been identified using faster approaches (SAT/GJK).

		\warning Returned impact normal is not normalized

		\param tri				[in] the triangle
		\param box				[in] the box
		\param motion			[in] (box) motion vector
		\param oneOverMotion	[in] precomputed inverse of motion vector
		\param hit				[out] impact point
		\param normal			[out] impact normal (warning: not normalized)
		\param d				[in/out] impact distance (please initialize with best current distance)
		\param isDoubleSided	[in] whether triangle is double-sided or not
		\return	true if an impact has been found
		*/
		bool sweepBoxTriangle(	const PxTriangle& tri, const PxBounds3& box,
								const PxVec3& motion, const PxVec3& oneOverMotion,
								PxVec3& hit, PxVec3& normal, PxReal& d, bool isDoubleSided=false);
	} // namespace Gu
}

#endif
