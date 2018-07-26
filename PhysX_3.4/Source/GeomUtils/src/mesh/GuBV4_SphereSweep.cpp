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

#include "foundation/PxSimpleTypes.h"
#include "foundation/PxMat44.h"
#include "GuBV4.h"
#include "GuBox.h"
#include "GuSphere.h"
#include "GuSIMDHelpers.h"
#include "GuSweepSphereTriangle.h"

using namespace physx;
using namespace Gu;

#if PX_INTEL_FAMILY  && !defined(PX_SIMD_DISABLED)

#include "PsVecMath.h"
using namespace physx::shdfnd::aos;

#include "GuBV4_Common.h"

// PT: for sphere-sweeps we use method 3 in \\sw\physx\PhysXSDK\3.4\trunk\InternalDocumentation\GU\Sweep strategies.ppt

namespace
{
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
#endif
		BV4_ALIGN16(Vec3p	mLocalDir_Padded);	// PT: TODO: this one could be switched to PaddedAligned & V4LoadA (TA34704)
		BV4_ALIGN16(Vec3p	mOrigin_Padded);	// PT: TODO: this one could be switched to PaddedAligned & V4LoadA (TA34704)
	};

	struct SphereSweepParams : RayParams
	{
		const IndTri32*		PX_RESTRICT	mTris32;
		const IndTri16*		PX_RESTRICT	mTris16;
		const PxVec3*		PX_RESTRICT	mVerts;

		PxVec3				mOriginalExtents_Padded;

		RaycastHitInternal	mStabbedFace;
		PxU32				mBackfaceCulling;
		PxU32				mEarlyExit;

		PxVec3				mP0, mP1, mP2;
		PxVec3				mBestTriNormal;
		float				mBestAlignmentValue;
		float				mBestDistance;
		float				mMaxDist;
	};
}

#include "GuBV4_AABBAABBSweepTest.h"

// PT: TODO: __fastcall removed to make it compile everywhere. Revisit.
static bool /*__fastcall*/ triSphereSweep(SphereSweepParams* PX_RESTRICT params, PxU32 primIndex, bool nodeSorting=true)
{
	PxU32 VRef0, VRef1, VRef2;
	getVertexReferences(VRef0, VRef1, VRef2, primIndex, params->mTris32, params->mTris16);

	const PxVec3& p0 = params->mVerts[VRef0];
	const PxVec3& p1 = params->mVerts[VRef1];
	const PxVec3& p2 = params->mVerts[VRef2];

	PxVec3 normal = (p1 - p0).cross(p2 - p0);

	// Backface culling
	const bool culled = params->mBackfaceCulling && normal.dot(params->mLocalDir_Padded) > 0.0f;
	if(culled)
		return false;

	const PxTriangle T(p0, p1, p2);	// PT: TODO: check potential bad ctor/dtor here (TA34704) <= or avoid creating the tri, not needed anymore

	normal.normalize();

	// PT: TODO: we lost some perf when switching to PhysX version. Revisit/investigate. (TA34704)
	float dist;
	bool directHit;
	if(!sweepSphereVSTri(T.verts, normal, params->mOrigin_Padded, params->mOriginalExtents_Padded.x, params->mLocalDir_Padded, dist, directHit, true))
		return false;

	const PxReal distEpsilon = GU_EPSILON_SAME_DISTANCE; // pick a farther hit within distEpsilon that is more opposing than the previous closest hit
	const PxReal alignmentValue = computeAlignmentValue(normal, params->mLocalDir_Padded);
	if(keepTriangle(dist, alignmentValue, params->mBestDistance, params->mBestAlignmentValue, params->mMaxDist, distEpsilon))
	{
		params->mStabbedFace.mDistance = dist;
		params->mStabbedFace.mTriangleID = primIndex;
		params->mP0 = p0;
		params->mP1 = p1;
		params->mP2 = p2;
		params->mBestDistance = PxMin(params->mBestDistance, dist); // exact lower bound
		params->mBestAlignmentValue = alignmentValue;
		params->mBestTriNormal = normal;
		if(nodeSorting)
		{
#ifndef GU_BV4_USE_SLABS
			setupRayData(params, dist, params->mOrigin_Padded, params->mLocalDir_Padded);
#endif
		}
		return true;
	}
	return false;
}

namespace
{
class LeafFunction_SphereSweepClosest
{
public:
	static PX_FORCE_INLINE void doLeafTest(SphereSweepParams* PX_RESTRICT params, PxU32 primIndex)
	{
		PxU32 nbToGo = getNbPrimitives(primIndex);
		do
		{
			triSphereSweep(params, primIndex);
			primIndex++;
		}while(nbToGo--);
	}
};

class LeafFunction_SphereSweepAny
{
public:
	static PX_FORCE_INLINE Ps::IntBool doLeafTest(SphereSweepParams* PX_RESTRICT params, PxU32 primIndex)
	{
		PxU32 nbToGo = getNbPrimitives(primIndex);
		do
		{
			if(triSphereSweep(params, primIndex))
				return 1;

			primIndex++;
		}while(nbToGo--);

		return 0;
	}
};

class ImpactFunctionSphere
{
public:
	static PX_FORCE_INLINE void computeImpact(PxVec3& impactPos, PxVec3& impactNormal, const Sphere& sphere, const PxVec3& dir, const PxReal t, const TrianglePadded& triangle)
	{
		computeSphereTriImpactData(impactPos, impactNormal, sphere.center, dir, t, triangle);
	}
};
}

template<class ParamsT>
static PX_FORCE_INLINE void setupSphereParams(ParamsT* PX_RESTRICT params, const Sphere& sphere, const PxVec3& dir, float maxDist, const BV4Tree* PX_RESTRICT tree, const PxMat44* PX_RESTRICT worldm_Aligned, const SourceMesh* PX_RESTRICT mesh, PxU32 flags)
{
	params->mOriginalExtents_Padded		= PxVec3(sphere.radius);
	params->mStabbedFace.mTriangleID	= PX_INVALID_U32;
	params->mStabbedFace.mDistance		= maxDist;
	params->mBestDistance				= PX_MAX_REAL;
	params->mBestAlignmentValue			= 2.0f;
	params->mMaxDist					= maxDist;
	setupParamsFlags(params, flags);

	setupMeshPointersAndQuantizedCoeffs(params, mesh, tree);

	computeLocalRay(params->mLocalDir_Padded, params->mOrigin_Padded, dir, sphere.center, worldm_Aligned);

#ifndef GU_BV4_USE_SLABS
	setupRayData(params, maxDist, params->mOrigin_Padded, params->mLocalDir_Padded);
#endif
}

#include "GuBV4_Internal.h"
#ifdef GU_BV4_USE_SLABS
	#include "GuBV4_Slabs.h"
#endif
#include "GuBV4_ProcessStreamOrdered_SegmentAABB_Inflated.h"
#include "GuBV4_ProcessStreamNoOrder_SegmentAABB_Inflated.h"
#ifdef GU_BV4_USE_SLABS
	#include "GuBV4_Slabs_KajiyaNoOrder.h"
	#include "GuBV4_Slabs_KajiyaOrdered.h"
#endif

Ps::IntBool BV4_SphereSweepSingle(const Sphere& sphere, const PxVec3& dir, float maxDist, const BV4Tree& tree, const PxMat44* PX_RESTRICT worldm_Aligned, SweepHit* PX_RESTRICT hit, PxU32 flags)
{
	const SourceMesh* PX_RESTRICT mesh = tree.mMeshInterface;

	SphereSweepParams Params;
	setupSphereParams(&Params, sphere, dir, maxDist, &tree, worldm_Aligned, mesh, flags);

	if(tree.mNodes)
	{
		if(Params.mEarlyExit)
			processStreamRayNoOrder(1, LeafFunction_SphereSweepAny)(tree.mNodes, tree.mInitData, &Params);
		else
			processStreamRayOrdered(1, LeafFunction_SphereSweepClosest)(tree.mNodes, tree.mInitData, &Params);
	}
	else
		doBruteForceTests<LeafFunction_SphereSweepAny, LeafFunction_SphereSweepClosest>(mesh->getNbTriangles(), &Params);

	return computeImpactDataT<ImpactFunctionSphere>(sphere, dir, hit, &Params, worldm_Aligned, (flags & QUERY_MODIFIER_DOUBLE_SIDED)!=0, (flags & QUERY_MODIFIER_MESH_BOTH_SIDES)!=0);
}

// PT: sphere sweep callback version - currently not used

namespace
{
	struct SphereSweepParamsCB : SphereSweepParams
	{
		// PT: these new members are only here to call computeImpactDataT during traversal :( 
		// PT: TODO: most of them may not be needed if we just move sphere to local space before traversal
		Sphere					mSphere;	// Sphere in original space (maybe not local/mesh space)
		PxVec3					mDir;		// Dir in original space (maybe not local/mesh space)
		const PxMat44*			mWorldm_Aligned;
		PxU32					mFlags;

		SweepUnlimitedCallback	mCallback;
		void*					mUserData;
		bool					mNodeSorting;
	};

class LeafFunction_SphereSweepCB
{
public:
	static PX_FORCE_INLINE Ps::IntBool doLeafTest(SphereSweepParamsCB* PX_RESTRICT params, PxU32 primIndex)
	{
		PxU32 nbToGo = getNbPrimitives(primIndex);
		do
		{
			if(triSphereSweep(params, primIndex, params->mNodeSorting))
			{
				// PT: TODO: in this version we must compute the impact data immediately,
				// which is a terrible idea in general, but I'm not sure what else I can do.
				SweepHit hit;
				const bool b = computeImpactDataT<ImpactFunctionSphere>(params->mSphere, params->mDir, &hit, params, params->mWorldm_Aligned, (params->mFlags & QUERY_MODIFIER_DOUBLE_SIDED)!=0, (params->mFlags & QUERY_MODIFIER_MESH_BOTH_SIDES)!=0);
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
void BV4_SphereSweepCB(const Sphere& sphere, const PxVec3& dir, float maxDist, const BV4Tree& tree, const PxMat44* PX_RESTRICT worldm_Aligned, SweepUnlimitedCallback callback, void* userData, PxU32 flags, bool nodeSorting)
{
	const SourceMesh* PX_RESTRICT mesh = tree.mMeshInterface;

	SphereSweepParamsCB Params;
	Params.mSphere			= sphere;
	Params.mDir				= dir;
	Params.mWorldm_Aligned	= worldm_Aligned;
	Params.mFlags			= flags;

	Params.mCallback		= callback;
	Params.mUserData		= userData;
	Params.mMaxDist			= maxDist;
	Params.mNodeSorting		= nodeSorting;
	setupSphereParams(&Params, sphere, dir, maxDist, &tree, worldm_Aligned, mesh, flags);
	
	PX_ASSERT(!Params.mEarlyExit);

	if(tree.mNodes)
	{
		if(nodeSorting)
			processStreamRayOrdered(1, LeafFunction_SphereSweepCB)(tree.mNodes, tree.mInitData, &Params);
		else
			processStreamRayNoOrder(1, LeafFunction_SphereSweepCB)(tree.mNodes, tree.mInitData, &Params);
	}
	else
		doBruteForceTests<LeafFunction_SphereSweepCB, LeafFunction_SphereSweepCB>(mesh->getNbTriangles(), &Params);
}


// Old box sweep callback version, using sphere code

namespace
{
struct BoxSweepParamsCB : SphereSweepParams
{
	MeshSweepCallback	mCallback;
	void*				mUserData;
};

class ExLeafTestSweepCB
{
public:
	static PX_FORCE_INLINE void doLeafTest(BoxSweepParamsCB* PX_RESTRICT params, PxU32 primIndex)
	{
		PxU32 nbToGo = getNbPrimitives(primIndex);
		do
		{
			PxU32 VRef0, VRef1, VRef2;
			getVertexReferences(VRef0, VRef1, VRef2, primIndex, params->mTris32, params->mTris16);

			{
//				const PxU32 vrefs[3] = { VRef0, VRef1, VRef2 };
				float dist = params->mStabbedFace.mDistance;
				if((params->mCallback)(params->mUserData, params->mVerts[VRef0], params->mVerts[VRef1], params->mVerts[VRef2], primIndex, /*vrefs,*/ dist))
					return;

				if(dist<params->mStabbedFace.mDistance)
				{
					params->mStabbedFace.mDistance = dist;
#ifndef GU_BV4_USE_SLABS
					setupRayData(params, dist, params->mOrigin_Padded, params->mLocalDir_Padded);
#endif
				}
			}

			primIndex++;
		}while(nbToGo--);
	}
};
}

void BV4_GenericSweepCB_Old(const PxVec3& origin, const PxVec3& extents, const PxVec3& dir, float maxDist, const BV4Tree& tree, const PxMat44* PX_RESTRICT worldm_Aligned, MeshSweepCallback callback, void* userData)
{
	BoxSweepParamsCB Params;
	Params.mCallback				= callback;
	Params.mUserData				= userData;
	Params.mOriginalExtents_Padded	= extents;

	Params.mStabbedFace.mTriangleID	= PX_INVALID_U32;
	Params.mStabbedFace.mDistance	= maxDist;

	computeLocalRay(Params.mLocalDir_Padded, Params.mOrigin_Padded, dir, origin, worldm_Aligned);

#ifndef GU_BV4_USE_SLABS
	setupRayData(&Params, maxDist, Params.mOrigin_Padded, Params.mLocalDir_Padded);
#endif

	const SourceMesh* PX_RESTRICT mesh = tree.mMeshInterface;

	setupMeshPointersAndQuantizedCoeffs(&Params, mesh, &tree);

	if(tree.mNodes)
	{
		processStreamRayOrdered(1, ExLeafTestSweepCB)(tree.mNodes, tree.mInitData, &Params);
	}
	else
	{
		const PxU32 nbTris = mesh->getNbTriangles();
		PX_ASSERT(nbTris<16);
		ExLeafTestSweepCB::doLeafTest(&Params, nbTris);
	}
}

#endif

