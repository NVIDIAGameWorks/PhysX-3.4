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

#ifndef GU_EPA_H
#define GU_EPA_H

#include "GuGJKUtil.h"
#include "GuGJKType.h"

namespace physx
{
  
namespace Gu
{
	//ML: The main entry point for EPA.
	// 
	//This function returns one of three status codes:
	//(1)EPA_FAIL:	the algorithm failed to create a valid polytope(the origin wasn't inside the polytope) from the input simplex.
	//(2)EPA_CONTACT : the algorithm found the MTD and converged successfully.
	//(3)EPA_DEGENERATE: the algorithm cannot make further progress and the result is unknown.

	GjkStatus epaPenetration(const GjkConvex& a, const GjkConvex& b, 					// two convexes, in the same space							   
							   PxU8* PX_RESTRICT aInd, PxU8* PX_RESTRICT bInd,			// warm start index points to create an initial simplex that EPA will work on
							   PxU8 _size,												// count of warm-start indices
							   Ps::aos::Vec3V& contactA, Ps::aos::Vec3V& contactB,		// a point on each body: when B is translated by normal*penetrationDepth, these are coincident
							   Ps::aos::Vec3V& normal, Ps::aos::FloatV& depth,			// MTD normal & penetration depth							    
							   const bool takeCoreShape = false);						// indicates whether we take support point from the core shape of the convexes
}

}

#endif
