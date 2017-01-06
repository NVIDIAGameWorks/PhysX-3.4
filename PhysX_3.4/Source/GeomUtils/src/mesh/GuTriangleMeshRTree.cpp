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

#include "GuTriangleMesh.h"
#include "GuTriangleMeshRTree.h"
#if PX_ENABLE_DYNAMIC_MESH_RTREE
#include "GuConvexEdgeFlags.h"
#endif

using namespace physx;

namespace physx
{

Gu::RTreeTriangleMesh::RTreeTriangleMesh(GuMeshFactory& factory, TriangleMeshData& d)
:	TriangleMesh(factory, d)
{
	PX_ASSERT(d.mType==PxMeshMidPhase::eBVH33);

	RTreeTriangleData& rtreeData = static_cast<RTreeTriangleData&>(d);
	mRTree = rtreeData.mRTree;
	rtreeData.mRTree.mPages = NULL;
}

Gu::TriangleMesh* Gu::RTreeTriangleMesh::createObject(PxU8*& address, PxDeserializationContext& context)
{
	RTreeTriangleMesh* obj = new (address) RTreeTriangleMesh(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(RTreeTriangleMesh);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

void Gu::RTreeTriangleMesh::exportExtraData(PxSerializationContext& stream)
{
	mRTree.exportExtraData(stream);
	TriangleMesh::exportExtraData(stream);
}

void Gu::RTreeTriangleMesh::importExtraData(PxDeserializationContext& context)
{
	mRTree.importExtraData(context);
	TriangleMesh::importExtraData(context);
}

#if PX_ENABLE_DYNAMIC_MESH_RTREE
PxVec3 * Gu::RTreeTriangleMesh::getVerticesForModification()
{
	return const_cast<PxVec3*>(getVertices());
}

template<typename IndexType>
struct RefitCallback : Gu::RTree::CallbackRefit
{
	const PxVec3* newPositions;
	const IndexType* indices;

	RefitCallback(const PxVec3* aNewPositions, const IndexType* aIndices) : newPositions(aNewPositions), indices(aIndices) {}
	PX_FORCE_INLINE ~RefitCallback() {}

	virtual void recomputeBounds(PxU32 index, shdfnd::aos::Vec3V& aMn, shdfnd::aos::Vec3V& aMx)
	{
		using namespace shdfnd::aos;

		// Each leaf box has a set of triangles
		Gu::LeafTriangles currentLeaf; currentLeaf.Data = index;
		PxU32 nbTris = currentLeaf.GetNbTriangles();
		PxU32 baseTri = currentLeaf.GetTriangleIndex();
		PX_ASSERT(nbTris > 0);
		const IndexType* vInds = indices + 3 * baseTri;
		Vec3V vPos = V3LoadU(newPositions[vInds[0]]);
		Vec3V mn = vPos, mx = vPos;
		//PxBounds3 result(newPositions[vInds[0]], newPositions[vInds[0]]);
		vPos = V3LoadU(newPositions[vInds[1]]);
		mn = V3Min(mn, vPos); mx = V3Max(mx, vPos);
		vPos = V3LoadU(newPositions[vInds[2]]);
		mn = V3Min(mn, vPos); mx = V3Max(mx, vPos);
		for (PxU32 i = 1; i < nbTris; i++)
		{
			const IndexType* vInds1 = indices + 3 * (baseTri + i);
			vPos = V3LoadU(newPositions[vInds1[0]]);
			mn = V3Min(mn, vPos); mx = V3Max(mx, vPos);
			vPos = V3LoadU(newPositions[vInds1[1]]);
			mn = V3Min(mn, vPos); mx = V3Max(mx, vPos);
			vPos = V3LoadU(newPositions[vInds1[2]]);
			mn = V3Min(mn, vPos); mx = V3Max(mx, vPos);
		}

		aMn = mn;
		aMx = mx;
	}
};

PxBounds3 Gu::RTreeTriangleMesh::refitBVH()
{
	PxBounds3 meshBounds;
	if (has16BitIndices())
	{
		RefitCallback<PxU16> cb(mVertices, static_cast<const PxU16*>(mTriangles));
		mRTree.refitAllStaticTree(cb, &meshBounds);
	}
	else
	{
		RefitCallback<PxU32> cb(mVertices, static_cast<const PxU32*>(mTriangles));
		mRTree.refitAllStaticTree(cb, &meshBounds);
	}

	// reset edge flags and remember we did that using a mesh flag (optimization)
	if ((mRTree.mFlags & RTree::IS_EDGE_SET) == 0)
	{
		mRTree.mFlags |= RTree::IS_EDGE_SET;
		const PxU32 nbTris = getNbTriangles();
		for (PxU32 i = 0; i < nbTris; i++)
			mExtraTrigData[i] |= (Gu::ETD_CONVEX_EDGE_01 | Gu::ETD_CONVEX_EDGE_12 | Gu::ETD_CONVEX_EDGE_20);
	}

	mAABB = meshBounds;
	return meshBounds;
}
#endif

} // namespace physx
