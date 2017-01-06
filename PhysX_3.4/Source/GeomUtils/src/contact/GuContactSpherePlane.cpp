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

#include "GuContactBuffer.h"
#include "GuContactMethodImpl.h"
#include "GuGeometryUnion.h"

namespace physx
{
namespace Gu
{
bool contactSpherePlane(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(renderOutput);
	PX_UNUSED(cache);
	PX_UNUSED(shape1);

	// Get actual shape data
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	//const PxPlaneGeometry& shapePlane = shape1.get<const PxPlaneGeometry>();

	//Sphere in plane space
	const PxVec3 sphere = transform1.transformInv(transform0.p);
	
	//Make sure we have a normalized plane
	//The plane is implicitly n=<1,0,0> d=0 (in plane-space)
	//PX_ASSERT(PxAbs(shape1.mNormal.magnitudeSquared() - 1.0f) < 0.000001f);

	//Separation
	const PxReal separation = sphere.x - shapeSphere.radius;

	if(separation<=params.mContactDistance)
	{
		const PxVec3 normal = transform1.q.getBasisVector0();
		const PxVec3 point  = transform0.p - normal * shapeSphere.radius;
		contactBuffer.contact(point, normal, separation);
		return true;
	}
	return false;
}
}//Gu
}//physx
