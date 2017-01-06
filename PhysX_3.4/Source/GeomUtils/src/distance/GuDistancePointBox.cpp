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

#include "GuDistancePointBox.h"

using namespace physx;

PxReal Gu::distancePointBoxSquared(	const PxVec3& point, 
									const PxVec3& boxOrigin, const PxVec3& boxExtent, const PxMat33& boxBase, 
									PxVec3* boxParam)
{
	// Compute coordinates of point in box coordinate system
	const PxVec3 diff = point - boxOrigin;

	PxVec3 closest(	boxBase.column0.dot(diff),
					boxBase.column1.dot(diff),
					boxBase.column2.dot(diff));
	
	// Project test point onto box
	PxReal sqrDistance = 0.0f;
	for(PxU32 ax=0; ax<3; ax++) 
	{
		if(closest[ax] < -boxExtent[ax])
		{
			const PxReal delta = closest[ax] + boxExtent[ax];
			sqrDistance += delta*delta;
			closest[ax] = -boxExtent[ax];
		}
		else if(closest[ax] > boxExtent[ax])
		{
			const PxReal delta = closest[ax] - boxExtent[ax];
			sqrDistance += delta*delta;
			closest[ax] = boxExtent[ax];
		}
	}
	
	if(boxParam) *boxParam = closest;
	
	return sqrDistance;
}
