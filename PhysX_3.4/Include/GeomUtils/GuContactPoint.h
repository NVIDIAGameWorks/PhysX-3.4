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

#ifndef GU_CONTACT_POINT_H
#define GU_CONTACT_POINT_H

/** \addtogroup geomutils
@{
*/

#include "foundation/PxVec3.h"

namespace physx
{
namespace Gu
{

struct ContactPoint
{
	/**
	\brief The normal of the contacting surfaces at the contact point.

	For two shapes s0 and s1, the normal points in the direction that s0 needs to move in to resolve the contact with s1.
	*/
	PX_ALIGN(16, PxVec3	normal);
	/**
	\brief The separation of the shapes at the contact point.  A negative separation denotes a penetration.
	*/
	PxReal	separation;

	/**
	\brief The point of contact between the shapes, in world space. 
	*/
	PX_ALIGN(16, PxVec3	point);	

	/**
	\brief The max impulse permitted at this point
	*/
	PxReal maxImpulse;

	PX_ALIGN(16, PxVec3 targetVel);

	/**
	\brief The static friction coefficient
	*/
	PxReal staticFriction;

	/**
	\brief Material flags for this contact (eDISABLE_FRICTION, eDISABLE_STRONG_FRICTION). @see PxMaterialFlag
	*/
	PxU8 materialFlags;

	/**
	\brief internal structure used for internal use only
	*/
	PxU16 forInternalUse;

	/**
	\brief The surface index of shape 1 at the contact point.  This is used to identify the surface material.

	\note This field is only supported by triangle meshes and heightfields, else it will be set to PXC_CONTACT_NO_FACE_INDEX.
	\note This value must be directly after internalFaceIndex0 in memory
	*/

	PxU32   internalFaceIndex1;

	/**
	\brief The dynamic friction coefficient
	*/
	PxReal dynamicFriction;
	/**
	\brief The restitution coefficient
	*/
	PxReal restitution;

};

}

}

/** @} */
#endif
