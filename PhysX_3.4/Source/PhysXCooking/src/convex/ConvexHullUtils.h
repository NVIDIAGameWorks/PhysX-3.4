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


#ifndef PX_CONVEXHULLUTILS_H
#define PX_CONVEXHULLUTILS_H

#include "foundation/PxMemory.h"
#include "foundation/PxPlane.h"

#include "CmPhysXCommon.h"

#include "PsUserAllocated.h"
#include "PsArray.h"
#include "PsMathUtils.h"

#include "PxConvexMeshDesc.h"

namespace physx
{

	//////////////////////////////////////////////////////////////////////////
	// helper class for hull construction, holds the vertices and planes together
	// while cropping the hull with planes
	class ConvexHull : public Ps::UserAllocated
	{
	public:

		// Helper class for halfedge representation
		class HalfEdge
		{
		public:
			PxI16 ea;         // the other half of the edge (index into edges list)
			PxU8 v;  // the vertex at the start of this edge (index into vertices list)
			PxU8 p;  // the facet on which this edge lies (index into facets list)
			HalfEdge(){}
			HalfEdge(PxI16 _ea, PxU8 _v, PxU8 _p) :ea(_ea), v(_v), p(_p){}
		};

		ConvexHull& operator = (const ConvexHull&);

		// construct the base cube hull from given max/min AABB
		ConvexHull(const PxVec3& bmin, const PxVec3& bmax, const Ps::Array<PxPlane>& inPlanes);

		// construct the base cube hull from given OBB
		ConvexHull(const PxVec3& extent, const PxTransform& transform, const Ps::Array<PxPlane>& inPlanes);

		// copy constructor
		ConvexHull(const ConvexHull& srcHull)
			: mInputPlanes(srcHull.getInputPlanes())
		{
			copyHull(srcHull);
		}

		// construct plain hull
		ConvexHull(const Ps::Array<PxPlane>& inPlanes)
			: mInputPlanes(inPlanes)
		{
		}

		// finds the candidate plane, returns -1 otherwise
		PxI32 findCandidatePlane(float planetestepsilon, float epsilon) const;

		// internal check of the hull integrity
		bool assertIntact(float epsilon) const;

		// return vertices
		const Ps::Array<PxVec3>& getVertices() const
		{
			return mVertices;
		}

		// return edges
		const Ps::Array<HalfEdge>& getEdges() const
		{
			return mEdges;
		}

		// return faces
		const Ps::Array<PxPlane>& getFacets() const
		{
			return mFacets;
		}

		// return input planes
		const Ps::Array<PxPlane>& getInputPlanes() const
		{
			return mInputPlanes;
		}

		// return vertices
		Ps::Array<PxVec3>& getVertices()
		{
			return mVertices;
		}

		// return edges
		Ps::Array<HalfEdge>& getEdges()
		{
			return mEdges;
		}

		// return faces
		Ps::Array<PxPlane>& getFacets()
		{
			return mFacets;
		}

		// returns the maximum number of vertices on a face
		PxU32 maxNumVertsPerFace() const;

		// copy the hull from source
		void copyHull(const ConvexHull& src)
		{
			mVertices.resize(src.getVertices().size());
			mEdges.resize(src.getEdges().size());
			mFacets.resize(src.getFacets().size());

			PxMemCopy(mVertices.begin(), src.getVertices().begin(), src.getVertices().size()*sizeof(PxVec3));
			PxMemCopy(mEdges.begin(), src.getEdges().begin(), src.getEdges().size()*sizeof(HalfEdge));
			PxMemCopy(mFacets.begin(), src.getFacets().begin(), src.getFacets().size()*sizeof(PxPlane));
		}

	private:
		Ps::Array<PxVec3>	mVertices;
		Ps::Array<HalfEdge> mEdges;
		Ps::Array<PxPlane>  mFacets;
		const Ps::Array<PxPlane>&	mInputPlanes;
	};

	//////////////////////////////////////////////////////////////////////////|
	// Crops the hull with a provided plane and with given epsilon
	// returns new hull if succeeded
	ConvexHull* convexHullCrop(const ConvexHull& convex, const PxPlane& slice, float planetestepsilon);

	//////////////////////////////////////////////////////////////////////////|
	// three planes intersection
	PX_FORCE_INLINE PxVec3 threePlaneIntersection(const PxPlane& p0, const PxPlane& p1, const PxPlane& p2)
	{
		PxMat33 mp = (PxMat33(p0.n, p1.n, p2.n)).getTranspose();
		PxMat33 mi = (mp).getInverse();
		PxVec3 b(p0.d, p1.d, p2.d);
		return -mi.transform(b);
	}

	//////////////////////////////////////////////////////////////////////////
	// Compute OBB around given convex hull
	bool computeOBBFromConvex(const PxConvexMeshDesc& desc, PxVec3& sides, PxTransform& matrix);
}

#endif
