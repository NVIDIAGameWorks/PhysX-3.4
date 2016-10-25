/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FORCEFIELD_ASSET_IMPL_H
#define FORCEFIELD_ASSET_IMPL_H

#include "Apex.h"
#include "ApexUsingNamespace.h"
#include "ForceFieldAsset.h"
#include "ForceFieldActor.h"
#include "ForceFieldPreview.h"
#include "ApexSDKHelpers.h"
#include "ModuleForceFieldImpl.h"
#include "ApexAssetAuthoring.h"
#include "ApexString.h"
#include "ApexAssetTracker.h"
#include "ApexAuthorableObject.h"
#include "ForceFieldAssetParams.h"
#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ApexAuthorableObject.h"

namespace nvidia
{
namespace forcefield
{

class ForceFieldActorDesc : public ApexDesc
{
public:
	physx::PxFilterData samplerFilterData;
	physx::PxFilterData boundaryFilterData;

	PxMat44 initialPose;
	
	//deprecated, has no effect
	float	 scale;
	PxActor* nxActor;
	const char* actorName;

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE ForceFieldActorDesc() : ApexDesc()
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
		initialPose = PxMat44(PxIdentity);
		scale = 1.0f;
		nxActor = NULL;
		actorName = NULL;
	}
};

/**
\brief Descriptor for a ForceField Asset
*/
class ForceFieldPreviewDesc
{
public:
	ForceFieldPreviewDesc() :
		mPose(PxMat44()),
		mIconScale(1.0f),
		mPreviewDetail(APEX_FORCEFIELD::FORCEFIELD_DRAW_ICON)
	{
		mPose = PxMat44(PxIdentity);
	};

	/**
	\brief The pose that translates from explosion preview coordinates to world coordinates.
	*/
	PxMat44							mPose;
	/**
	\brief The scale of the icon.
	*/
	float							mIconScale;
	/**
	\brief The detail options of the preview drawing
	*/
	uint32_t							mPreviewDetail;
};


class ForceFieldActorImpl;

class ForceFieldAssetImpl : public ForceFieldAsset, public ApexResourceInterface, public ApexResource, public ApexRWLockable
{
	friend class ForceFieldAssetDummyAuthoring;
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ForceFieldAssetImpl(ModuleForceFieldImpl*, ResourceList&, const char* name);
	ForceFieldAssetImpl(ModuleForceFieldImpl*, ResourceList&, NvParameterized::Interface*, const char*);

	~ForceFieldAssetImpl();

	/* Asset */
	const char* 					getName() const
	{
		return mName.c_str();
	}
	AuthObjTypeID					getObjTypeID() const
	{
		READ_ZONE();
		return mAssetTypeID;
	}
	const char* 					getObjTypeName() const
	{
		READ_ZONE();
		return getClassName();
	}
	uint32_t					forceLoadAssets();

	/* ApexInterface */
	virtual void					release()
	{
		mModule->mSdk->releaseAsset(*this);
	}

	/* ApexResourceInterface, ApexResource */
	uint32_t					getListIndex() const
	{
		return m_listIndex;
	}
	void							setListIndex(class ResourceList& list, uint32_t index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	/* ForceFieldAsset specific methods */
	void							releaseForceFieldActor(ForceFieldActor&);
	const ForceFieldAssetParams&	getForceFieldParameters() const
	{
		return *mParams;
	}
	float					getDefaultScale() const
	{
		READ_ZONE();
		return mParams->defScale;
	}
	void							destroy();
	ForceFieldPreview*			createForceFieldPreview(const ForceFieldPreviewDesc& desc, AssetPreviewScene* previewScene);
	ForceFieldPreview*			createForceFieldPreviewImpl(const ForceFieldPreviewDesc& desc, ForceFieldAssetImpl* forceFieldAsset, AssetPreviewScene* previewScene);
	void							releaseForceFieldPreview(ForceFieldPreview& preview);

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
	NvParameterized::Interface* getDefaultActorDesc();
	NvParameterized::Interface* getDefaultAssetPreviewDesc();
	virtual Actor* createApexActor(const NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/);
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
	static const char* 				getClassName()
	{
		return FORCEFIELD_AUTHORING_TYPE_NAME;
	}
	static AuthObjTypeID			mAssetTypeID;

	ModuleForceFieldImpl*				mModule;

	ResourceList					mForceFieldActors;
	ApexSimpleString				mName;
	ForceFieldAssetParams*			mParams;
	ForceFieldActorParams*			mDefaultActorParams;
	ForceFieldAssetPreviewParams*	mDefaultPreviewParams;

	GenericForceFieldKernelParams*	mGenericParams;
	RadialForceFieldKernelParams*	mRadialParams;
	ForceFieldFalloffParams*		mFalloffParams;
	ForceFieldNoiseParams*			mNoiseParams;

	void							initializeAssetNameTable();

	friend class ModuleForceFieldImpl;
	friend class ForceFieldActorImpl;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;
};

#ifndef WITHOUT_APEX_AUTHORING
class ForceFieldAssetAuthoringImpl : public ForceFieldAssetImpl, public ApexAssetAuthoring, public ForceFieldAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* ForceFieldAssetAuthoring */
	ForceFieldAssetAuthoringImpl(ModuleForceFieldImpl* m, ResourceList& l) :
		ForceFieldAssetImpl(m, l, "ForceFieldAssetAuthoringImpl") {}

	ForceFieldAssetAuthoringImpl(ModuleForceFieldImpl* m, ResourceList& l, const char* name) :
		ForceFieldAssetImpl(m, l, name) {}

	ForceFieldAssetAuthoringImpl(ModuleForceFieldImpl* m, ResourceList& l, NvParameterized::Interface* params, const char* name) :
		ForceFieldAssetImpl(m, l, params, name) {}

	~ForceFieldAssetAuthoringImpl() {}

	void							destroy()
	{
		delete this;
	}

	/* AssetAuthoring */
	const char* 					getName(void) const
	{
		READ_ZONE();
		return ForceFieldAssetImpl::getName();
	}
	const char* 					getObjTypeName() const
	{
		return ForceFieldAssetImpl::getClassName();
	}
	virtual bool					prepareForPlatform(nvidia::apex::PlatformTag)
	{
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}

	void setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

	/* ApexInterface */
	virtual void					release()
	{
		mModule->mSdk->releaseAssetAuthoring(*this);
	}

	NvParameterized::Interface* getNvParameterized() const
	{
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
};
#endif

}
} // end namespace nvidia

#endif // FORCEFIELD_ASSET_IMPL_H
