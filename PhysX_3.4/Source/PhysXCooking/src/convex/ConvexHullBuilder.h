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


#ifndef PX_CONVEXHULLBUILDER_H
#define PX_CONVEXHULLBUILDER_H

#include "GuConvexMeshData.h"
#include "PsUserAllocated.h"
#include "PxCooking.h"

namespace physx
{
	struct PxHullPolygon;
	class ConvexHullLib;

	namespace Gu
	{
		struct EdgeDescData;
		struct ConvexHullData;
	} // namespace Gu

	struct HullTriangleData
	{
		PxU32	mRef[3];
	};

	class ConvexHullBuilder : public Ps::UserAllocated
	{
		public:
												ConvexHullBuilder(Gu::ConvexHullData* hull, const bool buildGRBData);
												~ConvexHullBuilder();

					bool						init(PxU32 nbVerts, const PxVec3* verts, const PxU32* indices, const PxU32 nbIndices, const PxU32 nbPolygons, 
													const PxHullPolygon* hullPolygons, bool doValidation = true, ConvexHullLib* hullLib = NULL);

					bool						save(PxOutputStream& stream, bool platformMismatch)	const;
					bool						copy(Gu::ConvexHullData& hullData, PxU32& nb);
					
					bool						createEdgeList(bool doValidation, PxU32 nbEdges);
					bool						checkHullPolygons()	const;										

					bool						calculateVertexMapTable(PxU32 nbPolygons, bool userPolygons = false);					

		PX_INLINE	PxU32						computeNbPolygons()		const
												{
													PX_ASSERT(mHull->mNbPolygons);
													return mHull->mNbPolygons;
												}

					PxVec3*						mHullDataHullVertices;
					Gu::HullPolygonData*		mHullDataPolygons;
					PxU8*						mHullDataVertexData8;
					PxU8*						mHullDataFacesByEdges8;
					PxU8*						mHullDataFacesByVertices8;

					PxU16*						mEdgeData16;	//!< Edge indices indexed by hull polygons
					PxU16*						mEdges;			//!< Edge to vertex mapping

					Gu::ConvexHullData*			mHull;
					bool						mBuildGRBData;
					
		protected:										
					bool						computeGeomCenter(PxVec3& , PxU32 numFaces, HullTriangleData* faces) const; 
	};
}

#endif	// PX_CONVEXHULLBUILDER_H

