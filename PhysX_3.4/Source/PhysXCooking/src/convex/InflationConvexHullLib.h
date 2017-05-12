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

#ifndef PX_INFLATION_CONVEXHULLLIB_H
#define PX_INFLATION_CONVEXHULLLIB_H

#include "ConvexHullLib.h"
#include "Ps.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

namespace local
{
	class HullTriangles;	
}

namespace physx
{
	class ConvexHull;

	//////////////////////////////////////////////////////////////////////////
	// internal hull lib results
	struct ConvexHullLibResult
	{
		// return code
		enum ErrorCode
		{
			eSUCCESS = 0,		// success!
			eFAILURE,		// failed.
			eVERTEX_LIMIT_REACHED,		// vertex limit reached fallback.
			eZERO_AREA_TEST_FAILED,		// area test failed - failed to create simplex
			ePOLYGON_LIMIT_REACHED		// polygons hard limit 255 reached
		};

		PxU32	mVcount;
		PxU32	mIndexCount;
		PxU32	mPolygonCount;		
		PxVec3*	mVertices;
		PxU32*	mIndices;		
		PxHullPolygon* mPolygons;


		ConvexHullLibResult()
			: mVcount(0), mIndexCount(0), mPolygonCount(0),
			mVertices(NULL), mIndices(NULL), mPolygons(NULL)
		{
		}
	};	

	//////////////////////////////////////////////////////////////////////////
	// inflation based hull library. Using the legacy Stan hull lib with inflation
	// We construct the hull using incremental method and then inflate the resulting planes
	// by specified skinWidth. We take the planes and crop AABB with them to construct 
	// the final hull. This method may reduce the number of polygons significantly
	// in case of lot of vertices are used. On the other hand, we produce new vertices 
	// and enlarge the original hull constructed from the given input points.
	// Generally speaking, the increase of vertices is usually too big, so it is not worthy
	// to use this algorithm to reduce the number of polygons. This method is also very unprecise
	// and may produce invalid hulls. It is recommended to use the new quickhull library.
	class InflationConvexHullLib: public ConvexHullLib, public Ps::UserAllocated
	{
		PX_NOCOPY(InflationConvexHullLib)
	public:

		// functions
		InflationConvexHullLib(const PxConvexMeshDesc& desc, const PxCookingParams& params);

		~InflationConvexHullLib();

		// computes the convex hull from provided points
		virtual PxConvexMeshCookingResult::Enum createConvexHull();

		// Inflation convex hull does not store edge information so we cannot provide the edge list
		virtual bool createEdgeList(const PxU32 , const PxU8* , PxU8** , PxU16** , PxU16** ) { return false; }

		// fills the convexmeshdesc with computed hull data
		virtual void fillConvexMeshDesc(PxConvexMeshDesc& desc);

	protected:
		// internal

		// compute the hull
		ConvexHullLibResult::ErrorCode computeHull(PxU32 vertsCount, const PxVec3* verts);

		// computes the hull
		ConvexHullLibResult::ErrorCode calchull(const PxVec3* verts, PxU32 verts_count, ConvexHull*& hullOut);

		// computes the actual hull using the incremental algorithm
		ConvexHullLibResult::ErrorCode calchullgen(const PxVec3* verts, PxU32 verts_count, local::HullTriangles&  triangles);

		// calculates the hull planes from the triangles
		bool calchullplanes(const PxVec3* verts, local::HullTriangles&  triangles, Ps::Array<PxPlane>& planes);

		// construct the hull from given planes - create new verts
		bool overhull(const PxVec3* verts, PxU32 vertsCount,const Ps::Array<PxPlane>& planes, ConvexHull*& hullOut);

		// expand the hull with the limited triangles set
		ConvexHullLibResult::ErrorCode expandHull(const PxVec3* verts, PxU32 vertsCount, const local::HullTriangles&  triangles, ConvexHull*& hullOut);

		// expand the hull with the limited triangles set
		ConvexHullLibResult::ErrorCode expandHullOBB(const PxVec3* verts, PxU32 vertsCount, const local::HullTriangles&  triangles, ConvexHull*& hullOut);

	private:
		bool							mFinished;
		ConvexHullLibResult				mHullResult;
		float							mTolerance;
		float							mPlaneTolerance;
	};
}

#endif
