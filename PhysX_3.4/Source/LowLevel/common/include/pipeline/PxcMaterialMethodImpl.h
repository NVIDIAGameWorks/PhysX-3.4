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

#ifndef PXC_MATERIALMETHOD_H
#define PXC_MATERIALMETHOD_H

#include "CmPhysXCommon.h"
#include "PxGeometry.h"

namespace physx
{

struct PxsShapeCore;
struct PxsMaterialInfo;
class PxcNpThreadContext;

#define MATERIAL_METHOD_ARGS			\
	const PxsShapeCore* shape0,			\
	const PxsShapeCore* shape1,			\
	PxcNpThreadContext& pairContext,	\
	PxsMaterialInfo* materialInfo


#define SINGLE_MATERIAL_METHOD_ARGS			\
	const PxsShapeCore* shape,				\
	const PxU32	index,						\
	PxcNpThreadContext& pairContext,		\
	PxsMaterialInfo* materialInfo			

/*!
Method prototype for fetch material routines
*/
typedef bool (*PxcGetMaterialMethod) (MATERIAL_METHOD_ARGS);

typedef bool (*PxcGetSingleMaterialMethod) (SINGLE_MATERIAL_METHOD_ARGS);

extern PxcGetMaterialMethod g_GetMaterialMethodTable[][PxGeometryType::eGEOMETRY_COUNT];

extern PxcGetSingleMaterialMethod g_GetSingleMaterialMethodTable[PxGeometryType::eGEOMETRY_COUNT];

}

#endif
