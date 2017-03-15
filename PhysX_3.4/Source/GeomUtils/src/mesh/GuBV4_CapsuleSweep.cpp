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

#include "GuSIMDHelpers.h"
#include "GuInternal.h"

#include "GuBV4_BoxOverlap_Internal.h"
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

#include "GuBV4_BoxBoxOverlapTest.h"

#ifdef GU_BV4_USE_SLABS
	#include "GuBV4_Slabs.h"
#endif
#include "GuBV4_ProcessStreamOrdered_OBBOBB.h"
#include "GuBV4_ProcessStreamNoOrder_OBBOBB.h"
#ifdef GU_BV4_USE_SLABS
	#include "GuBV4_Slabs_SwizzledNoOrder.h"
	#include "GuBV4_Slabs_SwizzledOrdered.h"
#endif

Ps::IntBool BV4_CapsuleSweepSingle(const Capsule& capsule, const PxVec3& dir, float maxDist, const BV4Tree& tree, SweepHit* PX_RESTRICT hit, PxU32 flags)
{
	const SourceMesh* PX_RESTRICT mesh = tree.mMeshInterface;

	CapsuleSweepParams Params;
	setupCapsuleParams(&Params, capsule, dir, maxDist, &tree, mesh, flags);

	if(tree.mNodes)
	{
		if(Params.mEarlyExit)
			processStreamNoOrder<LeafFunction_CapsuleSweepAny>(tree.mNodes, tree.mInitData, &Params);
		else
			processStreamOrdered<LeafFunction_CapsuleSweepClosest>(tree.mNodes, tree.mInitData, &Params);
	}
	else
		doBruteForceTests<LeafFunction_CapsuleSweepAny, LeafFunction_CapsuleSweepClosest>(mesh->getNbTriangles(), &Params);

	return computeImpactDataT<ImpactFunctionCapsule>(capsule, dir, hit, &Params, NULL, (flags & QUERY_MODIFIER_DOUBLE_SIDED)!=0, (flags & QUERY_MODIFIER_MESH_BOTH_SIDES)!=0);
}

// PT: capsule sweep callback version - currently not used

namespace
{
	struct CapsuleSweepParamsCB : CapsuleSweepParams
	{
		// PT: these new members are only here to call computeImpactDataT during traversal :( 
		// PT: TODO: most of them may not be needed
		// PT: TODO: for example mCapsuleCB probably dup of mLocalCapsule
		Capsule					mCapsuleCB;		// Capsule in original space (maybe not local/mesh space)
		PxVec3					mDirCB;			// Dir in original space (maybe not local/mesh space)
		const PxMat44*			mWorldm_Aligned;
		PxU32					mFlags;

		SweepUnlimitedCallback	mCallback;
		void*					mUserData;
		float					mMaxDist;
		bool					mNodeSorting;
	};

class LeafFunction_CapsuleSweepCB
{
public:

	static PX_FORCE_INLINE Ps::IntBool doLeafTest(CapsuleSweepParamsCB* PX_RESTRICT params, PxU32 primIndex)
	{
		PxU32 nbToGo = getNbPrimitives(primIndex);
		do
		{
			if(triCapsuleSweep(params, primIndex, params->mNodeSorting))
			{
				// PT: TODO: in this version we must compute the impact data immediately,
				// which is a terrible idea in general, but I'm not sure what else I can do.
				SweepHit hit;
				const bool b = computeImpactDataT<ImpactFunctionCapsule>(params->mCapsuleCB, params->mDirCB, &hit, params, params->mWorldm_Aligned, (params->mFlags & QUERY_MODIFIER_DOUBLE_SIDED)!=0, (params->mFlags & QUERY_MODIFIER_MESH_BOTH_SIDES)!=0);
				PX_ASSERT(b);
				PX_UNUSED(b);

				reportUnlimitedCallbackHit(params, hit);
			}
			primIndex++;
		}while(nbToGo--);

		return 0;
	}
};
}

// PT: for design decisions in this function, refer to the comments of BV4_GenericSweepCB().
void BV4_CapsuleSweepCB(const Capsule& capsule, const PxVec3& dir, float maxDist, const BV4Tree& tree, const PxMat44* PX_RESTRICT worldm_Aligned, SweepUnlimitedCallback callback, void* userData, PxU32 flags, bool nodeSorting)
{
	const SourceMesh* PX_RESTRICT mesh = tree.mMeshInterface;

	CapsuleSweepParamsCB Params;
	Params.mCapsuleCB		= capsule;
	Params.mDirCB			= dir;
	Params.mWorldm_Aligned	= worldm_Aligned;
	Params.mFlags			= flags;

	Params.mCallback		= callback;
	Params.mUserData		= userData;
	Params.mMaxDist			= maxDist;
	Params.mNodeSorting		= nodeSorting;
	setupCapsuleParams(&Params, capsule, dir, maxDist, &tree, mesh, flags);

	PX_ASSERT(!Params.mEarlyExit);

	if(tree.mNodes)
	{
		if(nodeSorting)
			processStreamOrdered<LeafFunction_CapsuleSweepCB>(tree.mNodes, tree.mInitData, &Params);
		else
			processStreamNoOrder<LeafFunction_CapsuleSweepCB>(tree.mNodes, tree.mInitData, &Params);
	}
	else
		doBruteForceTests<LeafFunction_CapsuleSweepCB, LeafFunction_CapsuleSweepCB>(mesh->getNbTriangles(), &Params);
}

#endif
