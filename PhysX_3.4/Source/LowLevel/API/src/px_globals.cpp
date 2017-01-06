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

#include "PxvGlobals.h"
#include "PxsContext.h"
#include "PxcContactMethodImpl.h"
#include "GuContactMethodImpl.h"


#if PX_SUPPORT_GPU_PHYSX
#include "PxPhysXGpu.h"

physx::PxPhysXGpu* gPxPhysXGpu;
#endif

namespace physx
{

PxvOffsetTable gPxvOffsetTable;

bool PxcLegacyContactSphereHeightField	(GU_CONTACT_METHOD_ARGS);
bool PxcLegacyContactCapsuleHeightField	(GU_CONTACT_METHOD_ARGS);
bool PxcLegacyContactBoxHeightField		(GU_CONTACT_METHOD_ARGS);
bool PxcLegacyContactConvexHeightField	(GU_CONTACT_METHOD_ARGS);

bool PxcPCMContactSphereHeightField		(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactCapsuleHeightField	(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactBoxHeightField		(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactConvexHeightField		(GU_CONTACT_METHOD_ARGS);

bool PxcContactSphereHeightField		(GU_CONTACT_METHOD_ARGS);
bool PxcContactCapsuleHeightField		(GU_CONTACT_METHOD_ARGS);
bool PxcContactBoxHeightField			(GU_CONTACT_METHOD_ARGS);
bool PxcContactConvexHeightField		(GU_CONTACT_METHOD_ARGS);

bool gUnifiedHeightfieldCollision = false;

void PxvRegisterHeightFields()
{
	g_ContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]			= PxcContactSphereHeightField;
	g_ContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcContactCapsuleHeightField;
	g_ContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcContactBoxHeightField;
	g_ContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]		= PxcContactConvexHeightField;

	g_PCMContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]		= PxcPCMContactSphereHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcPCMContactCapsuleHeightField;
	g_PCMContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcPCMContactBoxHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]	= PxcPCMContactConvexHeightField;
	gUnifiedHeightfieldCollision = true;
}

void PxvRegisterLegacyHeightFields()
{
	g_ContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]			= PxcLegacyContactSphereHeightField;
	g_ContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcLegacyContactCapsuleHeightField;
	g_ContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcLegacyContactBoxHeightField;
	g_ContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]		= PxcLegacyContactConvexHeightField;

	g_PCMContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]		= PxcLegacyContactSphereHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcLegacyContactCapsuleHeightField;
	g_PCMContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcLegacyContactBoxHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]	= PxcLegacyContactConvexHeightField;
	gUnifiedHeightfieldCollision = false;
}

void PxvInit(const PxvOffsetTable& offsetTable)
{
#if PX_SUPPORT_GPU_PHYSX
	gPxPhysXGpu = NULL;
#endif
	gPxvOffsetTable = offsetTable;
}

void PxvTerm()
{
#if PX_SUPPORT_GPU_PHYSX
	if (gPxPhysXGpu)
	{
		gPxPhysXGpu->release();
		gPxPhysXGpu = NULL;
	}
#endif
}

}

#if PX_SUPPORT_GPU_PHYSX
namespace physx
{
	//forward declare stuff from PxPhysXGpuModuleLoader.cpp
	void PxLoadPhysxGPUModule(const char* appGUID);
	typedef physx::PxPhysXGpu* (PxCreatePhysXGpu_FUNC)();
	extern PxCreatePhysXGpu_FUNC* g_PxCreatePhysXGpu_Func;

	PxPhysXGpu* PxvGetPhysXGpu(bool createIfNeeded)
	{
		if (!gPxPhysXGpu && createIfNeeded)
		{
			PxLoadPhysxGPUModule(NULL);
			if (g_PxCreatePhysXGpu_Func)
			{
				gPxPhysXGpu = g_PxCreatePhysXGpu_Func();
			}
		}

		return gPxPhysXGpu;
	}
}
#endif
