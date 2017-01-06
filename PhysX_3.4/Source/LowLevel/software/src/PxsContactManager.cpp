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


#include "PxsContactManager.h"
#include "PxsRigidBody.h"
#include "PxcContactMethodImpl.h"
#include "PxvManager.h"
#include "PxsIslandSim.h"

using namespace physx;

PxsContactManager::PxsContactManager(PxsContext*, PxU32 index) /*:
	mUserData	(NULL)*/
{
	mFlags = 0;

	// PT: TODO: any reason why we don't initialize all members here, e.g. shapeCore pointers?
	mNpUnit.index				= index;
	mNpUnit.rigidCore0			= NULL;
	mNpUnit.rigidCore1			= NULL;
	mNpUnit.restDistance		= 0;
	mNpUnit.dominance0			= 1u;
	mNpUnit.dominance1			= 1u;
	mNpUnit.frictionDataPtr		= NULL;
	mNpUnit.frictionPatchCount	= 0;
}

PxsContactManager::~PxsContactManager()
{
}


void PxsContactManager::setCCD(bool enable)
{
	PxU32 flags = mFlags & (~PXS_CM_CCD_CONTACT);
	if (enable)
		flags |= PXS_CM_CCD_LINEAR;
	else
		flags &= ~PXS_CM_CCD_LINEAR;

	mFlags = flags;
}



void PxsContactManager::resetCachedState()
{ 
	// happens when the body transform or shape relative transform changes.

	PxcNpWorkUnitClearCachedState(mNpUnit);
}

void PxsContactManager::resetFrictionCachedState()
{ 
	// happens when the body transform or shape relative transform changes.

	PxcNpWorkUnitClearFrictionCachedState(mNpUnit);
}


