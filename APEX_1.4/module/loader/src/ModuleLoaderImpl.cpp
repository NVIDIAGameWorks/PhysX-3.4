/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexUsingNamespace.h"
#include <PsHashMap.h>
#include <PsHash.h>

#include "ModuleLoaderImpl.h"
#include "Modules.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace apex
{

#if !defined(_USRDLL) && !PX_OSX
#	define INSTANTIATE_MODULE(instFunc) do { instFunc(); } while(0)
#else
#	define INSTANTIATE_MODULE(instFunc)
#endif

#if defined(_USRDLL) || PX_OSX

ApexSDKIntl* gApexSdk = 0;
ApexSDK* GetApexSDK()
{
	return gApexSdk;
}
ApexSDKIntl* GetInternalApexSDK()
{
	return gApexSdk;
}

APEX_API Module*  CALL_CONV createModule(
    ApexSDKIntl* inSdk,
    ModuleIntl** niRef,
    uint32_t APEXsdkVersion,
    uint32_t PhysXsdkVersion,
    ApexCreateError* errorCode)
{
	if (APEXsdkVersion != APEX_SDK_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	if (PhysXsdkVersion != PX_PHYSICS_VERSION)
	{
		if (errorCode)
		{
			*errorCode = APEX_CE_WRONG_VERSION;
		}
		return NULL;
	}

	gApexSdk = inSdk;

	ModuleLoaderImpl* impl = PX_NEW(ModuleLoaderImpl)(inSdk);
	*niRef  = (ModuleIntl*) impl;
	return (Module*) impl;
}

#else

void instantiateModuleLoader()
{
	ApexSDKIntl* sdk = GetInternalApexSDK();
	ModuleLoaderImpl* impl = PX_NEW(ModuleLoaderImpl)(sdk);
	sdk->registerExternalModule((Module*) impl, (ModuleIntl*) impl);
}

#endif

void ModuleLoaderImpl::loadModules(const char** names, uint32_t size, Module** modules)
{
	WRITE_ZONE();
	typedef HashMap<const char*, uint32_t, nvidia::Hash<const char*> > MapName2Idx;
	MapName2Idx mapName2Idx;

	if (modules)
	{
		for (uint32_t i = 0; i < size; ++i)
		{
			mapName2Idx[names[i]] = i;
		}
	}

	// TODO: check that module is not yet loaded
#	define MODULE(name, instFunc, hasInit) { \
		if (mapName2Idx.find(name)) { \
			INSTANTIATE_MODULE(instFunc); \
			mCreateModuleError[name] = APEX_CE_NO_ERROR; \
			Module *module = mSdk->createModule(name, &mCreateModuleError[name]); \
			PX_ASSERT(module && name); \
			if (module) \
			{ \
				mModules.pushBack(module); \
				if( modules ) modules[mapName2Idx[name]] = module; \
				if( hasInit && module ) { \
					NvParameterized::Interface* params = module->getDefaultModuleDesc(); \
					module->init(*params); \
				} \
			} \
		} \
	}
#	include "ModuleXmacro.h"
}

// TODO: check that module is not yet loaded in all ModuleLoaderImpl::load*-methods

Module* ModuleLoaderImpl::loadModule(const char* name_)
{
	WRITE_ZONE();
#	define MODULE(name, instFunc, hasInit) { \
		if( 0 == ::strcmp(name, name_) ) { \
			INSTANTIATE_MODULE(instFunc); \
			mCreateModuleError[name] = APEX_CE_NO_ERROR; \
			Module *module = mSdk->createModule(name, &mCreateModuleError[name]); \
			PX_ASSERT(module && name); \
			if (module) \
			{ \
				mModules.pushBack(module); \
				if( hasInit && module ) { \
					NvParameterized::Interface* params = module->getDefaultModuleDesc(); \
					module->init(*params); \
				} \
			} \
			return module; \
		} \
	}
#	include "ModuleXmacro.h"

	return 0;
}

void ModuleLoaderImpl::loadAllModules()
{
	WRITE_ZONE();
	// TODO: check that module is not yet loaded
#	define MODULE(name, instFunc, hasInit) { \
		INSTANTIATE_MODULE(instFunc); \
		mCreateModuleError[name] = APEX_CE_NO_ERROR; \
		Module *module = mSdk->createModule(name, &mCreateModuleError[name]); \
		PX_ASSERT(module && name); \
		if (module) \
		{ \
			mModules.pushBack(module); \
			if( hasInit && module ) { \
				NvParameterized::Interface* params = module->getDefaultModuleDesc(); \
				module->init(*params); \
			} \
		} \
	}
#	include "ModuleXmacro.h"
}

const ModuleLoader::ModuleNameErrorMap& ModuleLoaderImpl::getLoadedModulesErrors() const
{
	return mCreateModuleError;
}

void ModuleLoaderImpl::loadAllLegacyModules()
{
	WRITE_ZONE();
	// TODO: check that module is not yet loaded
#	define MODULE(name, instFunc, hasInit)
#	define MODULE_LEGACY(name, instFunc, hasInit) { \
		INSTANTIATE_MODULE(instFunc); \
		mCreateModuleError[name] = APEX_CE_NO_ERROR; \
		Module *module = mSdk->createModule(name, &mCreateModuleError[name]); \
		PX_ASSERT(module && name); \
		if (module) \
		{ \
			mModules.pushBack(module); \
		} \
	}
#	include "ModuleXmacro.h"
}

uint32_t ModuleLoaderImpl::getIdx(const char* name) const
{
	for (uint32_t i = 0; i < mModules.size(); ++i)
		if (0 == ::strcmp(name, mModules[i]->getName()))
		{
			return i;
		}

	return UINT32_MAX;
}

uint32_t ModuleLoaderImpl::getIdx(Module* module) const
{
	for (uint32_t i = 0; i < mModules.size(); ++i)
		if (module == mModules[i])
		{
			return i;
		}

	return UINT32_MAX;
}

uint32_t ModuleLoaderImpl::getLoadedModuleCount() const
{
	READ_ZONE();
	return mModules.size();
}

Module* ModuleLoaderImpl::getLoadedModule(uint32_t idx) const
{
	READ_ZONE();
	return mModules[idx];
}

Module* ModuleLoaderImpl::getLoadedModule(const char* name) const
{
	READ_ZONE();
	uint32_t idx = getIdx(name);
	return  UINT32_MAX == idx ? 0 : mModules[idx];
}

void ModuleLoaderImpl::releaseModule(uint32_t idx)
{
	WRITE_ZONE();
	if (UINT32_MAX != idx)
	{
		mCreateModuleError.erase(mModules[idx]->getName());
		mSdk->releaseModule(mModules[idx]);
		mModules.remove(idx);
	}
}

void ModuleLoaderImpl::releaseModule(const char* name)
{
	WRITE_ZONE();
	releaseModule(getIdx(name));
}

void ModuleLoaderImpl::releaseModule(Module* module)
{
	WRITE_ZONE();
	releaseModule(getIdx(module));
}

void ModuleLoaderImpl::releaseModules(Module** modules, uint32_t size)
{
	for (uint32_t i = 0; i < size; ++i)
	{
		releaseModule(modules[i]);
	}
}

void ModuleLoaderImpl::releaseModules(const char** names, uint32_t size)
{
	WRITE_ZONE();
	for (uint32_t i = 0; i < size; ++i)
	{
		releaseModule(names[i]);
	}
}

void ModuleLoaderImpl::releaseLoadedModules()
{
	WRITE_ZONE();
	while (!mModules.empty())
	{
		releaseModule(uint32_t(0));
	}
}

}
} // physx::apex
