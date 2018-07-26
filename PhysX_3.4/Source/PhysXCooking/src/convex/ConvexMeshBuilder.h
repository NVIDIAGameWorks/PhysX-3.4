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


#ifndef PX_COLLISION_CONVEXMESHBUILDER
#define PX_COLLISION_CONVEXMESHBUILDER

#include "GuConvexMeshData.h"
#include "PxCooking.h"
#include "ConvexPolygonsBuilder.h"

namespace physx
{
	//////////////////////////////////////////////////////////////////////////
	// Convex mesh builder, creates the convex mesh from given polygons and creates internal data
	class ConvexMeshBuilder
	{
	public:
									ConvexMeshBuilder(const bool buildGRBData);
									~ConvexMeshBuilder();

				// loads the computed or given convex hull from descriptor. 
				// the descriptor does contain polygons directly, triangles are not allowed
				bool				build(const PxConvexMeshDesc&, PxU32 gaussMapVertexLimit, bool validateOnly = false, ConvexHullLib* hullLib = NULL);

				// save the convex mesh into stream
				bool				save(PxOutputStream& stream, bool platformMismatch)		const;

				// copy the convex mesh into internal convex mesh, which can be directly used then
				bool				copy(Gu::ConvexHullData& convexData, PxU32& nb);

				// loads the convex mesh from given polygons
				bool				loadConvexHull(const PxConvexMeshDesc&, ConvexHullLib* hullLib);

				// computed hull polygons from given triangles
				bool				computeHullPolygons(const PxU32& nbVerts,const PxVec3* verts, const PxU32& nbTriangles, const PxU32* triangles, PxAllocatorCallback& inAllocator,
										 PxU32& outNbVerts, PxVec3*& outVertices, PxU32& nbIndices, PxU32*& indices, PxU32& nbPolygons, PxHullPolygon*& polygons);

				// compute big convex data
				bool				computeGaussMaps();

				// compute mass, inertia tensor
				void				computeMassInfo(bool lowerPrecision);
// TEST_INTERNAL_OBJECTS
				// internal objects
				void				computeInternalObjects();
//~TEST_INTERNAL_OBJECTS

				// return computed mass
				PxReal				getMass() const { return mMass; }

				// return computed inertia tensor
				const PxMat33&		getInertia() const { return mInertia; }

				// return big convex data
				BigConvexData*		getBigConvexData() const  { return mBigConvexData; }

				// set big convex data
				void				setBigConvexData(BigConvexData* data) { mBigConvexData = data; }

		mutable	ConvexPolygonsBuilder	hullBuilder;

	protected:
		Gu::ConvexHullData			mHullData;		

		BigConvexData*				mBigConvexData;		//!< optional, only for large meshes! PT: redundant with ptr in chull data? Could also be end of other buffer
		PxReal						mMass;				//this is mass assuming a unit density that can be scaled by instances!
		PxMat33						mInertia;			//in local space of mesh!

	};

}

#endif
