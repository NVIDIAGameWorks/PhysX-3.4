/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULELEGACY_H_
#define MODULELEGACY_H_

#include "Apex.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ModuleBase.h"
#include "ModuleLoader.h"
#include "PsArray.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{

class ModuleLoaderImpl : public ModuleLoader, public ModuleIntl, public ModuleBase, public ApexRWLockable
{
	physx::Array<Module*> mModules; // Loaded modules
	ModuleLoader::ModuleNameErrorMap mCreateModuleError; // Modules error during it creating

	uint32_t getIdx(const char* name) const;

	uint32_t getIdx(Module* module) const;

public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleLoaderImpl(ApexSDKIntl* inSdk)
	{
		mName = "Loader";
		mSdk = inSdk;
		mApiProxy = this;
	}

	Module* loadModule(const char* name);

	void loadModules(const char** names, uint32_t size, Module** modules);

	void loadAllModules();

	const ModuleNameErrorMap& getLoadedModulesErrors() const;

	void loadAllLegacyModules();

	uint32_t getLoadedModuleCount() const;

	Module* getLoadedModule(uint32_t idx) const;

	Module* getLoadedModule(const char* name) const;

	void releaseModule(uint32_t idx);

	void releaseModule(const char* name);

	void releaseModule(Module* module);

	void releaseModules(const char** modules, uint32_t size);

	void releaseModules(Module** modules, uint32_t size);

	void releaseLoadedModules();

protected:

	virtual ~ModuleLoaderImpl() {}

	// ModuleBase's stuff

	void						init(NvParameterized::Interface&) {}

	NvParameterized::Interface* getDefaultModuleDesc()
	{
		return 0;
	}

	void release()
	{
		ModuleBase::release();
	}
	void destroy()
	{
		ModuleBase::destroy();
		delete this;
	}

	const char*					getName() const
	{
		return ModuleBase::getName();
	}

	AuthObjTypeID				getModuleID() const
	{
		return UINT32_MAX;
	}
	RenderableIterator* 	createRenderableIterator(const Scene&)
	{
		return NULL;
	}

	// ModuleIntl's stuff

	ModuleSceneIntl* 				createInternalModuleScene(SceneIntl&, RenderDebugInterface*)
	{
		return NULL;
	}
	void						releaseModuleSceneIntl(ModuleSceneIntl&) {}
	uint32_t				forceLoadAssets()
	{
		return 0;
	}
};

}
} // physx::apex

#endif
