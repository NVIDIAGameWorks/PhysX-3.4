/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifdef WIN32
#include "ModuleUpdateLoader.h"

typedef HMODULE (GetUpdatedModule_FUNC)(const char*, const char*);

ModuleUpdateLoader::ModuleUpdateLoader(const char* updateLoaderDllName)
	: mGetUpdatedModuleFunc(NULL)
{
	mUpdateLoaderDllHandle = LoadLibrary(updateLoaderDllName);

	if (mUpdateLoaderDllHandle != NULL)
	{
		mGetUpdatedModuleFunc = GetProcAddress(mUpdateLoaderDllHandle, "GetUpdatedModule");
	}
}

ModuleUpdateLoader::~ModuleUpdateLoader()
{
	if (mUpdateLoaderDllHandle != NULL)
	{
		FreeLibrary(mUpdateLoaderDllHandle);
		mUpdateLoaderDllHandle = NULL;
	}
}

HMODULE ModuleUpdateLoader::loadModule(const char* moduleName, const char* appGuid)
{
	HMODULE result = NULL;

	if (mGetUpdatedModuleFunc != NULL)
	{
		// Try to get the module through PhysXUpdateLoader
		GetUpdatedModule_FUNC* getUpdatedModuleFunc = (GetUpdatedModule_FUNC*)mGetUpdatedModuleFunc;
		result = getUpdatedModuleFunc(moduleName, appGuid);
	}
	else
	{
		// If no PhysXUpdateLoader, just load the DLL directly
		result = LoadLibrary(moduleName);
	}

	return result;
}


#endif	// WIN32
