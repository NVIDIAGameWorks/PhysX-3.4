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

#ifndef PX_QUICKHULL_CONVEXHULLLIB_H
#define PX_QUICKHULL_CONVEXHULLLIB_H

#include "ConvexHullLib.h"
#include "Ps.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

namespace local
{
	class QuickHull;
	struct QuickHullVertex;
}

namespace physx
{
	class ConvexHull;

	//////////////////////////////////////////////////////////////////////////
	// Quickhull lib constructs the hull from given input points. The resulting hull 
	// will only contain a subset of the input points. The algorithm does incrementally
	// adds most furthest vertices to the starting simplex. The produced hulls are build with high precision
	// and produce more stable and correct results, than the legacy algorithm. 
	class QuickHullConvexHullLib: public ConvexHullLib, public Ps::UserAllocated
	{
		PX_NOCOPY(QuickHullConvexHullLib)
	public:

		// functions
		QuickHullConvexHullLib(const PxConvexMeshDesc& desc, const PxCookingParams& params);

		~QuickHullConvexHullLib();

		// computes the convex hull from provided points
		virtual PxConvexMeshCookingResult::Enum createConvexHull();

		// fills the convexmeshdesc with computed hull data
		virtual void fillConvexMeshDesc(PxConvexMeshDesc& desc);

		// provide the edge list information
		virtual bool createEdgeList(const PxU32, const PxU8* , PxU8** , PxU16** , PxU16** );

	protected:
		// if vertex limit reached we need to expand the hull using the OBB slicing
		PxConvexMeshCookingResult::Enum expandHullOBB();

		// if vertex limit reached we need to expand the hull using the plane shifting
		PxConvexMeshCookingResult::Enum expandHull();

		// checks for collinearity and co planarity
		// returns true if the simplex was ok, we can reuse the computed tolerances and min/max values
		bool cleanupForSimplex(PxVec3* vertices, PxU32 vertexCount, local::QuickHullVertex* minimumVertex, 
			local::QuickHullVertex* maximumVertex, float& tolerance, float& planeTolerance);

		// fill the result desc from quick hull convex
		void fillConvexMeshDescFromQuickHull(PxConvexMeshDesc& desc);

		// fill the result desc from cropped hull convex
		void fillConvexMeshDescFromCroppedHull(PxConvexMeshDesc& desc);

	private:
		local::QuickHull*		mQuickHull;		// the internal quick hull representation
		ConvexHull*				mCropedConvexHull; //the hull cropped from OBB, used for vertex limit path

		PxU8*					mOutMemoryBuffer;   // memory buffer used for output data
		PxU16*					mFaceTranslateTable; // translation table mapping output faces to internal quick hull table
	};
}

#endif
