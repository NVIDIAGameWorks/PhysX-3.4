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


#ifndef PX_PHYSICS_SCP_INTERACTION_FLAGS
#define PX_PHYSICS_SCP_INTERACTION_FLAGS

#include "foundation/Px.h"

namespace physx
{

namespace Sc
{
	struct InteractionFlag	// PT: TODO: use PxFlags
	{
		enum Enum
		{
			eELEMENT_ELEMENT	= (1 << 0), // Interactions between rigid body shapes or interactions between rigid body and particle packet shapes
			eRB_ELEMENT			= (1 << 1) | eELEMENT_ELEMENT, // Interactions between rigid body shapes
			eCONSTRAINT			= (1 << 2),
			eFILTERABLE			= (1 << 3), // Interactions that go through the filter code
			eIN_DIRTY_LIST		= (1 << 4),	// The interaction is in the dirty list
			eIS_FILTER_PAIR		= (1 << 5),	// The interaction is tracked by the filter callback mechanism
			eIS_ACTIVE			= (1 << 6)
		};
	};

	struct InteractionDirtyFlag
	{
		enum Enum
		{
			eFILTER_STATE		= (1 << 0), // All changes filtering related
			eMATERIAL			= (1 << 1),
			eBODY_KINEMATIC		= (1 << 2) | eFILTER_STATE,  // A transition between dynamic and kinematic (and vice versa) require a refiltering
			eDOMINANCE			= (1 << 3),
			eREST_OFFSET		= (1 << 4),
			eVISUALIZATION		= (1 << 5)
		};
	};


} // namespace Sc


} // namespace physx


#endif // PX_PHYSICS_SCP_INTERACTION_FLAGS
