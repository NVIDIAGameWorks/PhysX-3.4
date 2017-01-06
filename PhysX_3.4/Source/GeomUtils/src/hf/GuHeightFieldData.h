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

#ifndef GU_HEIGHTFIELD_DATA_H
#define GU_HEIGHTFIELD_DATA_H

#include "foundation/PxSimpleTypes.h"
#include "PxHeightFieldFlag.h"
#include "PxHeightFieldSample.h"
#include "GuCenterExtents.h"

namespace physx
{

namespace Gu
{

#if PX_VC 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
struct PX_PHYSX_COMMON_API HeightFieldData
{
// PX_SERIALIZATION
	PX_FORCE_INLINE								HeightFieldData()									{}
	PX_FORCE_INLINE								HeightFieldData(const PxEMPTY) :	flags(PxEmpty)	{}
//~PX_SERIALIZATION

	//properties
		// PT: WARNING: bounds must be followed by at least 32bits of data for safe SIMD loading
					CenterExtents				mAABB;
					PxU32						rows;					// PT: WARNING: don't change this member's name (used in ConvX)
					PxU32						columns;				// PT: WARNING: don't change this member's name (used in ConvX)
					PxReal						rowLimit;				// PT: to avoid runtime int-to-float conversions on Xbox
					PxReal						colLimit;				// PT: to avoid runtime int-to-float conversions on Xbox
					PxReal						nbColumns;				// PT: to avoid runtime int-to-float conversions on Xbox
					PxHeightFieldSample*		samples;				// PT: WARNING: don't change this member's name (used in ConvX)
					PxReal						thickness;
					PxReal						convexEdgeThreshold;

					PxHeightFieldFlags			flags;

					PxHeightFieldFormat::Enum	format;

	PX_FORCE_INLINE	const CenterExtentsPadded&	getPaddedBounds()				const
												{
													// PT: see compile-time assert below
													return static_cast<const CenterExtentsPadded&>(mAABB);
												}
};
#if PX_VC 
     #pragma warning(pop) 
#endif

	// PT: 'getPaddedBounds()' is only safe if we make sure the bounds member is followed by at least 32bits of data
	PX_COMPILE_TIME_ASSERT(PX_OFFSET_OF(Gu::HeightFieldData, rows)>=PX_OFFSET_OF(Gu::HeightFieldData, mAABB)+4);

} // namespace Gu

}

#endif
