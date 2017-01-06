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


#ifndef PXC_CONTACTMETHODIMPL_H
#define PXC_CONTACTMETHODIMPL_H

#include "GuGeometryUnion.h"
#include "CmPhysXCommon.h"
#include "GuContactMethodImpl.h"

namespace physx
{
namespace Cm
{
	class RenderOutput;
}

namespace Gu
{
	class ContactBuffer;
	struct Cache;
	struct NarrowPhaseParams;
}

struct PxcNpCache;
class PxcNpThreadContext;
class PxsContext;
class PxcConvexTriangles;
class PxsRigidBody;
struct PxsCCDShape;

namespace Cm
{
	class FastVertex2ShapeScaling;
}

/*!\file
This file contains forward declarations of all implemented contact methods.
*/


/*! Parameter list without names to avoid unused parameter warnings 
*/
#define CONTACT_METHOD_ARGS_UNUSED			\
	const Gu::GeometryUnion&,				\
	const Gu::GeometryUnion&,				\
	const PxTransform&,						\
	const PxTransform&,						\
	const Gu::NarrowPhaseParams&,			\
	Gu::Cache&,								\
	Gu::ContactBuffer&,						\
	Cm::RenderOutput*


/*!
Method prototype for contact generation routines
*/
typedef bool (*PxcContactMethod) (GU_CONTACT_METHOD_ARGS);


// Matrix of types
extern PxcContactMethod g_ContactMethodTable[][PxGeometryType::eGEOMETRY_COUNT];
extern const bool g_CanUseContactCache[][PxGeometryType::eGEOMETRY_COUNT];
extern PxcContactMethod g_PCMContactMethodTable[][PxGeometryType::eGEOMETRY_COUNT];

extern bool gEnablePCMCaching[][PxGeometryType::eGEOMETRY_COUNT];

extern bool gUnifiedHeightfieldCollision;
}

#endif
