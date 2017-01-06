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


#ifndef CM_WINDOWS_LOADLIBRARY_H
#define CM_WINDOWS_LOADLIBRARY_H

#include "foundation/PxPreprocessor.h"
#include "windows/PxWindowsDelayLoadHook.h"
#include "windows/PsWindowsInclude.h"
#include "windows/PsWindowsLoadLibrary.h"

#ifdef PX_SECURE_LOAD_LIBRARY
#include "nvSecureLoadLibrary.h"
#endif


namespace physx
{
namespace Cm
{
	EXTERN_C IMAGE_DOS_HEADER __ImageBase;

	PX_INLINE HMODULE WINAPI loadLibrary(const char* name)
	{
#ifdef PX_SECURE_LOAD_LIBRARY
		HMODULE retVal = nvLoadSignedLibrary(name,true);
		if(!retVal)
		{
			exit(1);
		}
		return retVal;
#else
		return ::LoadLibraryA( name );
#endif		
	};

	PX_INLINE FARPROC WINAPI physXCommonDliNotePreLoadLibrary(const char* libraryName, const physx::PxDelayLoadHook* delayLoadHook)
	{	
		if(!delayLoadHook)
		{
			return (FARPROC)loadLibrary(libraryName);
		}
		else
		{
			if(strstr(libraryName, "PxFoundation"))
			{
				return (FARPROC)physx::shdfnd::foundationDliNotePreLoadLibrary(libraryName, delayLoadHook);
			}

			if(strstr(libraryName, "PhysX3Common"))
			{
				if(strstr(libraryName, "DEBUG"))
					return (FARPROC)loadLibrary(delayLoadHook->getPhysXCommonDEBUGDllName());

				if(strstr(libraryName, "CHECKED"))
					return (FARPROC)Cm::loadLibrary(delayLoadHook->getPhysXCommonCHECKEDDllName());

				if(strstr(libraryName, "PROFILE"))
					return (FARPROC)Cm::loadLibrary(delayLoadHook->getPhysXCommonPROFILEDllName());

				return (FARPROC)Cm::loadLibrary(delayLoadHook->getPhysXCommonDllName());
			}

			if(strstr(libraryName, "PxPvdSDK"))
			{
				if(strstr(libraryName, "DEBUG"))
					return (FARPROC)Cm::loadLibrary(delayLoadHook->getPxPvdSDKDEBUGDllName());

				if(strstr(libraryName, "CHECKED"))
					return (FARPROC)Cm::loadLibrary(delayLoadHook->getPxPvdSDKCHECKEDDllName());

				if(strstr(libraryName, "PROFILE"))
					return (FARPROC)Cm::loadLibrary(delayLoadHook->getPxPvdSDKPROFILEDllName());

				return (FARPROC)Cm::loadLibrary(delayLoadHook->getPxPvdSDKDllName());
			}
		}
		return NULL;
    }
} // namespace Cm
} // namespace physx


#endif	// CM_WINDOWS_LOADLIBRARY_H
