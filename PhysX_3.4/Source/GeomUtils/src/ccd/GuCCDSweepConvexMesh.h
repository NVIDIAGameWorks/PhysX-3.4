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


#ifndef GU_CCD_SWEEP_H
#define GU_CCD_SWEEP_H

#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "PsVecTransform.h"
#include "GuGeometryUnion.h"
#include "CmScaling.h"

#define GU_TRIANGLE_SWEEP_METHOD_ARGS				\
	const Gu::GeometryUnion& shape0,				\
	const Gu::GeometryUnion& shape1,				\
	const PxTransform& transform0,					\
	const PxTransform& transform1,					\
	const PxTransform& lastTm0,						\
	const PxTransform& lastTm1,						\
	PxReal restDistance,							\
	PxVec3& worldNormal,							\
	PxVec3& worldPoint,								\
	const Cm::FastVertex2ShapeScaling& meshScaling,	\
	Gu::TriangleV& triangle,						\
	const PxF32 toiEstimate


#define GU_SWEEP_METHOD_ARGS			\
	const Gu::CCDShape& shape0,			\
	const Gu::CCDShape& shape1,			\
	const PxTransform& transform0,		\
	const PxTransform& transform1,		\
	const PxTransform& lastTm0,			\
	const PxTransform& lastTm1,			\
	PxReal restDistance,				\
	PxVec3& worldNormal,				\
	PxVec3& worldPoint,					\
	const PxF32 toiEstimate,			\
	PxU32& outCCDFaceIndex,				\
	const PxReal fastMovingThreshold


#define GU_SWEEP_ESTIMATE_ARGS			\
	const CCDShape& shape0,				\
	const CCDShape& shape1,				\
	const PxTransform& transform0,		\
	const PxTransform& transform1,		\
	const PxTransform& lastTr0,			\
	const PxTransform& lastTr1,			\
	const PxReal restDistance,			\
	const PxReal fastMovingThreshold


#define GU_SWEEP_METHOD_ARGS_UNUSED		\
	const Gu::CCDShape& /*shape0*/,		\
	const Gu::CCDShape& /*shape1*/,		\
	const PxTransform& /*transform0*/,	\
	const PxTransform& /*transform1*/,	\
	const PxTransform& /*lastTm0*/,		\
	const PxTransform& /*lastTm1*/,		\
	PxReal /*restDistance*/,			\
	PxVec3& /*worldNormal*/,			\
	PxVec3& /*worldPoint*/,				\
	const PxF32 /*toiEstimate*/,		\
	PxU32& /*outCCDFaceIndex*/,			\
	const PxReal /*fastMovingThreshold*/

namespace physx
{
namespace Gu
{
	struct CCDShape
	{
		const Gu::GeometryUnion*	mGeometry;
		PxReal						mFastMovingThreshold;		//The CCD threshold for this shape
		PxTransform					mPrevTransform;				//This shape's previous transform
		PxTransform					mCurrentTransform;			//This shape's current transform
		PxVec3						mExtents;					//The extents of this shape's AABB
		PxVec3						mCenter;					//The center of this shape's AABB
		PxU32						mUpdateCount;				//How many times this shape has been updated in the CCD. This is correlated with the CCD body's update count.
	};

	PX_FORCE_INLINE PxF32 sweepAABBAABB(const PxVec3& centerA, const PxVec3& extentsA, const PxVec3& centerB, const PxVec3& extentsB, const PxVec3& trA, const PxVec3& trB)
	{
		//Sweep 2 AABBs against each other, return the TOI when they hit else PX_MAX_REAL if they don't hit
		const PxVec3 cAcB = centerA - centerB;
		const PxVec3 sumExtents = extentsA + extentsB;

		//Initial hit
		if(PxAbs(cAcB.x) <= sumExtents.x && 
			PxAbs(cAcB.y) <= sumExtents.y && 
			PxAbs(cAcB.z) <= sumExtents.z)
			return 0.f;

		//No initial hit - perform the sweep
		const PxVec3 relTr = trB - trA;
		PxF32 tfirst = 0.f;
		PxF32 tlast = 1.f;

		const PxVec3 aMax = centerA + extentsA;
		const PxVec3 aMin = centerA - extentsA;
		const PxVec3 bMax = centerB + extentsB;
		const PxVec3 bMin = centerB - extentsB;

		const PxF32 eps = 1e-6f;

		for(PxU32 a = 0; a < 3; ++a)
		{
			if(relTr[a] < -eps)
			{
				if(bMax[a] < aMin[a]) 
					return PX_MAX_REAL;
				if(aMax[a] < bMin[a]) 
					tfirst = PxMax((aMax[a] - bMin[a])/relTr[a], tfirst);
				if(bMax[a] > aMin[a]) 
					tlast = PxMin((aMin[a] - bMax[a])/relTr[a], tlast);
			}
			else if(relTr[a] > eps)
			{
				if(bMin[a] > aMax[a]) 
					return PX_MAX_REAL;
				if(bMax[a] < aMin[a]) 
					tfirst = PxMax((aMin[a] - bMax[a])/relTr[a], tfirst);
				if(aMax[a] > bMin[a]) 
					tlast = PxMin((aMax[a] - bMin[a])/relTr[a], tlast);
			}
			else
			{
				if(bMax[a] < aMin[a] || bMin[a] > aMax[a])
					return PX_MAX_REAL;
			}

			//No hit
			if(tfirst > tlast)
				return PX_MAX_REAL;
		}
		//There was a hit so return the TOI
		return tfirst; 
	}

	PX_PHYSX_COMMON_API PxReal SweepShapeShape(GU_SWEEP_METHOD_ARGS);

	PX_PHYSX_COMMON_API PxReal SweepEstimateAnyShapeHeightfield(GU_SWEEP_ESTIMATE_ARGS);

	PX_PHYSX_COMMON_API PxReal SweepEstimateAnyShapeMesh(GU_SWEEP_ESTIMATE_ARGS);


}
}
#endif

