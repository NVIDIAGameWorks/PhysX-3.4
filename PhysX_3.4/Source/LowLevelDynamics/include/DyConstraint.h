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


#ifndef PXD_SHADER_H
#define PXD_SHADER_H

#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"
#include "PxvConfig.h"
#include "PxvDynamics.h"
#include "PxConstraint.h"
#include "DyConstraintWriteBack.h"

namespace physx
{

class PxsRigidBody;

namespace Dy
{


#if PX_VC 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif
PX_ALIGN_PREFIX(16)
struct Constraint
{
public:

	PxReal								linBreakForce;														//0
	PxReal								angBreakForce;														//4
	PxU32								constantBlockSize;													//8

	PxConstraintSolverPrep				solverPrep;															//12
	PxConstraintProject					project;															//16
	void*								constantBlock;														//20

	PxsRigidBody*						body0;																//24
	PxsRigidBody*						body1;																//28

	PxsBodyCore*						bodyCore0;															//32
	PxsBodyCore*						bodyCore1;															//36
	PxU32								flags;																//40
	PxU32								index;																//44 //this is also a constraint write back index
	PxReal								minResponseThreshold;												//48
}
PX_ALIGN_SUFFIX(16);
#if PX_VC 
     #pragma warning(pop) 
#endif

#if !PX_P64_FAMILY
PX_COMPILE_TIME_ASSERT(64==sizeof(Constraint));
#endif

}

}

#endif
