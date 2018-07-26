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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_CONTACTBUFFER_H
#define GU_CONTACTBUFFER_H

#include "PxPhysXConfig.h"
#include "PxContact.h"
#include "GuContactPoint.h"

namespace physx
{
namespace Gu
{

	struct NarrowPhaseParams
	{
		PX_FORCE_INLINE	NarrowPhaseParams(PxReal contactDistance, PxReal meshContactMargin, PxReal toleranceLength) :
				mContactDistance(contactDistance),
				mMeshContactMargin(meshContactMargin),
				mToleranceLength(toleranceLength)	{}

		PxReal	mContactDistance;
		PxReal	mMeshContactMargin;	// PT: Margin used to generate mesh contacts. Temp & unclear, should be removed once GJK is default path.
		PxReal	mToleranceLength;	// PT: copy of PxTolerancesScale::length
	};

//sizeof(SavedContactData)/sizeof(PxU32) = 17, 1088/17 = 64 triangles in the local array
#define LOCAL_CONTACTS_SIZE		1088

class ContactBuffer
{
public:

	static const PxU32 MAX_CONTACTS = 64;

	Gu::ContactPoint	contacts[MAX_CONTACTS];
	PxU32				count;
	PxU32				pad;

	PX_FORCE_INLINE void reset()
	{
		count = 0;
	}

	PX_FORCE_INLINE bool contact(const PxVec3& worldPoint, 
				 const PxVec3& worldNormalIn, 
				 PxReal separation, 
				 PxU32 faceIndex1 = PXC_CONTACT_NO_FACE_INDEX
				 )
	{
		PX_ASSERT(PxAbs(worldNormalIn.magnitude()-1)<1e-3f);

		if(count>=MAX_CONTACTS)
			return false;

		Gu::ContactPoint& p	= contacts[count++];
		p.normal			= worldNormalIn;
		p.point				= worldPoint;
		p.separation		= separation;
		p.internalFaceIndex1= faceIndex1;
		return true;
	}

	PX_FORCE_INLINE bool contact(const PxVec3& worldPoint,
		const PxVec3& worldNormalIn,
		PxReal separation,
		PxU16 internalUsage,
		PxU32 faceIndex1 = PXC_CONTACT_NO_FACE_INDEX
		)
	{
		PX_ASSERT(PxAbs(worldNormalIn.magnitude() - 1)<1e-3f);

		if (count >= MAX_CONTACTS)
			return false;

		Gu::ContactPoint& p = contacts[count++];
		p.normal = worldNormalIn;
		p.point = worldPoint;
		p.separation = separation;
		p.internalFaceIndex1 = faceIndex1;
		p.forInternalUse = internalUsage;
		return true;
	}

	PX_FORCE_INLINE bool contact(const Gu::ContactPoint & pt)
	{
		if(count>=MAX_CONTACTS)
			return false;
		contacts[count++] = pt;
		return true;
	}

	PX_FORCE_INLINE Gu::ContactPoint* contact()
	{
		if(count>=MAX_CONTACTS)
			return NULL;
		return &contacts[count++];
	}
};

}
}

#endif
