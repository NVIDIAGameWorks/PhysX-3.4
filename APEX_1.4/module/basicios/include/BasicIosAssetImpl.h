/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef BASIC_IOS_ASSET_IMPL_H
#define BASIC_IOS_ASSET_IMPL_H

#include "ApexUsingNamespace.h"
#include "Apex.h"
#include "IofxAsset.h"
#include "BasicIosAsset.h"
#include "InstancedObjectSimulationIntl.h"
#include "ApexSDKHelpers.h"
#include "ApexAssetAuthoring.h"
#include "ApexString.h"
#include "ResourceProviderIntl.h"
#include "ApexAuthorableObject.h"
#include "BasicIOSAssetParam.h"
#include "ApexAssetTracker.h"
#include "ApexRand.h"
#include "ApexRWLockable.h"

#include "ReadCheck.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{
namespace IOFX
{
class IofxAsset;
}
	
namespace basicios
{

class ModuleBasicIosImpl;
class BasicIosActorImpl;

/**
\brief Descriptor needed to create a BasicIOS Actor
*/
class BasicIosActorDesc : public ApexDesc
{
public:
	///Radius of a particle (overrides authered value)
	float				radius;
	///Density of a particle (overrides authered value)
	float				density;

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE BasicIosActorDesc() : ApexDesc()
	{
		init();
	}

	/**
	\brief sets members to default values.
	*/
	PX_INLINE void setToDefault()
	{
		ApexDesc::setToDefault();
		init();
	}

	/**
	\brief checks if this is a valid descriptor.
	*/
	PX_INLINE bool isValid() const
	{
		if (!ApexDesc::isValid())
		{
			return false;
		}

		return true;
	}

private:

	PX_INLINE void init()
	{
		// authored values will be used where these default values remain
		radius = 0.0f;
		density = 0.0f;
	}
};

class BasicIosAssetImpl : public BasicIosAsset,
	public ApexResourceInterface,
	public ApexResource,
	public ApexRWLockable
{
	friend class BasicIosAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	BasicIosAssetImpl(ModuleBasicIosImpl*, ResourceList&, const char*);
	BasicIosAssetImpl(ModuleBasicIosImpl* module, ResourceList&, NvParameterized::Interface*, const char*);
	~BasicIosAssetImpl();

	// Asset
	void							release();
	const char*						getName(void) const
	{
		return mName.c_str();
	}
	AuthObjTypeID					getObjTypeID() const
	{
		return mAssetTypeID;
	}
	const char* 					getObjTypeName() const
	{
		return getClassName();
	}
	// TODO: implement forceLoadAssets
	uint32_t					forceLoadAssets();

	Actor*					createIosActor(Scene& scene, IofxAsset* iofxAsset);
	void							releaseIosActor(Actor&);
	bool							getSupportsDensity() const;

	// Private API for this module only
	BasicIosActorImpl*                  getIosActorInScene(Scene& scene, bool mesh) const;

	// ApexResourceInterface methods
	void							setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	uint32_t					getListIndex() const
	{
		return m_listIndex;
	}

	float					getParticleRadius() const
	{
		READ_ZONE();
		return mParams->particleRadius;
	}
	float					getRestDensity() const
	{
		READ_ZONE();
		return mParams->restDensity;
	}
	float					getMaxInjectedParticleCount() const
	{
		READ_ZONE();
		return mParams->maxInjectedParticleCount;
	}
	uint32_t					getMaxParticleCount() const
	{
		READ_ZONE();
		return mParams->maxParticleCount;
	}
	float					getSceneGravityScale() const
	{
		return mParams->sceneGravityScale;
	}
	PxVec3&					getExternalAcceleration() const
	{
		return mParams->externalAcceleration;
	}
	float					getParticleMass() const;

	const NvParameterized::Interface* getAssetNvParameterized() const
	{
		return mParams;
	}
	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}

	NvParameterized::Interface* getDefaultActorDesc()
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	NvParameterized::Interface* getDefaultAssetPreviewDesc()
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual Actor* createApexActor(const NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/)
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual AssetPreview* createApexAssetPreview(const NvParameterized::Interface& /*params*/, AssetPreviewScene* /*previewScene*/)
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/) const
	{
		return true; // TODO implement this method
	}

	virtual bool isDirty() const
	{
		return false;
	}

protected:
	virtual void					destroy();

	static AuthObjTypeID			mAssetTypeID;
	static const char* 				getClassName()
	{
		return BASIC_IOS_AUTHORING_TYPE_NAME;
	}

	ResourceList					mIosActorList;

	ModuleBasicIosImpl*					mModule;
	ApexSimpleString				mName;

	BasicIOSAssetParam*				mParams;

	mutable QDSRand					mSRand;
	mutable QDNormRand				mNormRand;

	enum
	{
		UNIFORM,
		NORMAL
	}								mMassDistribType;

	void processParams();

	friend class ModuleBasicIosImpl;
	friend class BasicIosActorImpl;
	friend class BasicIosActorCPU;
	friend class BasicIosActorGPU;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;
	friend class BasicIosAuthorableObject;
};

#ifndef WITHOUT_APEX_AUTHORING
class BasicIosAssetAuthoringImpl : public BasicIosAssetAuthoring, public ApexAssetAuthoring, public BasicIosAssetImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	BasicIosAssetAuthoringImpl(ModuleBasicIosImpl* module, ResourceList& list);
	BasicIosAssetAuthoringImpl(ModuleBasicIosImpl* module, ResourceList& list, const char* name);
	BasicIosAssetAuthoringImpl(ModuleBasicIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name);

	virtual void	release();

	const char* 	getName(void) const
	{
		return BasicIosAssetImpl::getName();
	}
	const char* 	getObjTypeName() const
	{
		return BasicIosAssetImpl::getClassName();
	}
	virtual bool	prepareForPlatform(nvidia::apex::PlatformTag)
	{
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}
	void			setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	void			setParticleRadius(float radius)
	{
		mParams->particleRadius = radius;
	}
	void			setRestDensity(float density)
	{
		mParams->restDensity = density;
	}
	void			setMaxInjectedParticleCount(float count)
	{
		mParams->maxInjectedParticleCount = count;
	}
	void			setMaxParticleCount(uint32_t count)
	{
		mParams->maxParticleCount = count;
	}
	void			setParticleMass(float mass)
	{
		mParams->particleMass.center = mass;
	}

	void			setCollisionGroupName(const char* collisionGroupName);
	void			setCollisionGroupMaskName(const char* collisionGroupMaskName);

	NvParameterized::Interface* getNvParameterized() const
	{
		return (NvParameterized::Interface*)getAssetNvParameterized();
	}
	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}
};
#endif

}
} // namespace nvidia

#endif // BASIC_IOS_ASSET_IMPL_H
