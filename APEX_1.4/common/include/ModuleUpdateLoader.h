/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULEUPDATELOADER_H
#define MODULEUPDATELOADER_H

#ifdef WIN32

#if PX_X64
#define UPDATE_LOADER_DLL_NAME "PhysXUpdateLoader64.dll"
#else
#define UPDATE_LOADER_DLL_NAME "PhysXUpdateLoader.dll"
#endif

// This GUID should change any time we release APEX with a public interface change.
#if PX_PHYSICS_VERSION_MAJOR == 0
#define DEFAULT_APP_GUID "165F143C-15CB-47FA-ACE3-2002B3684026"
#else
#define DEFAULT_APP_GUID "1AE7180B-79E5-4234-91A7-E387331B5993"
#endif

//#include "PsWindowsInclude.h"
#include <windows.h>

class ModuleUpdateLoader
{
public:
	ModuleUpdateLoader(const char* updateLoaderDllName);
	~ModuleUpdateLoader();

	// Loads the given module through the update loader. Loads it from the path if 
	// the update loader doesn't find the requested module. Returns NULL if no
	// module found.
	HMODULE loadModule(const char* moduleName, const char* appGuid);

protected:
	HMODULE mUpdateLoaderDllHandle;
	FARPROC mGetUpdatedModuleFunc;

	// unit test fixture
	friend class ModuleUpdateLoaderTest;
};

#endif	// WIN32
#endif	// MODULEUPDATELOADER_H
