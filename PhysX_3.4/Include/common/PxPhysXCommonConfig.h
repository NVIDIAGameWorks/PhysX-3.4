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


#ifndef PX_PHYSICS_COMMON_NX
#define PX_PHYSICS_COMMON_NX

/** \addtogroup common 
@{ */

#include "foundation/Px.h"

/*Disable support for VS2017 prior version 15.5.1 for windows platform, because of a compiler bug:
https://developercommunity.visualstudio.com/content/problem/66047/possible-compiler-bug.html
*/
#if (PX_VC == 15) && PX_WINDOWS && (_MSC_FULL_VER < 191225830)
#error Visual studio 2017 prior to 15.5.1 is not supported because of a compiler bug.
#endif

// define API function declaration (public API only needed because of extensions)
#if defined PX_PHYSX_STATIC_LIB || defined PX_PHYSX_CORE_STATIC_LIB
	#define PX_PHYSX_CORE_API
#else
	#if PX_WINDOWS
		#if defined PX_PHYSX_CORE_EXPORTS
			#define PX_PHYSX_CORE_API __declspec(dllexport)
		#else
			#define PX_PHYSX_CORE_API __declspec(dllimport)
		#endif
	#elif PX_UNIX_FAMILY
		#define PX_PHYSX_CORE_API PX_UNIX_EXPORT
    #else
		#define PX_PHYSX_CORE_API
    #endif
#endif

#if PX_SUPPORT_GPU_PHYSX

// define API function declaration
#if PX_WINDOWS 
#if defined PX_PHYSX_GPU_EXPORTS
#define PX_PHYSX_GPU_API __declspec(dllexport)
#else
#define PX_PHYSX_GPU_API __declspec(dllimport)
#endif
#elif PX_UNIX_FAMILY
#define PX_PHYSX_GPU_API PX_UNIX_EXPORT
#else
#define PX_PHYSX_GPU_API
#endif

#else // PX_SUPPORT_GPU_PHYSX
#define PX_PHYSX_GPU_API
#endif // PX_SUPPORT_GPU_PHYSX

#if PX_WINDOWS && !defined(__CUDACC__)
	#if defined PX_PHYSX_COMMON_EXPORTS
		#define PX_PHYSX_COMMON_API __declspec(dllexport)
	#else
		#define PX_PHYSX_COMMON_API __declspec(dllimport)
	#endif
#elif PX_UNIX_FAMILY
	#define PX_PHYSX_COMMON_API PX_UNIX_EXPORT
#else
	#define PX_PHYSX_COMMON_API
#endif

// Changing these parameters requires recompilation of the SDK

#if !PX_DOXYGEN
namespace physx
{
#endif
	class PxCollection;
	class PxBase;

	class PxHeightField;
	class PxHeightFieldDesc;

	class PxTriangleMesh;
	class PxConvexMesh;

	typedef PxU32 PxTriangleID;
	typedef PxU16 PxMaterialTableIndex;

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
