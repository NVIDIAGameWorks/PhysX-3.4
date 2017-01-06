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


#ifndef PXV_DYNAMICS_H
#define PXV_DYNAMICS_H

#include "foundation/PxVec3.h"
#include "foundation/PxQuat.h"
#include "foundation/PxTransform.h"
#include "foundation/PxSimpleTypes.h"
#include "PsIntrinsics.h"
#include "PxRigidDynamic.h"

namespace physx
{

/*!
\file
Dynamics interface.
*/

/************************************************************************/
/* Atoms                                                                */
/************************************************************************/

class PxsContext;
class PxsRigidBody;
class PxShape;
class PxGeometry;
struct PxsShapeCore;


struct PxsRigidCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	PxsRigidCore()	:		mFlags(0), mIdtBody2Actor(0)			{}
	PxsRigidCore(const PxEMPTY) : mFlags(PxEmpty)					{}

	PX_ALIGN_PREFIX(16)
	PxTransform			body2World PX_ALIGN_SUFFIX(16);
	PxRigidBodyFlags	mFlags;					// API body flags
	PxU8				mIdtBody2Actor;			// PT: true if PxsBodyCore::body2Actor is identity
	PxU16				solverIterationCounts;	//vel iters are in low word and pos iters in high word.

	PX_FORCE_INLINE	PxU32 isKinematic() const
	{
		return mFlags & PxRigidBodyFlag::eKINEMATIC;
	}

	PX_FORCE_INLINE PxU32 hasCCD() const
	{
		return mFlags & PxRigidBodyFlag::eENABLE_CCD;
	}

	PX_FORCE_INLINE	PxU32 hasCCDFriction() const
	{
		return mFlags & PxRigidBodyFlag::eENABLE_CCD_FRICTION;
	}
};
PX_COMPILE_TIME_ASSERT(sizeof(PxsRigidCore) == 32);


struct PxsBodyCore: public PxsRigidCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	PxsBodyCore() : PxsRigidCore()							{}
	PxsBodyCore(const PxEMPTY) : PxsRigidCore(PxEmpty)		{}

	PX_FORCE_INLINE	const PxTransform& getBody2Actor()	const	{ return body2Actor;	}
	PX_FORCE_INLINE	void setBody2Actor(const PxTransform& t)
	{
		mIdtBody2Actor = PxU8(t.p.isZero() && t.q.isIdentity());

		body2Actor = t;
	}
	protected:
	PxTransform				body2Actor;
	public:
	PxReal					ccdAdvanceCoefficient;		//64

	PxVec3					linearVelocity;
	PxReal					maxPenBias;

	PxVec3					angularVelocity;
	PxReal					contactReportThreshold;		//96
    
	PxReal					maxAngularVelocitySq;
	PxReal					maxLinearVelocitySq;
	PxReal					linearDamping;
	PxReal					angularDamping;				//112

	PxVec3					inverseInertia;
	PxReal					inverseMass;				//128
	
	PxReal					maxContactImpulse;			
	PxReal					sleepThreshold;				   
	PxReal					freezeThreshold;			
	PxReal					wakeCounter;				//144 this is authoritative wakeCounter

	PxReal					solverWakeCounter;			//this is calculated by the solver when it performs sleepCheck. It is committed to wakeCounter in ScAfterIntegrationTask if the body is still awake.
	PxU32					numCountedInteractions;
	PxU32					numBodyInteractions;		//Used by adaptive force to keep track of the total number of body interactions
	PxU16					isFastMoving;				//This could be a single bit but it's a u32 at the moment for simplicity's sake
	PxRigidDynamicLockFlags	lockFlags;					//160 This could be a u8 but it is a u32 for simplicity's sake. All fits into 16 byte alignment

	PX_FORCE_INLINE	bool	shouldCreateContactReports()	const
	{
		const PxU32* binary = reinterpret_cast<const PxU32*>(&contactReportThreshold);
		return *binary != 0x7f7fffff;	// PX_MAX_REAL
	}
};

PX_COMPILE_TIME_ASSERT(sizeof(PxsBodyCore) == 160);


}

#endif
