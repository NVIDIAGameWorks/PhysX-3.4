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

#ifndef GU_DISTANCE_POINT_BOX_H
#define GU_DISTANCE_POINT_BOX_H

#include "GuBox.h"

namespace physx
{
namespace Gu
{
	/**
	Return the square of the minimum distance from the surface of the box to the given point.
	\param point The point
	\param boxOrigin The origin of the box
	\param boxExtent The extent of the box
	\param boxBase The orientation of the box
	\param boxParam Set to coordinates of the closest point on the box in its local space
	*/
	PxReal distancePointBoxSquared(	const PxVec3& point,
									const PxVec3& boxOrigin, 
									const PxVec3& boxExtent, 
									const PxMat33& boxBase, 
									PxVec3* boxParam=NULL);

	/**
	Return the square of the minimum distance from the surface of the box to the given point.
	\param point The point
	\param box The box
	\param boxParam Set to coordinates of the closest point on the box in its local space
	*/
	PX_FORCE_INLINE PxReal distancePointBoxSquared(	const PxVec3& point, 
													const Gu::Box& box, 
													PxVec3* boxParam=NULL)
	{
		return distancePointBoxSquared(point, box.center, box.extents, box.rot, boxParam);
	}

} // namespace Gu

}

#endif
