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


#ifndef PXV_MANAGER_H
#define PXV_MANAGER_H

#include "foundation/PxVec3.h"
#include "foundation/PxQuat.h"
#include "foundation/PxTransform.h"
#include "foundation/PxMemory.h"
#include "PxvConfig.h"
#include "PxvGeometry.h"

namespace physx
{

class PxvContact;

/*!
\file
Manager interface
*/

/************************************************************************/
/* Managers                                                             */
/************************************************************************/

class PxsContactManager;
class PxsContext;

struct PxsRigidCore;
struct PxsShapeCore;



/*!
Type of PXD_MANAGER_CCD_MODE property
*/
enum PxvContactManagerCCDMode
{
	PXD_MANAGER_CCD_NONE,
	PXD_MANAGER_CCD_LINEAR
};


/*!
Manager descriptor
*/
struct PxvManagerDescRigidRigid
{
	/*!
	Manager user data

	\sa PXD_MANAGER_USER_DATA
	*/
	//void* userData;

	/*!
	Dominance setting for one way interactions.
	A dominance of 0 means the corresp. body will 
	not be pushable by the other body in the constraint.
	\sa PXD_MANAGER_DOMINANCE0
	*/
	PxU8		dominance0;
	
	/*!
	Dominance setting for one way interactions.
	A dominance of 0 means the corresp. body will 
	not be pushable by the other body in the constraint.
	\sa PXD_MANAGER_DOMINANCE1
	*/
	PxU8 	dominance1;

	/*!
	PxsRigidBodies
	*/
	PxsRigidBody*	rigidBody0;
	PxsRigidBody*	rigidBody1;

	/*!
	Shape Core structures
	*/

	const PxsShapeCore*	shapeCore0;
	const PxsShapeCore*	shapeCore1;

	/*!
	Body Core structures
	*/

	PxsRigidCore*	rigidCore0;
	PxsRigidCore*	rigidCore1;

	/*!
	Enable contact information reporting.

	*/
	int		reportContactInfo;

	/*!
	Enable contact impulse threshold reporting.

	*/
	int		hasForceThreshold;

	/*!
	Enable generated contacts to be changeable

	*/
	int		contactChangeable;

	/*!
	Disable strong friction

	*/
	//int		disableStrongFriction;

	/*!
	Contact resolution rest distance.

	*/
	PxReal		restDistance;

	/*!
	Disable contact response

	*/
	int		disableResponse;

	/*!
	Disable discrete contact generation

	*/
	int		disableDiscreteContact;

	/*!
	Disable CCD contact generation

	*/
	int		disableCCDContact;

	/*!
	Is connected to an articulation (1 - first body, 2 - second body)

	*/
	int		hasArticulations;

	/*!
	is connected to a dynamic (1 - first body, 2 - second body)
	*/
	int		hasDynamics;

	/*!
	Is the pair touching? Use when re-creating the manager with prior knowledge about touch status.
	
	positive: pair is touching
	0:        touch state unknown (this is a new pair)
	negative: pair is not touching

	Default is 0
	*/
	int		hasTouch;

	/*!
	Identifies whether body 1 is kinematic. We can treat kinematics as statics and embed velocity into constraint
	because kinematic bodies' velocities will not change
	*/
	bool    body1Kinematic;

	/*
	Index entries into the transform cache for shape 0
	*/

	PxU32 transformCache0;

	/*
	Index entries into the transform cache for shape 1
	*/

	PxU32 transformCache1;


	PxvManagerDescRigidRigid()
	{
		PxMemSet(this, 0, sizeof(PxvManagerDescRigidRigid));

		dominance0 = 1u;
		dominance1 = 1u;
	}
};


/*!
Report struct for contact manager touch reports
*/
struct PxvContactManagerTouchEvent
{
	/*!
	Manager handle
	*/
	PxsContactManager* manager;

	/*!
	Manager userdata
	*/
	void* userData;
};

}

#endif

