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

#ifndef GU_SWEEP_TESTS_H
#define GU_SWEEP_TESTS_H

#include "CmPhysXCommon.h"
#include "PxQueryReport.h"
#include "PxGeometry.h"

namespace physx
{
	class PxConvexMeshGeometry;
	class PxCapsuleGeometry;
	class PxTriangle;
	class PxBoxGeometry;

	// PT: TODO: unify this with raycast calls (names and order of params)

	// PT: we use defines to be able to quickly change the signature of all sweep functions.
	// (this also ensures they all use consistent names for passed parameters).
	// \param[in]	geom		geometry object to sweep against
	// \param[in]	pose		pose of geometry object
	// \param[in]	unitDir		sweep's unit dir
	// \param[in]	distance	sweep's length/max distance
	// \param[out]	sweepHit	hit result
	// \param[in]	hitFlags	query behavior flags
	// \param[in]	inflation	optional inflation value for swept shape

	// PT: sweep parameters for capsule
	#define GU_CAPSULE_SWEEP_FUNC_PARAMS	const PxGeometry& geom, const PxTransform& pose,												\
											const PxCapsuleGeometry& capsuleGeom_, const PxTransform& capsulePose_, const Gu::Capsule& lss,	\
											const PxVec3& unitDir, PxReal distance,															\
											PxSweepHit& sweepHit, const PxHitFlags hitFlags, PxReal inflation 

	// PT: sweep parameters for box
	#define GU_BOX_SWEEP_FUNC_PARAMS	const PxGeometry& geom, const PxTransform& pose,								\
										const PxBoxGeometry& boxGeom_, const PxTransform& boxPose_, const Gu::Box& box,	\
										const PxVec3& unitDir, PxReal distance,											\
										PxSweepHit& sweepHit, const PxHitFlags hitFlags, PxReal inflation 

	// PT: sweep parameters for convex
	#define GU_CONVEX_SWEEP_FUNC_PARAMS		const PxGeometry& geom, const PxTransform& pose,						\
											const PxConvexMeshGeometry& convexGeom, const PxTransform& convexPose,	\
											const PxVec3& unitDir, PxReal distance,									\
											PxSweepHit& sweepHit, const PxHitFlags hitFlags, PxReal inflation
	namespace Gu
	{
		class Capsule;
		class Box;

		// PT: function pointer for Geom-indexed capsule sweep functions
		// See GU_CAPSULE_SWEEP_FUNC_PARAMS for function parameters details.
		// \return		true if a hit was found, false otherwise
		typedef bool (*SweepCapsuleFunc)	(GU_CAPSULE_SWEEP_FUNC_PARAMS);

		// PT: function pointer for Geom-indexed box sweep functions
		// See GU_BOX_SWEEP_FUNC_PARAMS for function parameters details.
		// \return		true if a hit was found, false otherwise
		typedef bool (*SweepBoxFunc)		(GU_BOX_SWEEP_FUNC_PARAMS);

		// PT: function pointer for Geom-indexed box sweep functions
		// See GU_CONVEX_SWEEP_FUNC_PARAMS for function parameters details.
		// \return		true if a hit was found, false otherwise
		typedef bool (*SweepConvexFunc)		(GU_CONVEX_SWEEP_FUNC_PARAMS);

		// PT: typedef for bundles of all sweep functions, i.e. the function tables themselves (indexed by geom-type).
		typedef SweepCapsuleFunc	GeomSweepCapsuleTable	[PxGeometryType::eGEOMETRY_COUNT];
		typedef SweepBoxFunc		GeomSweepBoxTable		[PxGeometryType::eGEOMETRY_COUNT];
		typedef SweepConvexFunc		GeomSweepConvexTable	[PxGeometryType::eGEOMETRY_COUNT];

		struct GeomSweepFuncs
		{
			GeomSweepCapsuleTable	capsuleMap;
			GeomSweepCapsuleTable	preciseCapsuleMap;
			GeomSweepBoxTable		boxMap;
			GeomSweepBoxTable		preciseBoxMap;
			GeomSweepConvexTable	convexMap;
		};
		// PT: grabs all sweep function tables at once (for access by external non-Gu modules)
		PX_PHYSX_COMMON_API const GeomSweepFuncs& getSweepFuncTable();

		// PT: signature for sweep-vs-triangles functions.
		// We use defines to be able to quickly change the signature of all sweep functions.
		// (this also ensures they all use consistent names for passed parameters).
		// \param[in]	nbTris			number of triangles in input array
		// \param[in]	triangles		array of triangles to sweep the shape against
		// \param[in]	doubleSided		true if input triangles are double-sided
		// \param[in]	x				geom to sweep against input triangles
		// \param[in]	pose			pose of geom x
		// \param[in]	unitDir			sweep's unit dir
		// \param[in]	distance		sweep's length/max distance
		// \param[out]	hit				hit result
		// \param[in]	cachedIndex		optional initial triangle index (must be <nbTris)
		// \param[in]	inflation		optional inflation value for swept shape
		// \param[in]	hitFlags		query behavior flags
		#define GU_SWEEP_TRIANGLES_FUNC_PARAMS(x)	PxU32 nbTris, const PxTriangle* triangles, bool doubleSided,	\
													const x& geom, const PxTransform& pose,							\
													const PxVec3& unitDir, const PxReal distance,					\
													PxSweepHit& hit, const PxU32* cachedIndex,						\
													const PxReal inflation, PxHitFlags hitFlags

		bool sweepCapsuleTriangles		(GU_SWEEP_TRIANGLES_FUNC_PARAMS(PxCapsuleGeometry));
		bool sweepBoxTriangles			(GU_SWEEP_TRIANGLES_FUNC_PARAMS(PxBoxGeometry));
		bool sweepBoxTriangles_Precise	(GU_SWEEP_TRIANGLES_FUNC_PARAMS(PxBoxGeometry));

	}  // namespace Gu
}

#endif
