/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FORCEFIELD_ACTOR_IMPL_H__
#define __FORCEFIELD_ACTOR_IMPL_H__

#include "Apex.h"

#include "ForceFieldAsset.h"
#include "ForceFieldActor.h"
#include "ForceFieldAssetImpl.h"
#include "ApexActor.h"
#include "ApexString.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"
#include "FieldSamplerIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#endif

#include "ForceFieldFSCommon.h"

class ForceFieldAssetParams;

namespace nvidia
{
namespace forcefield
{

/*
PX_INLINE bool operator != (const GroupsMask64& d1, const GroupsMask64& d2)
{
	return d1.bits0 != d2.bits0 || d1.bits1 != d2.bits1;
}*/
PX_INLINE bool operator != (const physx::PxFilterData& d1, const physx::PxFilterData& d2)
{
	//if (d1.word3 != d2.word3) return d1.word3 < d2.word3;
	//if (d1.word2 != d2.word2) return d1.word2 < d2.word2;
	//if (d1.word1 != d2.word1) return d1.word1 < d2.word1;
	return d1.word0 != d2.word0 || d1.word1 != d2.word1 || d1.word2 != d2.word2 || d1.word3 != d2.word3;
}

class ForceFieldAssetImpl;
class ForceFieldScene;

/**
Union class to hold all kernel parameter types. Avoided the use of templates 
for the getters, as that resulting code using traits for type safty 
was about the same amount as the non-templated one.
*/
class ForceFieldFSKernelParamsUnion
{
public:
	ForceFieldFSKernelParams& getForceFieldFSKernelParams()
	{
		return reinterpret_cast<ForceFieldFSKernelParams&>(params);
	}

	const ForceFieldFSKernelParams& getForceFieldFSKernelParams() const
	{
		return reinterpret_cast<const ForceFieldFSKernelParams&>(params);
	}

	const RadialForceFieldFSKernelParams& getRadialForceFieldFSKernelParams() const
	{
		PX_ASSERT(kernelType == ForceFieldKernelType::RADIAL);
		return reinterpret_cast<const RadialForceFieldFSKernelParams&>(params);
	}

	RadialForceFieldFSKernelParams& getRadialForceFieldFSKernelParams()
	{
		PX_ASSERT(kernelType == ForceFieldKernelType::RADIAL);
		return reinterpret_cast<RadialForceFieldFSKernelParams&>(params);
	}

	GenericForceFieldFSKernelParams& getGenericForceFieldFSKernelParams()
	{
		PX_ASSERT(kernelType == ForceFieldKernelType::GENERIC);
		return reinterpret_cast<GenericForceFieldFSKernelParams&>(params);
	}

	const GenericForceFieldFSKernelParams& getGenericForceFieldFSKernelParams() const
	{
		PX_ASSERT(kernelType == ForceFieldKernelType::GENERIC);
		return reinterpret_cast<const GenericForceFieldFSKernelParams&>(params);
	}

	ForceFieldKernelType::Enum kernelType;

private:

	union
	{
		void* alignment; //makes data aligned to pointer size
		uint8_t radial[sizeof(RadialForceFieldFSKernelParams)];
		uint8_t generic[sizeof(GenericForceFieldFSKernelParams)];
	} params;
};

class ForceFieldActorImpl : public ForceFieldActor, public ApexRWLockable, public ApexActor, public ApexActorSource, public ApexResourceInterface, public ApexResource, public FieldSamplerIntl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* ForceFieldActorImpl methods */
	ForceFieldActorImpl(const ForceFieldActorDesc&, ForceFieldAssetImpl&, ResourceList&, ForceFieldScene&);
	~ForceFieldActorImpl() {}
	ForceFieldAsset* 	getForceFieldAsset() const;

	bool				disable();
	bool				enable();
	bool				isEnable()
	{
		READ_ZONE();
		return mEnable;
	}
	PxMat44		getPose() const;
	void				setPose(const PxMat44& pose);

	float	getCurrentScale(void) const
	{
		READ_ZONE();
		return getScale();
	}

	void setCurrentScale(float scale)
	{
		WRITE_ZONE();
		setScale(scale);
	}

	PX_DEPRECATED float		getScale() const
	{
		READ_ZONE();
		return 0.0f;
	}

	PX_DEPRECATED void				setScale(float scale);

	const char*			getName() const
	{
		READ_ZONE();
		return mName.c_str();
	}
	void				setName(const char* name)
	{
		WRITE_ZONE();
		mName = name;
	}

	void				setStrength(const float strength);
	void				setLifetime(const float lifetime);

	//kernel specific functionality
	void				setRadialFalloffType(const char* type);
	void				setRadialFalloffMultiplier(const float multiplier);

	// deprecated
	void				setFalloffType(const char* type);
	void				setFalloffMultiplier(const float multiplier);

	void                updatePoseAndBounds();  // Called by ExampleScene::fetchResults()

	/* ApexResourceInterface, ApexResource */
	void				release();
	uint32_t		getListIndex() const
	{
		return m_listIndex;
	}
	void				setListIndex(class ResourceList& list, uint32_t index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	/* Actor, ApexActor */
	void                destroy();
	Asset*		getOwner() const;

	/* PhysX scene management */
	void				setPhysXScene(PxScene*);
	PxScene*			getPhysXScene() const;

	void				getLodRange(float& min, float& max, bool& intOnly) const;
	float		getActiveLod() const;
	void				forceLod(float lod);
	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		WRITE_ZONE();
		ApexActor::setEnableDebugVisualization(state);
	}

	/* FieldSamplerIntl */
	virtual bool		updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	virtual PxVec3 queryFieldSamplerVelocity() const
	{
		return PxVec3(0.0f);
	}

protected:
	void				updateForceField(float dt);
	void				releaseForceField();

protected:
	ForceFieldScene*		mForceFieldScene;
	
	//not used, setters and getters deprecated
	//float			mScale;

	uint32_t			mFlags;

	ApexSimpleString		mName;

	ForceFieldAssetImpl*		mAsset;

	bool					mEnable;
	float			mElapsedTime;

	/* Force field actor parameters */
	float			mLifetime;
	void					initActorParams(const PxMat44& initialPose);

	/* Field Sampler Stuff */
	bool					mFieldSamplerChanged;
	void					initFieldSampler(const ForceFieldActorDesc& desc);
	void					releaseFieldSampler();

	/* Debug Rendering Stuff */
	void					visualize();
	void					visualizeIncludeShape();
	void					visualizeForces();

	ForceFieldFSKernelParamsUnion mKernelParams;
	ForceFieldFSKernelParamsUnion mKernelExecutionParams; //buffered data

	friend class ForceFieldScene;
};

class ForceFieldActorCPU : public ForceFieldActorImpl
{
public:
	ForceFieldActorCPU(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list, ForceFieldScene& scene);
	~ForceFieldActorCPU();

	/* FieldSamplerIntl */
	virtual void executeFieldSampler(const ExecuteData& data);

	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		ApexActor::setEnableDebugVisualization(state);
	}


private:
};

#if APEX_CUDA_SUPPORT

class ForceFieldActorGPU : public ForceFieldActorCPU
{
public:
	ForceFieldActorGPU(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list, ForceFieldScene& scene);
	~ForceFieldActorGPU();

	/* FieldSamplerIntl */
	virtual bool updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const;

private:
	ApexCudaConstMemGroup				mConstMemGroup;
	InplaceHandle<RadialForceFieldFSKernelParams>	mParamsHandle;
};

#endif

}
} // end namespace nvidia

#endif
