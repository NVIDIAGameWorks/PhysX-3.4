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


#ifndef PXC_RIGIDBODY_H
#define PXC_RIGIDBODY_H

#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"
#include "PxvDynamics.h"
#include "CmSpatialVector.h"

namespace physx
{

class PxsContactManager;
struct PxsCCDPair;
struct PxsCCDBody;

#define PX_INTERNAL_LOCK_FLAG_START 7

PX_ALIGN_PREFIX(16)
class PxcRigidBody
{
public:

	enum PxcRigidBodyFlag
	{
		eFROZEN =						1 << 0,			//This flag indicates that the stabilization is enabled and the body is
														//"frozen". By "frozen", we mean that the body's transform is unchanged
														//from the previous frame. This permits various optimizations.
		eFREEZE_THIS_FRAME =			1 << 1,
		eUNFREEZE_THIS_FRAME =			1 << 2,
		eACTIVATE_THIS_FRAME =			1 << 3,
		eDEACTIVATE_THIS_FRAME =		1 << 4,
		eDISABLE_GRAVITY =				1 << 5,
		eSPECULATIVE_CCD =				1 << 6,
		//KS - copied here for GPU simulation to avoid needing to pass another set of flags around.
		eLOCK_LINEAR_X =				1 << (PX_INTERNAL_LOCK_FLAG_START),
		eLOCK_LINEAR_Y =				1 << (PX_INTERNAL_LOCK_FLAG_START + 1),
		eLOCK_LINEAR_Z =				1 << (PX_INTERNAL_LOCK_FLAG_START + 2),
		eLOCK_ANGULAR_X =				1 << (PX_INTERNAL_LOCK_FLAG_START + 3),
		eLOCK_ANGULAR_Y =				1 << (PX_INTERNAL_LOCK_FLAG_START + 4),
		eLOCK_ANGULAR_Z =				1 << (PX_INTERNAL_LOCK_FLAG_START + 5)

	};

	PX_FORCE_INLINE PxcRigidBody(PxsBodyCore* core)  
	: mLastTransform(core->body2World),
	  mCCD(NULL),
	  mCore(core)
	{
	}

	void						adjustCCDLastTransform();

protected:
	
	~PxcRigidBody()
	{
	}

public:
	
	PxTransform					mLastTransform;				//28 (28)

	PxU16						mInternalFlags;				//30 (30)
	PxU16						solverIterationCounts;		//32 (32)

	PxsCCDBody*					mCCD;						//36 (40)	// only valid during CCD	

	PxsBodyCore*				mCore;						//40 (48)
	
#if !PX_P64_FAMILY
	PxU32						alignmentPad[2];			//48 (48)
#endif

	PxVec3						sleepLinVelAcc;				//60 (60)
	PxReal						freezeCount;				//64 (64)
	   
	PxVec3						sleepAngVelAcc;				//76 (76)
	PxReal						accelScale;					//80 (80)
	

}
PX_ALIGN_SUFFIX(16);
PX_COMPILE_TIME_ASSERT(0 == (sizeof(PxcRigidBody) & 0x0f));

}

#endif //PXC_RIGIDBODY_H
