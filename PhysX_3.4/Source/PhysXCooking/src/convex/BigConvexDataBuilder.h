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

#ifndef BIG_CONVEX_DATA_BUILDER_H
#define BIG_CONVEX_DATA_BUILDER_H

#include "foundation/PxMemory.h"
#include "PsVecMath.h"

namespace physx
{
	struct HullTriangleData;
	class BigConvexData;
	class ConvexHullBuilder;

	//////////////////////////////////////////////////////////////////////////
	//! Valencies creation structure
	struct ValenciesCreate
	{
		//! Constructor
								ValenciesCreate()	{ PxMemZero(this, sizeof(*this)); }

				PxU32			nbVerts;		//!< Number of vertices
				PxU32			nbFaces;		//!< Number of faces
		const	PxU32*			dFaces;			//!< List of faces (triangle list)
		const	PxU16*			wFaces;			//!< List of faces (triangle list)
				bool			adjacentList;	//!< Compute list of adjacent vertices or not
	};

	//////////////////////////////////////////////////////////////////////////

	class BigConvexDataBuilder : public Ps::UserAllocated
	{
		public:
									BigConvexDataBuilder(const Gu::ConvexHullData* hull, BigConvexData* gm, const PxVec3* hullVerts);
									~BigConvexDataBuilder();
	// Support vertex map
				bool				precompute(PxU32 subdiv);				

				bool				initialize();				

				bool				save(PxOutputStream& stream, bool platformMismatch)	const;

				bool				computeValencies(const ConvexHullBuilder& meshBuilder);
	//~Support vertex map

	// Valencies

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Computes valencies and adjacent vertices.
		 *	After the call, get results with the appropriate accessors.
		 *
		 *	\param		vc		[in] creation structure
		 *	\return		true if success.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				bool				compute(const ValenciesCreate& vc)	const;

				bool				saveValencies(PxOutputStream& stream, bool platformMismatch)		const;
	//~Valencies
	protected:		
		PX_FORCE_INLINE void		precomputeSample(const PxVec3& dir, PxU8& startIndex, float negativeDir);

	private:
		const Gu::ConvexHullData*	mHull;
		BigConvexData*				mSVM;
		const	PxVec3*				mHullVerts;

	};

}

#endif // BIG_CONVEX_DATA_BUILDER_H
