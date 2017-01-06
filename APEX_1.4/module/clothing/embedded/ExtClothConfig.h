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


#ifndef EXT_CLOTH_CONFIG_NX
#define EXT_CLOTH_CONFIG_NX

/** \addtogroup common 
@{ */

#include "Px.h"

// Exposing of the Cloth API. Run API meta data generation in Tools/PhysXMetaDataGenerator when changing.
#define APEX_USE_CLOTH_API 1

// define API function declaration (public API only needed because of extensions)
#if defined EXT_CLOTH_STATIC_LIB
	#define EXT_CLOTH_CORE_API
#else
	#if PX_WINDOWS_FAMILY || PX_WINRT
		#if defined EXT_CLOTH_CORE_EXPORTS
			#define EXT_CLOTH_CORE_API PX_DLL_EXPORT
		#else
			#define EXT_CLOTH_CORE_API PX_DLL_IMPORT
		#endif
	#elif PX_UNIX_FAMILY
		#define EXT_CLOTH_CORE_API PX_UNIX_EXPORT
    #else
		#define EXT_CLOTH_CORE_API
    #endif
#endif

#if PX_WINDOWS_FAMILY || PX_WINRT && !defined(__CUDACC__)
	#if defined EXT_CLOTH_COMMON_EXPORTS
		#define EXT_CLOTH_COMMON_API __declspec(dllexport)
	#else
		#define EXT_CLOTH_COMMON_API __declspec(dllimport)
	#endif
#elif PX_UNIX_FAMILY
	#define EXT_CLOTH_COMMON_API PX_UNIX_EXPORT
#else
	#define EXT_CLOTH_COMMON_API
#endif

// Changing these parameters requires recompilation of the SDK

#ifndef PX_DOXYGEN
namespace physx
{
#endif
	class PxCollection;
	class PxBase;

	class PxHeightField;
	class PxHeightFieldDesc;

	class PxTriangleMesh;
	class PxConvexMesh;

	typedef uint32_t PxTriangleID;
	typedef uint16_t PxMaterialTableIndex;

#ifndef PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
