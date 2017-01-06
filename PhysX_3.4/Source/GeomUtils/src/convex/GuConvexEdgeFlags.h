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

#ifndef GU_CONVEX_EDGE_FLAGS_H
#define GU_CONVEX_EDGE_FLAGS_H

#include "CmPhysXCommon.h"

namespace physx
{
namespace Gu
{
	enum ExtraTrigDataFlag
	{
		ETD_CONVEX_EDGE_01	= (1<<3),	// PT: important value, don't change
		ETD_CONVEX_EDGE_12	= (1<<4),	// PT: important value, don't change
		ETD_CONVEX_EDGE_20	= (1<<5),	// PT: important value, don't change

		ETD_CONVEX_EDGE_ALL	= ETD_CONVEX_EDGE_01|ETD_CONVEX_EDGE_12|ETD_CONVEX_EDGE_20
	};

	// PT: helper function to make sure we use the proper default flags everywhere
	PX_FORCE_INLINE PxU8 getConvexEdgeFlags(const PxU8* extraTrigData, PxU32 triangleIndex)
	{
		return extraTrigData ? extraTrigData[triangleIndex] : PxU8(ETD_CONVEX_EDGE_ALL);
	}
}
}

#endif
