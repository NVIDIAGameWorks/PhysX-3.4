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


         
#ifndef PXC_NPWORKUNIT_H
#define PXC_NPWORKUNIT_H

#include "PxcNpThreadContext.h"
#include "PxcMaterialMethodImpl.h"
#include "PxcNpCache.h"

namespace physx
{

class PxvContact;

struct PxsRigidCore;
struct PxsShapeCore;

class PxsMaterialManager;

struct PxcNpWorkUnitFlag
{
	enum Enum
	{
		eOUTPUT_CONTACTS			= 1,
		eOUTPUT_CONSTRAINTS			= 2,
		eDISABLE_STRONG_FRICTION	= 4,
		eARTICULATION_BODY0			= 8,
		eARTICULATION_BODY1			= 16,
		eDYNAMIC_BODY0				= 32,
		eDYNAMIC_BODY1				= 64,
		eMODIFIABLE_CONTACT			= 128,
		eFORCE_THRESHOLD			= 256,
		eDETECT_DISCRETE_CONTACT	= 512,
		eHAS_KINEMATIC_ACTOR		= 1024,
		eDISABLE_RESPONSE			= 2048,
		eDETECT_CCD_CONTACTS		= 4096
	};
};

struct PxcNpWorkUnitStatusFlag
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
		eREFRESHED_WITH_TOUCH		= (1 << 6),
		eTOUCH_KNOWN				= eHAS_NO_TOUCH | eHAS_TOUCH	// The touch status is known (if narrowphase never ran for a pair then no flag will be set)
	};
};

/*
 * A struct to record the number of work units a particular constraint pointer references.
 * This is created at the beginning of the constriant data and is used to bypass constraint preparation when the 
 * bodies are not moving a lot. In this case, we can recycle the constraints and save ourselves some cycles.
*/
struct PxcNpWorkUnit;
struct PxcNpWorkUnitBatch
{
	PxcNpWorkUnit* mUnits[4];
	PxU32 mSize;
};

struct PxcNpWorkUnit
{
	
	const PxsRigidCore*		rigidCore0;					// INPUT								//4		//8
	const PxsRigidCore*		rigidCore1;					// INPUT								//8		//16
		
	const PxsShapeCore*		shapeCore0;					// INPUT								//12	//24
	const PxsShapeCore*		shapeCore1;					// INPUT								//16	//32

	PxU8*					ccdContacts;				// OUTPUT								//20	//40

	PxU8*					frictionDataPtr;			// INOUT								//24	//48

	PxU16					flags;						// INPUT								//26	//50
	PxU8					frictionPatchCount;			// INOUT 								//27	//51
	PxU8					statusFlags;				// OUTPUT (see PxcNpWorkUnitStatusFlag) //28	//52

	PxU8					dominance0;					// INPUT								//29	//53
	PxU8					dominance1;					// INPUT								//30	//54
	PxU8					geomType0;					// INPUT								//31	//55
	PxU8					geomType1;					// INPUT								//32	//56

	PxU32					index;						// INPUT								//36	//60

	PxReal					restDistance;				// INPUT								//40	//64

	PxU32					mTransformCache0;			//										//44	//68
	PxU32					mTransformCache1;			//										//48	//72
	
	PxU32					mEdgeIndex;					//inout the island gen edge index		//52	//76
	PxU32					mNpIndex;					//INPUT									//56	//80

};

//#if !defined(PX_P64)
//PX_COMPILE_TIME_ASSERT(0 == (sizeof(PxcNpWorkUnit) & 0x0f));
//#endif

PX_FORCE_INLINE void PxcNpWorkUnitClearContactState(PxcNpWorkUnit& n)
{
	n.ccdContacts = NULL;
}


PX_FORCE_INLINE void PxcNpWorkUnitClearCachedState(PxcNpWorkUnit& n)
{
	n.frictionDataPtr = 0;
	n.frictionPatchCount = 0;
	n.ccdContacts = NULL;
}

PX_FORCE_INLINE void PxcNpWorkUnitClearFrictionCachedState(PxcNpWorkUnit& n)
{
	n.frictionDataPtr = 0;
	n.frictionPatchCount = 0;
	n.ccdContacts = NULL;
}

#if !defined(PX_P64)
//PX_COMPILE_TIME_ASSERT(sizeof(PxcNpWorkUnit)==128);
#endif

}

#endif
