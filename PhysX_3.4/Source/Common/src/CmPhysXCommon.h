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


#ifndef PX_PHYSICS_COMMON
#define PX_PHYSICS_COMMON

//! \file Top level internal include file for PhysX SDK

#include "Ps.h"

// Enable debug visualization
#define PX_ENABLE_DEBUG_VISUALIZATION	1

// Enable simulation statistics generation
#define PX_ENABLE_SIM_STATS 1

// PT: typical "invalid" value in various CD algorithms
#define	PX_INVALID_U32		0xffffffff
#define PX_INVALID_U16		0xffff

// PT: this used to be replicated everywhere in the code, causing bugs to sometimes reappear (e.g. TTP 3587).
// It is better to define it in a header and use the same constant everywhere. The original value (1e-05f)
// caused troubles (e.g. TTP 1705, TTP 306).
#define PX_PARALLEL_TOLERANCE	1e-02f

#define PX_USE_16_BIT_HANDLES 0

namespace physx
{
	// alias shared foundation to something usable
	namespace Ps = shdfnd;
}

#if PX_CHECKED
	#define PX_CHECK_MSG(exp, msg)				(!!(exp) || (physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, msg), 0) )
	#define PX_CHECK(exp)						PX_CHECK_MSG(exp, #exp)
	#define PX_CHECK_AND_RETURN(exp,msg)		{ if(!(exp)) { PX_CHECK_MSG(exp, msg); return; } }
	#define PX_CHECK_AND_RETURN_NULL(exp,msg)	{ if(!(exp)) { PX_CHECK_MSG(exp, msg); return 0; } }
	#define PX_CHECK_AND_RETURN_VAL(exp,msg,r)	{ if(!(exp)) { PX_CHECK_MSG(exp, msg); return r; } }
#else
	#define PX_CHECK_MSG(exp, msg)
	#define PX_CHECK(exp)
	#define PX_CHECK_AND_RETURN(exp,msg)
	#define PX_CHECK_AND_RETURN_NULL(exp,msg)
	#define PX_CHECK_AND_RETURN_VAL(exp,msg,r)
#endif

#if PX_VC
	// VC compiler defines __FUNCTION__ as a string literal so it is possible to concatenate it with another string
	// Example: #define PX_CHECK_VALID(x)	PX_CHECK_MSG(physx::shdfnd::checkValid(x), __FUNCTION__ ": parameter invalid!")
	#define PX_CHECK_VALID(x)				PX_CHECK_MSG(physx::shdfnd::checkValid(x), __FUNCTION__)
#elif PX_GCC_FAMILY
	// GCC compiler defines __FUNCTION__ as a variable, hence, it is NOT possible concatenate an additional string to it
	// In GCC, __FUNCTION__ only returns the function name, using __PRETTY_FUNCTION__ will return the full function definition
	#define PX_CHECK_VALID(x)				PX_CHECK_MSG(physx::shdfnd::checkValid(x), __PRETTY_FUNCTION__)
#else
	// Generic macro for other compilers
	#define PX_CHECK_VALID(x)				PX_CHECK_MSG(physx::shdfnd::checkValid(x), __FUNCTION__)
#endif


#endif
