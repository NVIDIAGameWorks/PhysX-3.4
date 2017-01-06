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


#ifndef PXD_INIT_H
#define PXD_INIT_H

#include "PxvConfig.h"
#include "PsBasicTemplates.h"

namespace physx
{

/*!
\file
PhysX Low-level, Memory management
*/

/************************************************************************/
/* Error Handling                                                       */
/************************************************************************/


enum PxvErrorCode
{
	PXD_ERROR_NO_ERROR = 0,
	PXD_ERROR_INVALID_PARAMETER,
	PXD_ERROR_INVALID_PARAMETER_SIZE,
	PXD_ERROR_INTERNAL_ERROR,
	PXD_ERROR_NOT_IMPLEMENTED,
	PXD_ERROR_NO_CONTEXT,
	PXD_ERROR_NO_TASK_MANAGER,
	PXD_ERROR_WARNING
};

class PxShape;
class PxRigidActor;
struct PxsShapeCore;
struct PxsRigidCore;

struct PxvOffsetTable
{
	PX_FORCE_INLINE PxvOffsetTable() {}

	PX_FORCE_INLINE const PxShape* convertPxsShape2Px(const PxsShapeCore* pxs) const
	{
		return shdfnd::pointerOffset<const PxShape*>(pxs, pxsShapeCore2PxShape); 
	}

	PX_FORCE_INLINE const PxRigidActor* convertPxsRigidCore2PxRigidBody(const PxsRigidCore* pxs) const
	{
		return shdfnd::pointerOffset<const PxRigidActor*>(pxs, pxsRigidCore2PxRigidBody); 
	}

	PX_FORCE_INLINE const PxRigidActor* convertPxsRigidCore2PxRigidStatic(const PxsRigidCore* pxs) const
	{
		return shdfnd::pointerOffset<const PxRigidActor*>(pxs, pxsRigidCore2PxRigidStatic); 
	}

	ptrdiff_t	pxsShapeCore2PxShape;
	ptrdiff_t	pxsRigidCore2PxRigidBody;
	ptrdiff_t	pxsRigidCore2PxRigidStatic;
};
extern PxvOffsetTable gPxvOffsetTable;

/*!
Initialize low-level implementation.
*/

void PxvInit(const PxvOffsetTable& offsetTable);


/*!
Shut down low-level implementation.
*/
void PxvTerm();

/*!
Initialize low-level implementation.
*/

void PxvRegisterHeightFields();

void PxvRegisterLegacyHeightFields();

#if PX_SUPPORT_GPU_PHYSX
class PxPhysXGpu* PxvGetPhysXGpu(bool createIfNeeded);
#endif

}

#endif
