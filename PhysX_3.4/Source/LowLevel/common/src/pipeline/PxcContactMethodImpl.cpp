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


#include "PxGeometry.h"
#include "PxcContactMethodImpl.h"

namespace physx
{

// PT: those prototypes shouldn't be public. Keep them here.

// Sphere - other
bool PxcContactSphereSphere				(GU_CONTACT_METHOD_ARGS);
bool PxcContactSpherePlane				(GU_CONTACT_METHOD_ARGS);
bool PxcContactSphereCapsule			(GU_CONTACT_METHOD_ARGS);
bool PxcContactSphereBox				(GU_CONTACT_METHOD_ARGS);
bool PxcContactSphereConvex				(GU_CONTACT_METHOD_ARGS);
bool PxcContactSphereMesh				(GU_CONTACT_METHOD_ARGS);
bool PxcContactSphereHeightField		(GU_CONTACT_METHOD_ARGS);

// Plane - other
bool PxcContactPlaneCapsule				(GU_CONTACT_METHOD_ARGS);
bool PxcContactPlaneBox					(GU_CONTACT_METHOD_ARGS);
bool PxcContactPlaneConvex				(GU_CONTACT_METHOD_ARGS);

// Capsule - other
bool PxcContactCapsuleCapsule			(GU_CONTACT_METHOD_ARGS);
bool PxcContactCapsuleBox				(GU_CONTACT_METHOD_ARGS);
bool PxcContactCapsuleConvex			(GU_CONTACT_METHOD_ARGS);
bool PxcContactCapsuleMesh				(GU_CONTACT_METHOD_ARGS);
bool PxcContactCapsuleHeightField		(GU_CONTACT_METHOD_ARGS);

// Box - other
bool PxcContactBoxBox					(GU_CONTACT_METHOD_ARGS);
bool PxcContactBoxConvex				(GU_CONTACT_METHOD_ARGS);
bool PxcContactBoxMesh					(GU_CONTACT_METHOD_ARGS);
bool PxcContactBoxHeightField			(GU_CONTACT_METHOD_ARGS);

// Convex - other
bool PxcContactConvexConvex				(GU_CONTACT_METHOD_ARGS);
bool PxcContactConvexMesh				(GU_CONTACT_METHOD_ARGS);
bool PxcContactConvexHeightField		(GU_CONTACT_METHOD_ARGS);


static bool PxcInvalidContactPair		(CONTACT_METHOD_ARGS_UNUSED)
{
	return false;
}

//PCM Sphere - other
bool PxcPCMContactSphereSphere			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactSpherePlane			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactSphereBox				(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactSphereCapsule			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactSphereConvex			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactSphereMesh			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactSphereHeightField		(GU_CONTACT_METHOD_ARGS);

// Plane - other
bool PxcPCMContactPlaneCapsule			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactPlaneBox				(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactPlaneConvex			(GU_CONTACT_METHOD_ARGS);

//PCM Capsule - other
bool PxcPCMContactCapsuleCapsule		(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactCapsuleBox			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactCapsuleConvex			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactCapsuleMesh			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactCapsuleHeightField	(GU_CONTACT_METHOD_ARGS);

//PCM Box - other
bool PxcPCMContactBoxBox				(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactBoxConvex				(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactBoxMesh				(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactBoxHeightField		(GU_CONTACT_METHOD_ARGS);

//PCM Convex
bool PxcPCMContactConvexConvex			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactConvexMesh			(GU_CONTACT_METHOD_ARGS);
bool PxcPCMContactConvexHeightField		(GU_CONTACT_METHOD_ARGS);


#define DYNAMIC_CONTACT_REGISTRATION(x) PxcInvalidContactPair
//#define DYNAMIC_CONTACT_REGISTRATION(x) x

//Table of contact methods for different shape-type combinations
PxcContactMethod g_ContactMethodTable[][PxGeometryType::eGEOMETRY_COUNT] = 
{
	//PxGeometryType::eSPHERE
	{
		PxcContactSphereSphere,			//PxGeometryType::eSPHERE
		PxcContactSpherePlane,			//PxGeometryType::ePLANE
		PxcContactSphereCapsule,		//PxGeometryType::eCAPSULE
		PxcContactSphereBox,			//PxGeometryType::eBOX
		PxcContactSphereConvex,			//PxGeometryType::eCONVEXMESH
		PxcContactSphereMesh,			//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_CONTACT_REGISTRATION(PxcContactSphereHeightField),	//PxGeometryType::eHEIGHTFIELD	//TODO: make HF midphase that will mask this
		
	},

	//PxGeometryType::ePLANE
	{
		0,								//PxGeometryType::eSPHERE
		PxcInvalidContactPair,			//PxGeometryType::ePLANE
		PxcContactPlaneCapsule,			//PxGeometryType::eCAPSULE
		PxcContactPlaneBox,				//PxGeometryType::eBOX
		PxcContactPlaneConvex,			//PxGeometryType::eCONVEXMESH
		PxcInvalidContactPair,			//PxGeometryType::eTRIANGLEMESH
		PxcInvalidContactPair,			//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eCAPSULE
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		PxcContactCapsuleCapsule,		//PxGeometryType::eCAPSULE
		PxcContactCapsuleBox,			//PxGeometryType::eBOX
		PxcContactCapsuleConvex,		//PxGeometryType::eCONVEXMESH
		PxcContactCapsuleMesh,			//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_CONTACT_REGISTRATION(PxcContactCapsuleHeightField),	//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eBOX
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		PxcContactBoxBox,				//PxGeometryType::eBOX
		PxcContactBoxConvex,			//PxGeometryType::eCONVEXMESH
		PxcContactBoxMesh,				//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_CONTACT_REGISTRATION(PxcContactBoxHeightField),		//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eCONVEXMESH
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		PxcContactConvexConvex,			//PxGeometryType::eCONVEXMESH
		PxcContactConvexMesh,			//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_CONTACT_REGISTRATION(PxcContactConvexHeightField),	//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eTRIANGLEMESH
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		0,								//PxGeometryType::eCONVEXMESH
		PxcInvalidContactPair,			//PxGeometryType::eTRIANGLEMESH
		PxcInvalidContactPair,			//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eHEIGHTFIELD
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		0,								//PxGeometryType::eCONVEXMESH
		0,								//PxGeometryType::eTRIANGLEMESH
		PxcInvalidContactPair,			//PxGeometryType::eHEIGHTFIELD
	},
};


//#if	PERSISTENT_CONTACT_MANIFOLD
//Table of contact methods for different shape-type combinations
PxcContactMethod g_PCMContactMethodTable[][PxGeometryType::eGEOMETRY_COUNT] = 
{
	//PxGeometryType::eSPHERE
	{
		PxcPCMContactSphereSphere,										//PxGeometryType::eSPHERE
		PxcPCMContactSpherePlane,										//PxGeometryType::ePLANE
		PxcPCMContactSphereCapsule,										//PxGeometryType::eCAPSULE
		PxcPCMContactSphereBox,											//PxGeometryType::eBOX
		PxcPCMContactSphereConvex,										//PxGeometryType::eCONVEXMESH
		PxcPCMContactSphereMesh,										//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_CONTACT_REGISTRATION(PxcPCMContactSphereHeightField),	//PxGeometryType::eHEIGHTFIELD	//TODO: make HF midphase that will mask this	
	},

	//PxGeometryType::ePLANE
	{
		0,															//PxGeometryType::eSPHERE
		PxcInvalidContactPair,										//PxGeometryType::ePLANE
		PxcPCMContactPlaneCapsule,									//PxGeometryType::eCAPSULE
		PxcPCMContactPlaneBox,										//PxGeometryType::eBOX  
		PxcPCMContactPlaneConvex,										//PxGeometryType::eCONVEXMESH
		PxcInvalidContactPair,										//PxGeometryType::eTRIANGLEMESH
		PxcInvalidContactPair,										//PxGeometryType::eHEIGHTFIELD
	},  

	//PxGeometryType::eCAPSULE
	{
		0,																//PxGeometryType::eSPHERE
		0,																//PxGeometryType::ePLANE
		PxcPCMContactCapsuleCapsule,									//PxGeometryType::eCAPSULE
		PxcPCMContactCapsuleBox,										//PxGeometryType::eBOX
		PxcPCMContactCapsuleConvex,										//PxGeometryType::eCONVEXMESH
		PxcPCMContactCapsuleMesh,										//PxGeometryType::eTRIANGLEMESH	
		DYNAMIC_CONTACT_REGISTRATION(PxcPCMContactCapsuleHeightField),	//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eBOX
	{
		0,																//PxGeometryType::eSPHERE
		0,																//PxGeometryType::ePLANE
		0,																//PxGeometryType::eCAPSULE
		PxcPCMContactBoxBox,											//PxGeometryType::eBOX
		PxcPCMContactBoxConvex,											//PxGeometryType::eCONVEXMESH
		PxcPCMContactBoxMesh,											//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_CONTACT_REGISTRATION(PxcPCMContactBoxHeightField),		//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this

	},

	//PxGeometryType::eCONVEXMESH
	{
		0,																	//PxGeometryType::eSPHERE
		0,																	//PxGeometryType::ePLANE
		0,																	//PxGeometryType::eCAPSULE
		0,																	//PxGeometryType::eBOX
		PxcPCMContactConvexConvex,											//PxGeometryType::eCONVEXMESH
		PxcPCMContactConvexMesh,											//PxGeometryType::eTRIANGLEMESH
		DYNAMIC_CONTACT_REGISTRATION(PxcPCMContactConvexHeightField),		//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eTRIANGLEMESH
	{
		0,																//PxGeometryType::eSPHERE
		0,																//PxGeometryType::ePLANE
		0,																//PxGeometryType::eCAPSULE
		0,																//PxGeometryType::eBOX
		0,																//PxGeometryType::eCONVEXMESH
		PxcInvalidContactPair,											//PxGeometryType::eTRIANGLEMESH
		PxcInvalidContactPair,											//PxGeometryType::eHEIGHTFIELD
	},   

	//PxGeometryType::eHEIGHTFIELD
	{
		0,																//PxGeometryType::eSPHERE
		0,																//PxGeometryType::ePLANE
		0,																//PxGeometryType::eCAPSULE
		0,																//PxGeometryType::eBOX
		0,																//PxGeometryType::eCONVEXMESH
		0,																//PxGeometryType::eTRIANGLEMESH
		PxcInvalidContactPair,											//PxGeometryType::eHEIGHTFIELD
	},

};
}
