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

#ifndef PX_FOUNDATION_DELAY_LOAD_HOOK
#define PX_FOUNDATION_DELAY_LOAD_HOOK

#include "foundation/PxPreprocessor.h"

/** \addtogroup foundation
@{
*/

#if !PX_DOXYGEN
namespace physx
{
#endif
	/**
 	\brief PxFoundationDelayLoadHook

	This is a helper class for delay loading the PxFoundation dll. 
	If a PxFoundation dll with a non-default file name needs to be loaded, 
	PxFoundationDelayLoadHook can be sub-classed to provide the custom file names.

	Once the names are set, the instance must be set for use by the loading dll. 
 	*/
	class PxFoundationDelayLoadHook
	{
	public:
		PxFoundationDelayLoadHook() {}
		virtual ~PxFoundationDelayLoadHook() {}

		virtual const char* getPxFoundationDEBUGDllName() const = 0;
		virtual const char* getPxFoundationCHECKEDDllName() const = 0;
		virtual const char* getPxFoundationPROFILEDllName() const = 0;
		virtual const char* getPxFoundationDllName() const = 0;

	protected:
	private:
	};

#if !PX_DOXYGEN
} // namespace physx
#endif
/** @} */
#endif
