/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_BASIC_FS_IMPL_H__
#define __MODULE_BASIC_FS_IMPL_H__

#include "Apex.h"
#include "ModuleBasicFS.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ModuleBase.h"

#include "ApexSDKHelpers.h"
#include "ApexRWLockable.h"
#include "ModuleBasicFSRegistration.h"


namespace nvidia
{
namespace apex
{
class SceneIntl;
class ModuleFieldSamplerIntl;
}	
namespace basicfs
{

class BasicFSAssetImpl;
class JetFSAssetAuthoring;
class AttractorFSAssetAuthoring;
class VortexFSAssetAuthoring;
class BasicFSScene;

class ModuleBasicFSImpl : public ModuleBasicFS, public ModuleIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleBasicFSImpl(ApexSDKIntl* sdk);
	~ModuleBasicFSImpl();

	// base class methods
	void						init(NvParameterized::Interface&);
	NvParameterized::Interface* getDefaultModuleDesc();
	void release()
	{
		ModuleBase::release();
	}
	void destroy();
	const char* getName() const
	{
		return ModuleBase::getName();
	}

	ModuleSceneIntl* 				createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void						releaseModuleSceneIntl(ModuleSceneIntl&);
	AuthObjTypeID				getModuleID() const;
	RenderableIterator* 	createRenderableIterator(const Scene&);

	AuthObjTypeID             getJetFSAssetTypeID() const;
	AuthObjTypeID             getAttractorFSAssetTypeID() const;
	AuthObjTypeID             getVortexFSAssetTypeID() const;
	AuthObjTypeID             getNoiseFSAssetTypeID() const;
	AuthObjTypeID             getWindFSAssetTypeID() const;

	ApexActor* 					getApexActor(Actor*, AuthObjTypeID) const;

	ModuleFieldSamplerIntl* 		getInternalModuleFieldSampler();

	BasicFSScene* 				getBasicFSScene(const Scene& apexScene); // return to protected
protected:
	ResourceList				mAuthorableObjects;

	ResourceList				mBasicFSScenes;

	friend class BasicFSAssetImpl;
	friend class JetFSAsset;
	friend class AttractorFSAsset;
	friend class VortexFSAsset;
	friend class BasicFSScene;

private:

	BasicFSModuleParameters* 			mModuleParams;

	ModuleFieldSamplerIntl* 				mFieldSamplerModule;
};

}
} // end namespace nvidia::apex

#endif // __MODULE_BASIC_FS_IMPL_H__
