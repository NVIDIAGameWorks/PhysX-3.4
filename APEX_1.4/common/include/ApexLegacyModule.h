/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_LEGACY_MODULE
#define APEX_LEGACY_MODULE

#include "nvparameterized/NvParameterizedTraits.h"

#include "ModuleIntl.h"
#include "ModuleBase.h"

namespace nvidia
{
namespace apex
{

struct LegacyClassEntry
{
	uint32_t version;
	uint32_t nextVersion;
	NvParameterized::Factory* factory;
	void (*freeParameterDefinitionTable)(NvParameterized::Traits* t);
	NvParameterized::Conversion* (*createConv)(NvParameterized::Traits*);
	NvParameterized::Conversion* conv;
};

template<typename IFaceT>
class TApexLegacyModule : public IFaceT, public ModuleIntl, public ModuleBase
{
public:
	virtual ~TApexLegacyModule() {}

	// base class methods
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
		releaseLegacyObjects();
		ModuleBase::destroy();
		delete this;
	}

	const char*					getName() const
	{
		return ModuleBase::getName();
	}

	ModuleSceneIntl* 				createInternalModuleScene(SceneIntl&, RenderDebugInterface*)
	{
		return NULL;
	}
	void						releaseModuleSceneIntl(ModuleSceneIntl&) {}
	uint32_t				forceLoadAssets()
	{
		return 0;
	}
	AuthObjTypeID				getModuleID() const
	{
		return UINT32_MAX;
	}
	RenderableIterator* 	createRenderableIterator(const Scene&)
	{
		return NULL;
	}

protected:
	virtual void releaseLegacyObjects() = 0;

	void registerLegacyObjects(LegacyClassEntry* e)
	{
		NvParameterized::Traits* t = mSdk->getParameterizedTraits();
		if (!t)
		{
			return;
		}

		for (; e->factory; ++e)
		{
			t->registerFactory(*e->factory);

			e->conv = e->createConv(t);
			t->registerConversion(e->factory->getClassName(), e->version, e->nextVersion, *e->conv);
		}
	}

	void unregisterLegacyObjects(LegacyClassEntry* e)
	{
		NvParameterized::Traits* t = mSdk->getParameterizedTraits();
		if (!t)
		{
			return;
		}

		for (; e->factory; ++e)
		{
			t->removeConversion(
			    e->factory->getClassName(),
			    e->version,
			    e->nextVersion
			);
			e->conv->release();

			t->removeFactory(e->factory->getClassName(), e->factory->getVersion());

			e->freeParameterDefinitionTable(t);
		}
	}
};

typedef TApexLegacyModule<Module> ApexLegacyModule;

} // namespace apex
} // namespace nvidia

#define DEFINE_CREATE_MODULE(ModuleBase) \
	ApexSDKIntl* gApexSdk = 0; \
	ApexSDK* GetApexSDK() { return gApexSdk; } \
	ApexSDKIntl* GetInternalApexSDK() { return gApexSdk; } \
	APEX_API Module*  CALL_CONV createModule( \
	        ApexSDKIntl* inSdk, \
	        ModuleIntl** niRef, \
	        uint32_t APEXsdkVersion, \
	        uint32_t PhysXsdkVersion, \
	        ApexCreateError* errorCode) \
	{ \
		if (APEXsdkVersion != APEX_SDK_VERSION) \
		{ \
			if (errorCode) *errorCode = APEX_CE_WRONG_VERSION; \
			return NULL; \
		} \
		\
		if (PhysXsdkVersion != PHYSICS_SDK_VERSION) \
		{ \
			if (errorCode) *errorCode = APEX_CE_WRONG_VERSION; \
			return NULL; \
		} \
		\
		gApexSdk = inSdk; \
		\
		ModuleBase *impl = PX_NEW(ModuleBase)(inSdk); \
		*niRef  = (ModuleIntl *) impl; \
		return (Module *) impl; \
	}

#define DEFINE_INSTANTIATE_MODULE(ModuleBase) \
	void instantiate##ModuleBase() \
	{ \
		ApexSDKIntl *sdk = GetInternalApexSDK(); \
		ModuleBase *impl = PX_NEW(ModuleBase)(sdk); \
		sdk->registerExternalModule((Module *) impl, (ModuleIntl *) impl); \
	}

#endif // __APEX_LEGACY_MODULE__
