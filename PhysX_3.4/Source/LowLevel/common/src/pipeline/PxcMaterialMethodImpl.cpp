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
#include "PxcMaterialMethodImpl.h"

namespace physx
{
bool PxcGetMaterialShapeShape			(MATERIAL_METHOD_ARGS);
bool PxcGetMaterialShapeMesh			(MATERIAL_METHOD_ARGS);
bool PxcGetMaterialShapeHeightField		(MATERIAL_METHOD_ARGS);
bool PxcGetMaterialShape				(SINGLE_MATERIAL_METHOD_ARGS);
bool PxcGetMaterialMesh					(SINGLE_MATERIAL_METHOD_ARGS);
bool PxcGetMaterialHeightField			(SINGLE_MATERIAL_METHOD_ARGS);


PxcGetSingleMaterialMethod g_GetSingleMaterialMethodTable[PxGeometryType::eGEOMETRY_COUNT] = 
{
	PxcGetMaterialShape,			//PxGeometryType::eSPHERE
	PxcGetMaterialShape,			//PxGeometryType::ePLANE
	PxcGetMaterialShape,			//PxGeometryType::eCAPSULE
	PxcGetMaterialShape,			//PxGeometryType::eBOX
	PxcGetMaterialShape,			//PxGeometryType::eCONVEXMESH
	PxcGetMaterialMesh,				//PxGeometryType::eTRIANGLEMESH	//not used: mesh always uses swept method for midphase.
	PxcGetMaterialHeightField,		//PxGeometryType::eHEIGHTFIELD	//TODO: make HF midphase that will mask this
		
};

//Table of contact methods for different shape-type combinations
PxcGetMaterialMethod g_GetMaterialMethodTable[][PxGeometryType::eGEOMETRY_COUNT] = 
{
	
	//PxGeometryType::eSPHERE
	{
		PxcGetMaterialShapeShape,			//PxGeometryType::eSPHERE
		PxcGetMaterialShapeShape,			//PxGeometryType::ePLANE
		PxcGetMaterialShapeShape,			//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,			//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,			//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,			//PxGeometryType::eTRIANGLEMESH	//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,		//PxGeometryType::eHEIGHTFIELD	//TODO: make HF midphase that will mask this
		
	},

	//PxGeometryType::ePLANE
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		PxcGetMaterialShapeShape,		//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,		//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,		//PxGeometryType::eCONVEXMESH
		0,								//PxGeometryType::eTRIANGLEMESH
		0,								//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eCAPSULE
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		PxcGetMaterialShapeShape,		//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,		//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,		//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,		//PxGeometryType::eTRIANGLEMESH		//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,		//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eBOX
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		PxcGetMaterialShapeShape,				//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,			//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,				//PxGeometryType::eTRIANGLEMESH		//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,		//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eCONVEXMESH
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		PxcGetMaterialShapeShape,			//PxGeometryType::eCONVEXMESH
		PxcGetMaterialShapeMesh,			//PxGeometryType::eTRIANGLEMESH		//not used: mesh always uses swept method for midphase.
		PxcGetMaterialShapeHeightField,	//PxGeometryType::eHEIGHTFIELD		//TODO: make HF midphase that will mask this
	},

	//PxGeometryType::eTRIANGLEMESH
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		0,								//PxGeometryType::eCONVEXMESH
		0,								//PxGeometryType::eTRIANGLEMESH
		0,								//PxGeometryType::eHEIGHTFIELD
	},

	//PxGeometryType::eHEIGHTFIELD
	{
		0,								//PxGeometryType::eSPHERE
		0,								//PxGeometryType::ePLANE
		0,								//PxGeometryType::eCAPSULE
		0,								//PxGeometryType::eBOX
		0,								//PxGeometryType::eCONVEXMESH
		0,								//PxGeometryType::eTRIANGLEMESH
		0,								//PxGeometryType::eHEIGHTFIELD
	},
		
};

}
