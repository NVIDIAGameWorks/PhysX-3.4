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



#ifndef DY_CORRELATIONBUFFER_H
#define DY_CORRELATIONBUFFER_H

#include "PxvConfig.h"
#include "foundation/PxSimpleTypes.h"
#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"
#include "DyFrictionPatch.h"
#include "GuContactBuffer.h"

namespace physx
{

struct PxcNpWorkUnit;
struct PxsMaterialInfo;

namespace Dy
{

struct CorrelationBuffer
{
	static const PxU32 MAX_FRICTION_PATCHES = 32;
	static const PxU16 LIST_END = 0xffff;

	struct ContactPatchData
	{
		PxU16 start;
		PxU16 next;
		PxU8 flags;
		PxU8 count;
		PxReal staticFriction, dynamicFriction, restitution;
	};

	// we can have as many contact patches as contacts, unfortunately
	ContactPatchData	contactPatches[Gu::ContactBuffer::MAX_CONTACTS];

	FrictionPatch	PX_ALIGN(16, frictionPatches[MAX_FRICTION_PATCHES]);
	PxVec3				PX_ALIGN(16, frictionPatchWorldNormal[MAX_FRICTION_PATCHES]);

	PxU32				frictionPatchContactCounts[MAX_FRICTION_PATCHES];
	PxU32				correlationListHeads[MAX_FRICTION_PATCHES+1];

	// contact IDs are only used to identify auxiliary contact data when velocity
	// targets have been set. 
	PxU16				contactID[MAX_FRICTION_PATCHES][2];

	PxU32 contactPatchCount, frictionPatchCount;

};

bool createContactPatches(CorrelationBuffer& fb, const Gu::ContactPoint* cb, PxU32 contactCount, PxReal normalTolerance);

bool correlatePatches(CorrelationBuffer& fb, 
					  const Gu::ContactPoint* cb,
					  const PxTransform& bodyFrame0,
					  const PxTransform& bodyFrame1,
					  PxReal normalTolerance,
					  PxU32 startContactPatchIndex,
					  PxU32 startFrictionPatchIndex);

void growPatches(CorrelationBuffer& fb,
				 const Gu::ContactPoint* buffer,
				 const PxTransform& bodyFrame0,
				 const PxTransform& bodyFrame1,
				 PxReal normalTolerance,
				 PxU32 frictionPatchStartIndex,
				 PxReal frictionOffsetThreshold);

}

}

#endif //DY_CORRELATIONBUFFER_H
