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

#include "GuBV4.h"
#include "GuSweepSphereTriangle.h"
using namespace physx;
using namespace Gu;

#if PX_INTEL_FAMILY  && !defined(PX_SIMD_DISABLED)

#include "PsVecMath.h"
using namespace physx::shdfnd::aos;

#include "GuBV4_Common.h"
#include "GuInternal.h"

#define SWEEP_AABB_IMPL

	// PT: TODO: refactor structure (TA34704)
	struct RayParams
	{
	#ifdef GU_BV4_QUANTIZED_TREE
		BV4_ALIGN16(Vec3p	mCenterOrMinCoeff_PaddedAligned);
		BV4_ALIGN16(Vec3p	mExtentsOrMaxCoeff_PaddedAligned);
	#endif
	#ifndef GU_BV4_USE_SLABS
		BV4_ALIGN16(Vec3p	mData2_PaddedAligned);
		BV4_ALIGN16(Vec3p	mFDir_PaddedAligned);
		BV4_ALIGN16(Vec3p	mData_PaddedAligned);
		BV4_ALIGN16(Vec3p	mLocalDir_PaddedAligned);
	#endif
		BV4_ALIGN16(Vec3p	mOrigin_Padded);		// PT: TODO: this one could be switched to PaddedAligned & V4LoadA (TA34704)
	};

#include "GuBV4_BoxSweep_Params.h"

namespace
{
	struct CapsuleSweepParams : BoxSweepParams
	{
		Capsule	mLocalCapsule;
		PxVec3	mCapsuleCenter;
		PxVec3	mExtrusionDir;
		PxU32	mEarlyExit;
		float	mBestAlignmentValue;
		float	mBestDistance;
		float	mMaxDist;
	};
}

#include "GuBV4_CapsuleSweep_Internal.h"
#include "GuBV4_Internal.h"

#include "GuBV4_AABBAABBSweepTest.h"
#ifdef GU_BV4_USE_SLABS
	#include "GuBV4_Slabs.h"
#endif
#include "GuBV4_ProcessStreamOrdered_SegmentAABB_Inflated.h"
#include "GuBV4_ProcessStreamNoOrder_SegmentAABB_Inflated.h"
#ifdef GU_BV4_USE_SLABS
	#include "GuBV4_Slabs_KajiyaNoOrder.h"
	#include "GuBV4_Slabs_KajiyaOrdered.h"
#endif

Ps::IntBool BV4_CapsuleSweepSingleAA(const Capsule& capsule, const PxVec3& dir, float maxDist, const BV4Tree& tree, SweepHit* PX_RESTRICT hit, PxU32 flags)
{
	const SourceMesh* PX_RESTRICT mesh = tree.mMeshInterface;

	CapsuleSweepParams Params;
	setupCapsuleParams(&Params, capsule, dir, maxDist, &tree, mesh, flags);

	if(tree.mNodes)
	{
		if(Params.mEarlyExit)
			processStreamRayNoOrder(1, LeafFunction_CapsuleSweepAny)(tree.mNodes, tree.mInitData, &Params);
		else
			processStreamRayOrdered(1, LeafFunction_CapsuleSweepClosest)(tree.mNodes, tree.mInitData, &Params);
	}
	else
		doBruteForceTests<LeafFunction_CapsuleSweepAny, LeafFunction_CapsuleSweepClosest>(mesh->getNbTriangles(), &Params);

	return computeImpactDataT<ImpactFunctionCapsule>(capsule, dir, hit, &Params, NULL, (flags & QUERY_MODIFIER_DOUBLE_SIDED)!=0, (flags & QUERY_MODIFIER_MESH_BOTH_SIDES)!=0);
}

#endif
