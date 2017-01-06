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

#ifndef GU_RAYCAST_TESTS_H
#define GU_RAYCAST_TESTS_H

#include "CmPhysXCommon.h"
#include "foundation/PxSimpleTypes.h"
#include "PxQueryReport.h"
#include "PxGeometry.h"

namespace physx
{
	// PT: TODO: why is PxHitFlag::eMESH_MULTIPLE used in the ray-vs-hf function, but not in the ray-vs-mesh function?

	// PT: we use a define to be able to quickly change the signature of all raycast functions.
	// (this also ensures they all use consistent names for passed parameters).
	// \param[in]	geom		geometry object to raycast against
	// \param[in]	pose		pose of geometry object
	// \param[in]	rayOrigin	ray's origin
	// \param[in]	rayDir		ray's unit dir
	// \param[in]	maxDist		ray's length/max distance
	// \param[in]	hitFlags	query behavior flags
	// \param[in]	maxHits		max number of hits = size of 'hits' buffer
	// \param[out]	hits		result buffer where to write raycast hits
	#define GU_RAY_FUNC_PARAMS	const PxGeometry& geom, const PxTransform& pose,					\
								const PxVec3& rayOrigin, const PxVec3& rayDir, PxReal maxDist,		\
								PxHitFlags hitFlags, PxU32 maxHits, PxRaycastHit* PX_RESTRICT hits
	namespace Gu
	{
		// PT: function pointer for Geom-indexed raycast functions
		// See GU_RAY_FUNC_PARAMS for function parameters details.
		// \return		number of hits written to 'hits' result buffer
		// \note		there's no mechanism to report overflow. Returned number of hits is just clamped to maxHits.
		typedef PxU32	(*RaycastFunc)		(GU_RAY_FUNC_PARAMS);

		// PT: typedef for a bundle of all raycast functions, i.e. the function table itself (indexed by geom-type).
		typedef RaycastFunc GeomRaycastTable[PxGeometryType::eGEOMETRY_COUNT];

		// PT: retrieves the raycast function table (for access by external non-Gu modules)
		PX_PHYSX_COMMON_API const GeomRaycastTable& getRaycastFuncTable();

	}  // namespace Gu
}

#endif
