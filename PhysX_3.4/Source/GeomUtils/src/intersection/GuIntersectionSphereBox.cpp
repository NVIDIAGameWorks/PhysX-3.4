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

#include "GuIntersectionSphereBox.h"
#include "GuSphere.h"
#include "GuBox.h"

using namespace physx;

bool Gu::intersectSphereBox(const Sphere& sphere, const Box& box)
{
	const PxVec3 delta = sphere.center - box.center;
	PxVec3 dRot = box.rot.transformTranspose(delta);	//transform delta into OBB body coords. (use method call!)

	//check if delta is outside AABB - and clip the vector to the AABB.
	bool outside = false;

	if(dRot.x < -box.extents.x)
	{ 
		outside = true; 
		dRot.x = -box.extents.x;
	}
	else if(dRot.x >  box.extents.x)
	{ 
		outside = true; 
		dRot.x = box.extents.x;
	}

	if(dRot.y < -box.extents.y)
	{ 
		outside = true; 
		dRot.y = -box.extents.y;
	}
	else if(dRot.y >  box.extents.y)
	{ 
		outside = true; 
		dRot.y = box.extents.y;
	}

	if(dRot.z < -box.extents.z)
	{ 
		outside = true; 
		dRot.z = -box.extents.z;
	}
	else if(dRot.z >  box.extents.z)
	{ 
		outside = true; 
		dRot.z = box.extents.z;
	}

	if(outside)	//if clipping was done, sphere center is outside of box.
	{
		const PxVec3 clippedDelta = box.rot.transform(dRot);	//get clipped delta back in world coords.

		const PxVec3 clippedVec = delta - clippedDelta;			  //what we clipped away.	
		const PxReal lenSquared = clippedVec.magnitudeSquared();
		const PxReal radius = sphere.radius;
		if(lenSquared > radius * radius)	// PT: objects are defined as closed, so we return 'true' in case of equality
			return false;	//disjoint
	}
	return true;
}
