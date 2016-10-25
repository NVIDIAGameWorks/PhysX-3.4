/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IOFX_ASSET_IMPL_H
#define IOFX_ASSET_IMPL_H

#include "Apex.h"
#include "ApexUsingNamespace.h"
#include "ApexSDKHelpers.h"
#include "ApexAssetAuthoring.h"
#include "ApexAssetTracker.h"
#include "ApexContext.h"
#include "IofxAsset.h"
#include "Modifier.h"
#include "ApexString.h"
#include "ResourceProviderIntl.h"
#include "ApexAuthorableObject.h"
#include "IofxAssetParameters.h"
#include "MeshIofxParameters.h"
#include "SpriteIofxParameters.h"
#include "ParamArray.h"
#include "ModifierImpl.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"
#include "ApexAuthorableObject.h"

// instead of having an initial color modifier, just drop
// in 4 color vs life modifiers instead
#define IOFX_SLOW_COMPOSITE_MODIFIERS 1

namespace nvidia
{
namespace iofx
{

typedef physx::Array<Modifier*> ModifierStack;

class ModuleIofxImpl;

class IofxAssetImpl : public IofxAsset,
	public ResourceList,
	public ApexResourceInterface,
	public ApexResource,
	public NvParameterized::SerializationCallback,
	public ApexContext,
	public ApexRWLockable
{
	friend class IofxAssetDummyAuthoring;
protected:
	IofxAssetImpl(ModuleIofxImpl* module, ResourceList& list, const char* name);
	IofxAssetImpl(ModuleIofxImpl* module,
	          ResourceList& list,
	          NvParameterized::Interface* params,
	          const char* name);

public:
	APEX_RW_LOCKABLE_BOILERPLATE

	~IofxAssetImpl();

	// Asset
	virtual void								release();
	virtual const char* 						getName(void) const
	{
		READ_ZONE();
		return mName.c_str();
	}
	virtual AuthObjTypeID						getObjTypeID() const
	{
		READ_ZONE();
		return mAssetTypeID;
	}
	virtual const char* 						getObjTypeName() const
	{
		READ_ZONE();
		return getClassName();
	}
	virtual uint32_t						forceLoadAssets();
	// ApexResourceInterface
	virtual void								setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	virtual uint32_t						getListIndex() const
	{
		return m_listIndex;
	}

	// Context
	virtual void								removeAllActors();
	RenderableIterator*					createRenderableIterator()
	{
		return ApexContext::createRenderableIterator();
	}
	void										releaseRenderableIterator(RenderableIterator& iter)
	{
		ApexContext::releaseRenderableIterator(iter);
	}

	void										addDependentActor(ApexActor* actor);

	bool isOpaqueMesh(uint32_t index) const;

	virtual uint32_t						getMeshAssetCount() const
	{
		READ_ZONE();
		return mRenderMeshList ? mRenderMeshList->size() : 0;
	}
	virtual const char*							getMeshAssetName(uint32_t index) const;
	virtual uint32_t   						getMeshAssetWeight(uint32_t index) const;
	virtual const char*							getSpriteMaterialName() const;
	virtual uint32_t						getContinuousModifierCount() const
	{
		return mContinuousModifierStack.size();
	}
	virtual const Modifier*					getSpawnModifiers(uint32_t& outCount) const
	{
		READ_ZONE();
		outCount = mSpawnModifierStack.size();
		return mSpawnModifierStack.front();
	}
	virtual const Modifier*					getContinuousModifiers(uint32_t& outCount) const
	{
		READ_ZONE();
		outCount = mContinuousModifierStack.size();
		return mContinuousModifierStack.front();
	}

	template<class ModifierType>
	PX_INLINE float								getMaxYFromCurveModifier(Modifier* modifier) const
	{
		float maxScale = 0.0f;
		ModifierType* m = DYNAMIC_CAST(ModifierType*)(modifier);
		const nvidia::apex::Curve* curve = m->getFunction();
		uint32_t numControlPoints = 0;
		const Vec2R* controlPoints = curve->getControlPoints(numControlPoints);
		for (uint32_t j = 0; j < numControlPoints; ++j)
		{
			maxScale = PxMax(maxScale, controlPoints->y);
			++controlPoints;
		}
		return maxScale;
	}

	virtual float						getScaleUpperBound(float maxVelocity) const
	{
		// check all modifiers and return the biggest scale they can produce
		READ_ZONE();
		float scale = 1.0f;

		for (uint32_t i = 0; i < mSpawnModifierStack.size(); ++i)
		{
			Modifier* modifier = mSpawnModifierStack[i];

			switch (modifier->getModifierType())
			{
			case ModifierType_SimpleScale:
			{
				SimpleScaleModifierImpl* m = DYNAMIC_CAST(SimpleScaleModifierImpl*)(modifier);
				scale *= m->getScaleFactor().maxElement();
				break;
			}
			case ModifierType_RandomScale:
			{
				RandomScaleModifierImpl* m = DYNAMIC_CAST(RandomScaleModifierImpl*)(modifier);
				scale *= m->getScaleFactor().maximum;
				break;
			}
			default:
				break;
			}

		}
		for (uint32_t i = 0; i < mContinuousModifierStack.size(); ++i)
		{
			Modifier* modifier = mContinuousModifierStack[i];
			switch (mContinuousModifierStack[i]->getModifierType())
			{
			case ModifierType_ScaleAlongVelocity:
			{
				ScaleAlongVelocityModifierImpl* m = DYNAMIC_CAST(ScaleAlongVelocityModifierImpl*)(modifier);
				scale *= m->getScaleFactor() * maxVelocity;
				break;
			}
			case ModifierType_ScaleVsLife:
			{
				float maxScale = getMaxYFromCurveModifier<ScaleVsLifeModifierImpl>(modifier);
				if (maxScale != 0.0f)
				{
					scale *= maxScale;
				}
				break;
			}
			case ModifierType_ScaleVsDensity:
			{
				float maxScale = getMaxYFromCurveModifier<ScaleVsDensityModifierImpl>(modifier);
				if (maxScale != 0.0f)
				{
					scale *= maxScale;
				}
				break;
			}
			case ModifierType_ScaleVsCameraDistance:
			{
				float maxScale = getMaxYFromCurveModifier<ScaleVsCameraDistanceModifierImpl>(modifier);
				if (maxScale != 0.0f)
				{
					scale *= maxScale;
				}
				break;
			}
			case ModifierType_OrientScaleAlongScreenVelocity:
			{
				OrientScaleAlongScreenVelocityModifierImpl* m = DYNAMIC_CAST(OrientScaleAlongScreenVelocityModifierImpl*)(modifier);
				scale *= m->getScalePerVelocity() * maxVelocity;
				break;
			}
			default:
				break;
			}
		}

		return scale;
	}

	uint32_t								getSpriteSemanticsBitmap() const
	{
		return mSpriteSemanticBitmap;
	}
	PX_INLINE bool								isSpriteSemanticUsed(IofxRenderSemantic::Enum semantic)
	{
		return (((1 << semantic) & mSpriteSemanticBitmap) ? true : false);
	}
	void										setSpriteSemanticsUsed(uint32_t spriteSemanticsBitmap);

	uint32_t								getMeshSemanticsBitmap() const
	{
		return mMeshSemanticBitmap;
	}
	PX_INLINE bool								isMeshSemanticUsed(IofxRenderSemantic::Enum semantic)
	{
		return (((1 << semantic) & mMeshSemanticBitmap) ? true : false);
	}
	void										setMeshSemanticsUsed(uint32_t meshSemanticsBitmap);

	/* objects that assist in force loading and proper "assets own assets" behavior */
	ApexAssetTracker							mRenderMeshAssetTracker;
	ApexAssetTracker							mSpriteMaterialAssetTracker;

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
		READ_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	};

	NvParameterized::Interface* getDefaultAssetPreviewDesc()
	{
		READ_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual Actor* createApexActor(const NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/)
	{
		READ_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual AssetPreview* createApexAssetPreview(const NvParameterized::Interface& /*params*/, AssetPreviewScene* /*previewScene*/)
	{
		READ_ZONE();
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual bool isValidForActorCreation(const ::NvParameterized::Interface& /*parms*/, Scene& /*apexScene*/) const
	{
		READ_ZONE();
		return true; // todo, implement!
	}

	virtual bool isDirty() const
	{
		READ_ZONE();
		return false;
	}

	void										destroy();
	static const char* 							getClassName()
	{
		return IOFX_AUTHORING_TYPE_NAME;
	}
	static AuthObjTypeID						mAssetTypeID;

	struct RenderMesh
	{
		uint32_t        mWeight;
		ApexSimpleString	mMeshAssetName;
	};

	// authorable data
	IofxAssetParameters*					   	mParams;
	physx::Array<RenderMesh>					mRenderMeshes;
	ModifierStack								mSpawnModifierStack;
	ModifierStack								mContinuousModifierStack;
	ParamArray<MeshIofxParametersNS::meshProperties_Type> *mRenderMeshList;
	SpriteIofxParameters*				   		mSpriteParams;

	// runtime data
	ModuleIofxImpl*							        mModule;
	ApexSimpleString							mName;
	uint32_t								mSpriteSemanticBitmap;
	uint32_t								mMeshSemanticBitmap;

#if IOFX_SLOW_COMPOSITE_MODIFIERS
	physx::Array<NvParameterized::Interface*>	mCompositeParams;
#endif

	PX_INLINE ModifierStack&					getModifierStack(uint32_t modStage)
	{
		switch (modStage)
		{
		case ModifierStage_Spawn:
			return mSpawnModifierStack;
		case ModifierStage_Continuous:
			return mContinuousModifierStack;
		default:
			PX_ALWAYS_ASSERT();
		};
		return mSpawnModifierStack; // should never get here.
	}

	PX_INLINE const ModifierStack&				getModifierStack(uint32_t modStage) const
	{
		switch (modStage)
		{
		case ModifierStage_Spawn:
			return mSpawnModifierStack;
		case ModifierStage_Continuous:
			return mContinuousModifierStack;
		default:
			PX_ALWAYS_ASSERT();
		};
		return mSpawnModifierStack; // should never get here.
	}

	PX_INLINE NvParameterized::ErrorType getModifierStack(uint32_t modStage, NvParameterized::Handle& h)
	{
		switch (modStage)
		{
		case ModifierStage_Spawn:
			return mParams->getParameterHandle("spawnModifierList", h);
		case ModifierStage_Continuous:
			return mParams->getParameterHandle("continuousModifierList", h);
		default:
			PX_ALWAYS_ASSERT();
		};
		return NvParameterized::ERROR_INDEX_OUT_OF_RANGE; // should never get here.
	}

	PX_INLINE NvParameterized::ErrorType getModifierStack(uint32_t modStage, NvParameterized::Handle& h) const
	{
		switch (modStage)
		{
		case ModifierStage_Spawn:
			return mParams->getParameterHandle("spawnModifierList", h);
		case ModifierStage_Continuous:
			return mParams->getParameterHandle("continuousModifierList", h);
		default:
			PX_ALWAYS_ASSERT();
		};
		return NvParameterized::ERROR_INDEX_OUT_OF_RANGE; // should never get here.
	}

	bool isSortingEnabled() const;

	/* NvParameterized Serialization callbacks */
	void						preSerialize(void* userData_ = NULL);
	void						postDeserialize(void* userData_ = NULL);

	// initialize a table of assets and resource IDs for resource tracking
	void						initializeAssetNameTable();

	uint32_t						getPubStateSize() const;
	uint32_t						getPrivStateSize() const;

	friend class ModuleIofxImpl;
	template <class T_Module, class T_Asset, class T_AssetAuthoring> friend class nvidia::apex::ApexAuthorableObject;
};

#ifndef WITHOUT_APEX_AUTHORING
class IofxAssetAuthoringImpl : public IofxAssetAuthoring, public IofxAssetImpl, public ApexAssetAuthoring
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	IofxAssetAuthoringImpl(ModuleIofxImpl* module, ResourceList& list) :
		IofxAssetImpl(module, list, "IofxAssetAuthoringImpl") {}

	IofxAssetAuthoringImpl(ModuleIofxImpl* module, ResourceList& list, const char* name) :
		IofxAssetImpl(module, list, name) {}

	IofxAssetAuthoringImpl(ModuleIofxImpl* module, ResourceList& list, NvParameterized::Interface* params, const char* name) :
		IofxAssetImpl(module, list, params, name) {}

	virtual void	        release();

	virtual const char* 			getName(void) const
	{
		READ_ZONE();
		return IofxAssetImpl::getName();
	}
	virtual const char* 			getObjTypeName() const;
	virtual bool					prepareForPlatform(nvidia::apex::PlatformTag)
	{
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}

	void setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}

#if IOFX_AUTHORING_API_ENABLED
	virtual void	        setMeshAssetName(const char* meshAssetName, uint32_t meshIndex = 0);
	virtual void	        setMeshAssetWeight(const uint32_t weight, uint32_t meshIndex = 0);
	virtual void            setSpriteMaterialName(const char* spriteMaterialName);

	virtual uint32_t	getMeshAssetCount()	const
	{
		return IofxAssetImpl::getMeshAssetCount();
	}
	virtual const char*		getMeshAssetName(uint32_t index) const
	{
		return IofxAssetImpl::getMeshAssetName(index);
	}
	virtual uint32_t   	getMeshAssetWeight(uint32_t index) const
	{
		return IofxAssetImpl::getMeshAssetWeight(index);
	}
	virtual const char*		getSpriteMaterialName() const
	{
		return IofxAssetImpl::getSpriteMaterialName();
	}

	virtual Modifier*		createModifier(uint32_t modStage, uint32_t modType);

	virtual void			removeModifier(uint32_t modStage, uint32_t position);
	virtual uint32_t	findModifier(uint32_t modStage, Modifier* modifier);
	virtual Modifier* 	getModifier(uint32_t modStage, uint32_t position) const;
	virtual uint32_t	getModifierCount(uint32_t modStage) const;
#endif

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
		READ_ZONE();
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}
	uint32_t			getAssetTarget() const;
};
#endif

}
} // namespace nvidia

#endif // IOFX_ASSET_IMPL_H
