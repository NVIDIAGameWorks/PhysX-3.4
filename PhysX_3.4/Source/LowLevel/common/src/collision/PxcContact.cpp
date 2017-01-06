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

#include "PxcContactMethodImpl.h"

namespace physx
{

////////////////////////////////////////////////////////////////////////////////////////////////////////
//non-pcm sphere function
bool PxcContactSphereSphere(GU_CONTACT_METHOD_ARGS)
{
	return contactSphereSphere(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactSphereCapsule(GU_CONTACT_METHOD_ARGS)
{
	return contactSphereCapsule(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactSphereBox(GU_CONTACT_METHOD_ARGS)
{
	return contactSphereBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactSpherePlane(GU_CONTACT_METHOD_ARGS)
{
	return contactSpherePlane(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactSphereConvex(GU_CONTACT_METHOD_ARGS)
{
	return contactCapsuleConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactSphereHeightField(GU_CONTACT_METHOD_ARGS)
{
	return contactSphereHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactSphereMesh(GU_CONTACT_METHOD_ARGS)
{
	return contactSphereMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//non-pcm plane functions
bool PxcContactPlaneBox(GU_CONTACT_METHOD_ARGS)
{
	return contactPlaneBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactPlaneCapsule(GU_CONTACT_METHOD_ARGS)
{
	return contactPlaneCapsule(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactPlaneConvex(GU_CONTACT_METHOD_ARGS)
{
	return contactPlaneConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//non-pcm capsule funtions
bool PxcContactCapsuleCapsule(GU_CONTACT_METHOD_ARGS)
{
	return contactCapsuleCapsule(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactCapsuleBox(GU_CONTACT_METHOD_ARGS)
{
	return contactCapsuleBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactCapsuleConvex(GU_CONTACT_METHOD_ARGS)
{
	return contactCapsuleConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactCapsuleHeightField(GU_CONTACT_METHOD_ARGS)
{
	return contactCapsuleHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactCapsuleMesh(GU_CONTACT_METHOD_ARGS)
{
	return contactCapsuleMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//non-pcm box functions
bool PxcContactBoxBox(GU_CONTACT_METHOD_ARGS)
{
	return contactBoxBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactBoxConvex(GU_CONTACT_METHOD_ARGS)
{
	return contactBoxConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactBoxHeightField(GU_CONTACT_METHOD_ARGS)
{
	return contactBoxHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactBoxMesh(GU_CONTACT_METHOD_ARGS)
{
	return contactBoxMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//non-pcm convex functions
bool PxcContactConvexConvex(GU_CONTACT_METHOD_ARGS)
{
	return contactConvexConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactConvexHeightField(GU_CONTACT_METHOD_ARGS)
{
	return contactConvexHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcContactConvexMesh(GU_CONTACT_METHOD_ARGS)
{
	return contactConvexMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//legacy height field functions
bool PxcLegacyContactSphereHeightField(GU_CONTACT_METHOD_ARGS)
{
	return legacyContactSphereHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcLegacyContactCapsuleHeightField(GU_CONTACT_METHOD_ARGS)
{
	return legacyContactCapsuleHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcLegacyContactBoxHeightField(GU_CONTACT_METHOD_ARGS)
{
	return legacyContactBoxHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcLegacyContactConvexHeightField(GU_CONTACT_METHOD_ARGS)
{
	return legacyContactConvexHeightfield(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//pcm sphere functions
bool PxcPCMContactSphereSphere(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactSphereSphere(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactSpherePlane(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactSpherePlane(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactSphereCapsule(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactSphereCapsule(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactSphereBox(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactSphereBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactSphereConvex(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactSphereConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactSphereHeightField(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactSphereHeightField(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactSphereMesh(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactSphereMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//pcm plane functions
bool PxcPCMContactPlaneBox(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactPlaneBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactPlaneCapsule(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactPlaneCapsule(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactPlaneConvex(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactPlaneConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//pcm capsule functions
bool PxcPCMContactCapsuleCapsule(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactCapsuleCapsule(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactCapsuleBox(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactCapsuleBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactCapsuleConvex(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactCapsuleConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactCapsuleHeightField(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactCapsuleHeightField(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactCapsuleMesh(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactCapsuleMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//pcm box functions
bool PxcPCMContactBoxBox(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactBoxBox(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactBoxConvex(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactBoxConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactBoxHeightField(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactBoxHeightField(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactBoxMesh(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactBoxMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

//pcm convex functions
bool PxcPCMContactConvexConvex(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactConvexConvex(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactConvexHeightField(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactConvexHeightField(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

bool PxcPCMContactConvexMesh(GU_CONTACT_METHOD_ARGS)
{
	return pcmContactConvexMesh(shape0, shape1, transform0, transform1, params, cache, contactBuffer, renderOutput);
}

}
