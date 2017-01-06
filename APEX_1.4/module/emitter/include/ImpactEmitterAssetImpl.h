/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IMPACT_EMITTER_ASSET_IMPL_H
#define IMPACT_EMITTER_ASSET_IMPL_H

#include "ImpactEmitterAsset.h"
#include "ApexSDKHelpers.h"
#include "ApexAssetAuthoring.h"
#include "ApexAssetTracker.h"
#include "ApexString.h"
#include "ResourceProviderIntl.h"
#include "ApexAuthorableObject.h"
#include "ImpactEmitterAssetParameters.h"
#include "ImpactEmitterActorParameters.h"
#include "EmitterAssetPreviewParameters.h"
#include "ImpactObjectEvent.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ApexAuthorableObject.h"

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace emitter
{

class ImpactEmitterActorImpl;
class ModuleEmitterImpl;

///Impact emitter actor descriptor. Used to create Impact emitter actors.
class ImpactEmitterActorDesc : public ApexDesc
{
public:

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ImpactEmitterActorDesc() : ApexDesc()
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
	}
};


/**
	Descriptor used to create a Destructible preview.
*/
class ImpactEmitterPreviewDesc : public ApexDesc
{
public:

	/**
	\brief Constructor sets to default.
	*/
	PX_INLINE		ImpactEmitterPreviewDesc()
	{
		setToDefault();
	}


	/**
	\brief Resets descriptor to default settings.
	*/
	PX_INLINE void	setToDefault()
	{
		ApexDesc::setToDefault();

		mPose = PxIdentity;
		mScale = 1.0f;
	}

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const
	{
		return ApexDesc::isValid();
	}

	/**
		Initial global pose of preview mesh
	*/
	PxMat44	mPose;
	float			mScale;
};


class ImpactEmitterAssetImpl :	public ImpactEmitterAsset,
	public ApexResourceInterface,
	public ApexResource,
	public NvParameterized::SerializationCallback,
	public ApexRWLockable
{
	friend class ImpactEmitterAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ImpactEmitterAssetImpl(ModuleEmitterImpl*, ResourceList&, const char* name);
	ImpactEmitterAssetImpl(ModuleEmitterImpl* module,
	                   ResourceList& list,
	                   NvParameterized::Interface* params,
	                   const char* name);

	~ImpactEmitterAssetImpl();

	/* Asset */
	const char* 			    getName() const
	{
		READ_ZONE();
		return mName.c_str();
	}
	AuthObjTypeID			    getObjTypeID() const
	{
		READ_ZONE();
		return mAssetTypeID;
	}
	const char* 			    getObjTypeName() const
	{
		READ_ZONE();
		return getClassName();
	}
	uint32_t				forceLoadAssets();

	/* ApexInterface */
	virtual void			    release();

	/* ApexResourceInterface, ApexResource */
	uint32_t					    getListIndex() const
	{
		return m_listIndex;
	}
	void					    setListIndex(class ResourceList& list, uint32_t index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	uint32_t                querySetID(const char* setName);
	void						getSetNames(const char** outSetNames, uint32_t& inOutNameCount) const;

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
	NvParameterized::Interface* getDefaultActorDesc();
	NvParameterized::Interface* getDefaultAssetPreviewDesc();
	virtual Actor* createApexActor(const NvParameterized::Interface& parms, Scene& apexScene);
	virtual AssetPreview* createApexAssetPreview(const NvParameterized::Interface& params, AssetPreviewScene* previewScene);
	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/) const
	{
		READ_ZONE();
		return true; // TODO implement this method
	}

	virtual bool isDirty() const
	{
		READ_ZONE();
		return false;
	}

protected:
	/* Typical asset members */
	static const char* 		    getClassName()
	{
		return IMPACT_EMITTER_AUTHORING_TYPE_NAME;
	}
	static AuthObjTypeID	    mAssetTypeID;
	void						initializeAssetNameTable();
	void                        destroy();

	/* NvParameterized Serialization callbacks */
	void						preSerialize(void* userData_ = NULL);
	void						postDeserialize(void* userData_ = NULL);
	void						buildEventNameIndexMap();

	ModuleEmitterImpl*              mModule;
	ResourceList              mEmitterActors;
	ApexSimpleString            mName;

	/* objects that assist in force loading and proper "assets own assets" behavior */
	ApexAssetTracker			mIofxAssetTracker;
	ApexAssetTracker			mIosAssetTracker;

	/* asset values */
	class EventNameIndexMap : public UserAllocated
	{
	public:
		ApexSimpleString		eventSetName;
		physx::Array<uint16_t>	eventIndices;
	};

	ImpactEmitterAssetParameters*   	mParams;
	ImpactEmitterActorParameters*   	mDefaultActorParams;
	EmitterAssetPreviewParameters*      mDefaultPreviewParams;
	physx::Array<EventNameIndexMap*> mEventNameIndexMaps;

	friend class ModuleEmitterImpl;
	friend class ImpactEmitterActorImpl;
	friend class ImpactEmitterParticleEvent;

	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;
};

#ifndef WITHOUT_APEX_AUTHORING
class ImpactEmitterAssetAuthoringImpl : public ImpactEmitterAssetAuthoring, public ApexAssetAuthoring, public ImpactEmitterAssetImpl
{
protected:
	APEX_RW_LOCKABLE_BOILERPLATE

	ImpactEmitterAssetAuthoringImpl(ModuleEmitterImpl* m, ResourceList& l) :
		ImpactEmitterAssetImpl(m, l, "ImpactEmitterAuthor") {}

	ImpactEmitterAssetAuthoringImpl(ModuleEmitterImpl* m, ResourceList& l, const char* name) :
		ImpactEmitterAssetImpl(m, l, name) {}

	ImpactEmitterAssetAuthoringImpl(ModuleEmitterImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		ImpactEmitterAssetImpl(m, l, params, name) {}

public:
	void			release();
	const char* 			getName(void) const
	{
		READ_ZONE();
		return ImpactEmitterAssetImpl::getName();
	}
	const char* 	getObjTypeName() const
	{
		READ_ZONE();
		return ImpactEmitterAssetImpl::getClassName();
	}
	virtual bool			prepareForPlatform(nvidia::apex::PlatformTag)
	{
		WRITE_ZONE();
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}
	void setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		WRITE_ZONE();
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	NvParameterized::Interface* getNvParameterized() const
	{
		READ_ZONE();
		return (NvParameterized::Interface*)getAssetNvParameterized();
	}
	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		WRITE_ZONE();
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}

	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;
};
#endif

}
} // end namespace nvidia

#endif // IMPACT_EMITTER_ASSET_IMPL_H
