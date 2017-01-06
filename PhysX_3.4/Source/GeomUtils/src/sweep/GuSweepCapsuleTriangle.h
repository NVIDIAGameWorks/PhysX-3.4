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

#ifndef GU_SWEEP_CAPSULE_TRIANGLE_H
#define GU_SWEEP_CAPSULE_TRIANGLE_H

#include "foundation/PxVec3.h"
#include "CmPhysXCommon.h"
#include "PxQueryReport.h"

namespace physx
{
	class PxTriangle;

namespace Gu
{
	class BoxPadded;
	class Capsule;

	/**
	Sweeps a capsule against a set of triangles.

	\param nbTris			[in] number of triangles in input array
	\param triangles		[in] array of input triangles
	\param capsule			[in] the capsule
	\param unitDir			[in] sweep's unit direcion
	\param distance			[in] sweep's length
	\param cachedIndex		[in] cached triangle index, or NULL. Cached triangle will be tested first.
	\param hit				[out] results
	\param triNormalOut		[out] triangle normal
	\param hitFlags			[in] query modifiers
	\param isDoubleSided	[in] true if input triangles are double-sided
	\param cullBox			[in] additional/optional culling box. Triangles not intersecting the box are quickly discarded.
	\warning	if using a cullbox, make sure all triangles can be safely V4Loaded (i.e. allocate 4 more bytes after last triangle)
	\return	true if an impact has been found
	*/
	bool sweepCapsuleTriangles_Precise(	PxU32 nbTris, const PxTriangle* PX_RESTRICT triangles,	// Triangle data
										const Capsule& capsule,									// Capsule data
										const PxVec3& unitDir, const PxReal distance,			// Ray data
										const PxU32* PX_RESTRICT cachedIndex,					// Cache data
										PxSweepHit& hit, PxVec3& triNormalOut,						// Results
										PxHitFlags hitFlags, bool isDoubleSided,				// Query modifiers
										const BoxPadded* cullBox=NULL);							// Cull data

} // namespace Gu

}

#endif
