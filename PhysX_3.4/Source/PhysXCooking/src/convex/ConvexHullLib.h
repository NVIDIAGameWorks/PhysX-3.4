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


#ifndef PX_CONVEXHULLLIB_H
#define PX_CONVEXHULLLIB_H

#include "PxConvexMeshDesc.h"
#include "PxCooking.h"
#include "CmPhysXCommon.h"

namespace physx
{	
	//////////////////////////////////////////////////////////////////////////
	// base class for the convex hull libraries - inflation based and quickhull
	class ConvexHullLib
	{		
		PX_NOCOPY(ConvexHullLib)
	public:
		// functions
		ConvexHullLib(const PxConvexMeshDesc& desc, const PxCookingParams& params)
			: mConvexMeshDesc(desc), mCookingParams(params), mSwappedIndices(NULL),
			mShiftedVerts(NULL)
		{
		}

		virtual ~ConvexHullLib();
			
		// computes the convex hull from provided points
		virtual PxConvexMeshCookingResult::Enum createConvexHull() = 0;

		// fills the PxConvexMeshDesc with computed hull data
		virtual void fillConvexMeshDesc(PxConvexMeshDesc& desc) = 0;

		// compute the edge list information if possible
		virtual bool createEdgeList(const PxU32 nbIndices, const PxU8* indices, PxU8** hullDataFacesByEdges8, PxU16** edgeData16, PxU16** edges) = 0;

		static const PxU32 gpuMaxVertsPerFace = 32;

	protected:

		// clean input vertices from duplicates, normalize etc.
		bool cleanupVertices(PxU32 svcount, // input vertex count
			const PxVec3* svertices, // vertices
			PxU32 stride,		// stride
			PxU32& vcount,		// output number of vertices
			PxVec3* vertices,	// location to store the results.			
			PxVec3& scale,		// scale
			PxVec3& center);	// center

		// shift vertices around origin and clean input vertices from duplicates, normalize etc.
		bool shiftAndcleanupVertices(PxU32 svcount, // input vertex count
			const PxVec3* svertices, // vertices
			PxU32 stride,		// stride
			PxU32& vcount,		// output number of vertices
			PxVec3* vertices,	// location to store the results.			
			PxVec3& scale,		// scale
			PxVec3& center);	// center

		void swapLargestFace(PxConvexMeshDesc& desc);

		void shiftConvexMeshDesc(PxConvexMeshDesc& desc);

	protected:
		const PxConvexMeshDesc&			mConvexMeshDesc;
		const PxCookingParams&			mCookingParams;
		PxU32*							mSwappedIndices;
		PxVec3							mOriginShift;
		PxVec3*							mShiftedVerts;
	};
}

#endif
