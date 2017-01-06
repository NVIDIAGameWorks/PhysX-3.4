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

#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"
#include "GuGeometryUnion.h"

using namespace physx;

//This version is ported 1:1 from novodex
static PX_FORCE_INLINE bool ContactSphereBox(const PxVec3& sphereOrigin, 
							 PxReal sphereRadius,
							 const PxVec3& boxExtents,
//							 const PxcCachedTransforms& boxCacheTransform, 
							 const PxTransform& boxTransform, 
							 PxVec3& point, 
							 PxVec3& normal, 
							 PxReal& separation, 
							 PxReal contactDistance)
{
//	const PxTransform& boxTransform = boxCacheTransform.getShapeToWorld();

	//returns true on contact
	const PxVec3 delta = sphereOrigin - boxTransform.p; // s1.center - s2.center;
	PxVec3 dRot = boxTransform.rotateInv(delta); //transform delta into OBB body coords.

	//check if delta is outside ABB - and clip the vector to the ABB.
	bool outside = false;

	if (dRot.x < -boxExtents.x)
	{ 
		outside = true; 
		dRot.x = -boxExtents.x;
	}
	else if (dRot.x >  boxExtents.x)
	{ 
		outside = true; 
		dRot.x = boxExtents.x;
	}

	if (dRot.y < -boxExtents.y)
	{ 
		outside = true; 
		dRot.y = -boxExtents.y;
	}
	else if (dRot.y >  boxExtents.y)
	{ 
		outside = true; 
		dRot.y = boxExtents.y;
	}

	if (dRot.z < -boxExtents.z)
	{ 
		outside = true; 
		dRot.z =-boxExtents.z;
	}
	else if (dRot.z >  boxExtents.z)
	{ 
		outside = true; 
		dRot.z = boxExtents.z;
	}

	if (outside) //if clipping was done, sphere center is outside of box.
	{
		point = boxTransform.rotate(dRot); //get clipped delta back in world coords.
		normal = delta - point; //what we clipped away.	
		const PxReal lenSquared = normal.magnitudeSquared();
		const PxReal inflatedDist = sphereRadius + contactDistance;
		if (lenSquared > inflatedDist * inflatedDist) 
			return false;	//disjoint

		//normalize to make it into the normal:
		separation = PxRecipSqrt(lenSquared);
		normal *= separation;	
		separation *= lenSquared;
		//any plane that touches the sphere is tangential, so a vector from contact point to sphere center defines normal.
		//we could also use point here, which has same direction.
		//this is either a faceFace or a vertexFace contact depending on whether the box's face or vertex collides, but we did not distinguish. 
		//We'll just use vertex face for now, this info isn't really being used anyway.
		//contact point is point on surface of cube closest to sphere center.
		point += boxTransform.p;
		separation -= sphereRadius;
		return true;
	}
	else
	{
		//center is in box, we definitely have a contact.
		PxVec3 locNorm;	//local coords contact normal

		/*const*/ PxVec3 absdRot;
		absdRot = PxVec3(PxAbs(dRot.x), PxAbs(dRot.y), PxAbs(dRot.z));
		/*const*/ PxVec3 distToSurface = boxExtents - absdRot;	//dist from embedded center to box surface along 3 dimensions.

		//find smallest element of distToSurface
		if (distToSurface.y < distToSurface.x)
		{
			if (distToSurface.y < distToSurface.z)
			{
				//y
				locNorm = PxVec3(0.0f, dRot.y > 0.0f ? 1.0f : -1.0f, 0.0f);
				separation = -distToSurface.y;
			}
			else
			{
				//z
				locNorm = PxVec3(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
				separation = -distToSurface.z;
			}
		}
		else
		{
			if (distToSurface.x < distToSurface.z)
			{
				//x
				locNorm = PxVec3(dRot.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
				separation = -distToSurface.x;
			}
			else
			{
				//z
				locNorm = PxVec3(0.0f,0.0f, dRot.z > 0.0f ? 1.0f : -1.0f);
				separation = -distToSurface.z;
			}
		}
		//separation so far is just the embedding of the center point; we still have to push out all of the radius.
		point = sphereOrigin;
		normal = boxTransform.rotate(locNorm);
		separation -= sphereRadius;
		return true;
	}
}

namespace physx
{
namespace Gu
{
bool contactSphereBox(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(renderOutput);
	PX_UNUSED(cache);

	const PxSphereGeometry& sphereGeom = shape0.get<const PxSphereGeometry>();
	const PxBoxGeometry& boxGeom = shape1.get<const PxBoxGeometry>();

	PxVec3 normal;
	PxVec3 point;
	PxReal separation;
	if(!ContactSphereBox(transform0.p, sphereGeom.radius, boxGeom.halfExtents, transform1, point, normal, separation, params.mContactDistance))
		return false;

	contactBuffer.contact(point, normal, separation);
	return true;
}
}//Gu
}//physx
