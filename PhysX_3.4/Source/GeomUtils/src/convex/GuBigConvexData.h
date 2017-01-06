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

#ifndef GU_BIG_CONVEX_DATA_H
#define GU_BIG_CONVEX_DATA_H

#include "foundation/PxSimpleTypes.h"

namespace physx
{

class BigConvexDataBuilder;
class PxcHillClimb;
class BigConvexData;

// Data

namespace Gu
{

struct Valency
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	PxU16		mCount;
	PxU16		mOffset;
};
PX_COMPILE_TIME_ASSERT(sizeof(Gu::Valency) == 4);

struct BigConvexRawData
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	// Support vertex map
	PxU16		mSubdiv;		// "Gaussmap" subdivision
	PxU16		mNbSamples;		// Total #samples in gaussmap PT: this is not even needed at runtime!

	PxU8*		mSamples;
	PX_FORCE_INLINE const PxU8*	getSamples2()	const
	{
		return mSamples + mNbSamples;
	}
	//~Support vertex map

	// Valencies data
	PxU32			mNbVerts;		//!< Number of vertices
	PxU32			mNbAdjVerts;	//!< Total number of adjacent vertices  ### PT: this is useless at runtime and should not be stored here
	Gu::Valency*	mValencies;		//!< A list of mNbVerts valencies (= number of neighbors)
	PxU8*			mAdjacentVerts;	//!< List of adjacent vertices
	//~Valencies data
};
#if PX_P64_FAMILY
PX_COMPILE_TIME_ASSERT(sizeof(Gu::BigConvexRawData) == 40);
#else
PX_COMPILE_TIME_ASSERT(sizeof(Gu::BigConvexRawData) == 24);
#endif

} // namespace Gu

}

#endif
