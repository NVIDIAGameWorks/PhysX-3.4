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

#ifndef GU_SHAPECONVEX_H
#define GU_SHAPECONVEX_H

#include "GuConvexMeshData.h"
#include "CmScaling.h"

namespace physx
{
namespace Gu
{
	struct PolygonalData;
	typedef void	(*HullPrefetchCB)		(PxU32 numVerts, const PxVec3* PX_RESTRICT verts);
	typedef void	(*HullProjectionCB)		(const PolygonalData& data, const PxVec3& dir, const Cm::Matrix34& world2hull, const Cm::FastVertex2ShapeScaling& scaling, PxReal& minimum, PxReal& maximum);
	typedef PxU32	(*SelectClosestEdgeCB)	(const PolygonalData& data, const Cm::FastVertex2ShapeScaling& scaling, const PxVec3& localDirection);

	struct PolygonalData
	{
		// Data
		PxVec3								mCenter;
		PxU32								mNbVerts;
		PxU32								mNbPolygons;
		PxU32								mNbEdges;
		const Gu::HullPolygonData*			mPolygons;
		const PxVec3*						mVerts;
		const PxU8*							mPolygonVertexRefs;
		const PxU8*							mFacesByEdges;
		const PxU16*						mVerticesByEdges;

		Gu::InternalObjectsData				mInternal;
		union
		{
			const Gu::BigConvexRawData*		mBigData;	// Only for big convexes
			const PxVec3*					mHalfSide;	// Only for boxes
		};

		// Code
		HullProjectionCB					mProjectHull;
		SelectClosestEdgeCB					mSelectClosestEdgeCB;

		PX_FORCE_INLINE const PxU8*	getPolygonVertexRefs(const Gu::HullPolygonData& poly)	const
		{
			return mPolygonVertexRefs + poly.mVRef8;
		}
	};

#if PX_VC 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
	class PX_PHYSX_COMMON_API PolygonalBox
	{
	public:
									PolygonalBox(const PxVec3& halfSide);

			void					getPolygonalData(PolygonalData* PX_RESTRICT dst)	const;

			const PxVec3&			mHalfSide;
			PxVec3					mVertices[8];
			Gu::HullPolygonData		mPolygons[6];
	private:
			PolygonalBox& operator=(const PolygonalBox&);
	};
#if PX_VC 
     #pragma warning(pop) 
#endif

	void getPolygonalData_Convex(PolygonalData* PX_RESTRICT dst, const Gu::ConvexHullData* PX_RESTRICT src, const Cm::FastVertex2ShapeScaling& scaling);
}
}

#endif
