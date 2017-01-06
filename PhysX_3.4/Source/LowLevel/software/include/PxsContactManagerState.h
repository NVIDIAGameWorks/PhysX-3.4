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

#ifndef PXS_CONTACT_MANAGER_STATE_H
#define PXS_CONTACT_MANAGER_STATE_H

#include "foundation/PxSimpleTypes.h"

namespace physx
{

	struct PxsShapeCore;

	/**
	There is an implicit 1:1 mapping between PxgContactManagerInput and PxsContactManagerOutput. The structures are split because PxgNpContactManagerInput contains constant
	data that is produced by the CPU code and PxgNpContactManagerOutput contains per-frame contact information produced by the NP.

	There is also a 1:1 mapping between the PxgNpContactManager and PxsContactManager. This mapping is handled within the PxgNPhaseCore.

	The previous contact states are implicitly cached in PxsContactManager and will be propagated to the solver. Friction correlation is also done implicitly using cached 
	information in PxsContactManager.
	The NP will produce a list of pairs that found/lost patches for the solver along with updating the PxgNpContactManagerOutput for all pairs.
	*/

	struct PxsContactManagerStatusFlag
	{
		enum Enum
		{
			eHAS_NO_TOUCH				= (1 << 0),
			eHAS_TOUCH					= (1 << 1),
			//eHAS_SOLVER_CONSTRAINTS		= (1 << 2),
			eREQUEST_CONSTRAINTS		= (1 << 3),
			eHAS_CCD_RETOUCH			= (1 << 4),	// Marks pairs that are touching at a CCD pass and were touching at discrete collision or at a previous CCD pass already
													// but we can not tell whether they lost contact in a pass before. We send them as pure eNOTIFY_TOUCH_CCD events to the 
													// contact report callback if requested.
			eDIRTY_MANAGER				= (1 << 5),
			eTOUCH_KNOWN				= eHAS_NO_TOUCH | eHAS_TOUCH	// The touch status is known (if narrowphase never ran for a pair then no flag will be set)
		};
	};
		

	struct PX_ALIGN_PREFIX(16) PxsContactManagerOutput
	{
		PxU8* contactPatches;				//Start index/ptr for contact patches
		PxU8* contactPoints;				//Start index/ptr for contact points
		PxReal* contactForces;				//Start index/ptr for contact forces
		PxU8 nbContacts;					//Num contacts
		PxU8 nbPatches;						//Num patches
		PxU8 statusFlag;					//Status flag (has touch etc.)
		PxU8 prevPatches;					//Previous number of patches

		PX_FORCE_INLINE PxU32* getInternalFaceIndice()
		{
			return reinterpret_cast<PxU32*>(contactForces + nbContacts);
		}
	} 
	PX_ALIGN_SUFFIX(16);

	struct /*PX_ALIGN_PREFIX(16)*/ PxsContactManagerPersistency
	{
		PxU8 mPrevPatches;
		PxU8 mNbFrictionPatches;
		PxU8 mNbPrevFrictionPatches;
	}
	/*PX_ALIGN_SUFFIX(16)*/;

}

#endif //PXG_CONTACT_MANAGER_H
