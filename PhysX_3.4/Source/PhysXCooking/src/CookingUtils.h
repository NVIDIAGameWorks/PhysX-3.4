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


#ifndef PX_COOKINGUTILS
#define PX_COOKINGUTILS

#include "foundation/PxVec3.h"
#include "PxPhysXConfig.h"

namespace physx
{
	//! Vertex cloud reduction result structure
	struct REDUCEDCLOUD
	{
				// Out
				PxVec3*				RVerts;		//!< Reduced list
				PxU32				NbRVerts;	//!< Reduced number of vertices
				PxU32*				CrossRef;	//!< nb_verts remapped indices
	};

	class ReducedVertexCloud
	{
		public:
		// Constructors/destructor
									ReducedVertexCloud(const PxVec3* verts, PxU32 nb_verts);
									~ReducedVertexCloud();
		// Free used bytes
				ReducedVertexCloud&	Clean();
		// Cloud reduction
				bool				Reduce(REDUCEDCLOUD* rc=NULL);
		// Data access
		PX_INLINE	PxU32			GetNbVerts()				const	{ return mNbVerts;		}
		PX_INLINE	PxU32			GetNbReducedVerts()			const	{ return mNbRVerts;		}
		PX_INLINE	const PxVec3*	GetReducedVerts()			const	{ return mRVerts;		}
		PX_INLINE	const PxVec3&	GetReducedVertex(PxU32 i)	const	{ return mRVerts[i];	}
		PX_INLINE	const PxU32*	GetCrossRefTable()			const	{ return mXRef;			}

		private:
		// Original vertex cloud
				PxU32				mNbVerts;	//!< Number of vertices
				const PxVec3*		mVerts;		//!< List of vertices (pointer copy)

		// Reduced vertex cloud
				PxU32				mNbRVerts;	//!< Reduced number of vertices
				PxVec3*				mRVerts;	//!< Reduced list of vertices
				PxU32*				mXRef;		//!< Cross-reference table (used to remap topologies)
	};

}

#endif
	
