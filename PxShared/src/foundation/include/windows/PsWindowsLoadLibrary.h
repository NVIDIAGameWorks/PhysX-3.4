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


#ifndef PS_WINDOWS_FOUNDATION_LOADLIBRARY_H
#define PS_WINDOWS_FOUNDATION_LOADLIBRARY_H

#include "foundation/PxPreprocessor.h"
#include "windows/PsWindowsInclude.h"
#include "foundation/windows/PxWindowsFoundationDelayLoadHook.h"

namespace physx
{
namespace shdfnd
{
	EXTERN_C IMAGE_DOS_HEADER __ImageBase;

	PX_INLINE FARPROC WINAPI foundationDliNotePreLoadLibrary(const char* libraryName, const physx::PxFoundationDelayLoadHook* delayLoadHook)
	{	
		if(!delayLoadHook)
		{
			return (FARPROC)::LoadLibraryA(libraryName);
		}
		else
		{
			if(strstr(libraryName, "PxFoundation"))
			{
				if(strstr(libraryName, "DEBUG"))
					return (FARPROC)::LoadLibraryA(delayLoadHook->getPxFoundationDEBUGDllName());

				if(strstr(libraryName, "CHECKED"))
					return (FARPROC)::LoadLibraryA(delayLoadHook->getPxFoundationCHECKEDDllName());

				if(strstr(libraryName, "PROFILE"))
					return (FARPROC)::LoadLibraryA(delayLoadHook->getPxFoundationPROFILEDllName());

				return (FARPROC)::LoadLibraryA(delayLoadHook->getPxFoundationDllName());
			}
		}
		return NULL;
    }
} // namespace shdfnd
} // namespace physx


#endif	// PS_WINDOWS_FOUNDATION_LOADLIBRARY_H
