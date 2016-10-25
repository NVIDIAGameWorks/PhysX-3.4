/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PARTICLE_IOS_ASSET_IMPL_H
#define PARTICLE_IOS_ASSET_IMPL_H

#include "Apex.h"
#include "iofx/IofxAsset.h"
#include "ParticleIosAsset.h"
#include "InstancedObjectSimulationIntl.h"
#include "ApexSDKHelpers.h"
#include "ApexAssetAuthoring.h"
#include "ApexString.h"
#include "ResourceProviderIntl.h"
#include "ApexAuthorableObject.h"
#include "ParticleIosAssetParam.h"
#include "ApexAssetTracker.h"
#include "ApexUsingNamespace.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{

namespace iofx
{
class IofxAsset;
}

namespace pxparticleios
{

class ModuleParticleIosImpl;
class ParticleIosActorImpl;


/**
\brief Descriptor needed to create a ParticleIOS Actor
*/
class ParticleIosActorDesc : public ApexDesc
{
public:
	///Radius of a particle (overrides authered value)
	float				radius;
	///Density of a particle (overrides authered value)
	float				density;

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ParticleIosActorDesc() : ApexDesc()
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

class ParticleIosAssetImpl : public ParticleIosAsset,
	public ApexResourceInterface,
	public ApexResource,
	public ApexRWLockable
{
	friend class ParticleIosAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ParticleIosAssetImpl(ModuleParticleIosImpl*, ResourceList&, const char*);
	ParticleIosAssetImpl(ModuleParticleIosImpl* module, ResourceList&, NvParameterized::Interface*, const char*);
	~ParticleIosAssetImpl();

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
	uint32_t					forceLoadAssets();

	Actor*					createIosActor(Scene& scene, IofxAsset* iofxAsset);
	void							releaseIosActor(Actor&);
	bool							getSupportsDensity() const
	{
		READ_ZONE();
		return mParams->DensityBuffer;
	}
	bool							isValidForActorCreation(const ::NvParameterized::Interface& /*actorParams*/, Scene& /*apexScene*/) const
	{
		return true;
	}

	bool							isDirty() const
	{
		return false;
	}


	// Private API for this module only
	ParticleIosActorImpl*               getIosActorInScene(Scene& scene, bool mesh) const;

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
	//float					getRestDensity() const				{ return mParams->restDensity; }
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
	const char*							getParticleTypeClassName() const
	{
		return mParams->particleType->className();
	}
	const ParticleIosAssetParam* getParticleDesc() const
	{
		return mParams;
	}
	float					getParticleMass() const
	{
		READ_ZONE();
		return mParams->particleMass;
	}

	const NvParameterized::Interface* getAssetNvParameterized() const
	{
		READ_ZONE();
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
		READ_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	NvParameterized::Interface* getDefaultAssetPreviewDesc()
	{
		READ_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual Actor* createApexActor(const NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/)
	{
		WRITE_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual AssetPreview* createApexAssetPreview(const NvParameterized::Interface& /*params*/, AssetPreviewScene* /*previewScene*/)
	{
		WRITE_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

protected:
	virtual void					destroy();

	static AuthObjTypeID			mAssetTypeID;
	static const char* 				getClassName()
	{
		return PARTICLE_IOS_AUTHORING_TYPE_NAME;
	}

	ResourceList					mIosActorList;

	ModuleParticleIosImpl*				mModule;
	ApexSimpleString				mName;

	ParticleIosAssetParam*			mParams;

	friend class ModuleParticleIosImpl;
	friend class ParticleIosActorImpl;
	friend class ParticleIosActorCPU;
	friend class ParticleIosActorGPU;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;
	friend class ParticleIosAuthorableObject;
};

#ifndef WITHOUT_APEX_AUTHORING
class ParticleIosAssetAuthoringImpl : public ParticleIosAssetAuthoring, public ApexAssetAuthoring, public ParticleIosAssetImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ParticleIosAssetAuthoringImpl(ModuleParticleIosImpl* module, ResourceList& list);
	ParticleIosAssetAuthoringImpl(ModuleParticleIosImpl* module, ResourceList& list, const char* name);
	ParticleIosAssetAuthoringImpl(ModuleParticleIosImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name);

	virtual void	release();

	const char* 	getName(void) const
	{
		READ_ZONE();
		return mName.c_str();
	}
	const char* 	getObjTypeName() const
	{
		READ_ZONE();
		return ParticleIosAssetImpl::getClassName();
	}
	virtual bool	prepareForPlatform(nvidia::apex::PlatformTag)
	{
		WRITE_ZONE();
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}
	void			setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		WRITE_ZONE();
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	void			setParticleRadius(float radius)
	{
		mParams->particleRadius = radius;
	}
	//void			setRestDensity( float density )			{ mParams->restDensity = density; }
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
		mParams->particleMass = mass;
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

#endif // PARTICLE_IOS_ASSET_IMPL_H
