/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __ATTRACTOR_FS_ACTOR_IMPL_H__
#define __ATTRACTOR_FS_ACTOR_IMPL_H__

#include "BasicFSActor.h"
#include "AttractorFSActor.h"
#include "ApexRWLockable.h"
#include "AttractorFSCommon.h"


namespace nvidia
{
namespace apex
{
class RenderMeshActor;
}
namespace basicfs
{

class AttractorFSAsset;
class BasicFSScene;
class AttractorFSActorParams;

class AttractorFSActorImpl : public BasicFSActor, public AttractorFSActor, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* AttractorFSActor methods */
	AttractorFSActorImpl(const AttractorFSActorParams&, AttractorFSAsset&, ResourceList&, BasicFSScene&);
	~AttractorFSActorImpl();

	BasicFSAsset* 		getAttractorFSAsset() const;

	PxVec3			getCurrentPosition() const
	{
		return mPose.p;
	}
	void					setCurrentPosition(const PxVec3& pos)
	{
		mPose.p = pos;
		mFieldSamplerChanged = true;
	}
	void					setFieldRadius(float radius)
	{
		mRadius = radius;
		mFieldSamplerChanged = true;
	}
	void					setConstFieldStrength(float strength);

	void					setVariableFieldStrength(float strength);

	void					setEnabled(bool isEnabled)
	{
		mFieldSamplerEnabled = isEnabled;
	}

	/* Renderable, RenderDataProvider */
	void					updateRenderResources(bool rewriteBuffers, void* userRenderData);
	void					dispatchRenderResources(UserRenderer& renderer);

	PxBounds3				getBounds() const
	{
		return ApexRenderable::getBounds();
	}

	void					lockRenderResources()
	{
		ApexRenderable::renderDataLock();
	}
	void					unlockRenderResources()
	{
		ApexRenderable::renderDataUnLock();
	}

	void					getLodRange(float& min, float& max, bool& intOnly) const;
	float			getActiveLod() const;
	void					forceLod(float lod);
	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		ApexActor::setEnableDebugVisualization(state);
	}

	Renderable*		getRenderable()
	{
		return this;
	}
	Actor*			getActor()
	{
		return this;
	}

	/* Resource, ApexResource */
	void					release();

	/* Actor, ApexActor */
	void					destroy();
	Asset*			getOwner() const;

	virtual void			simulate(float dt);

	virtual void			visualize();

	/* FieldSamplerIntl */
	virtual bool			updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	///Sets the uniform overall object scale
	virtual void				setCurrentScale(float scale) 
	{
		mScale = scale;
		mFieldSamplerChanged = true;
	}

	//Retrieves the uniform overall object scale
	virtual float				getCurrentScale(void) const
	{
		return mScale;
	}

protected:
	AttractorFSAsset*		mAsset;

	float			mRadius;

	float			mConstFieldStrength;
	float			mVariableFieldStrength;

	AttractorFSParams		mExecuteParams; 

	nvidia::Array<PxVec3> mDebugPoints;

	friend class BasicFSScene;
};

class AttractorFSActorCPU : public AttractorFSActorImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	AttractorFSActorCPU(const AttractorFSActorParams&, AttractorFSAsset&, ResourceList&, BasicFSScene&);
	~AttractorFSActorCPU();

	/* FieldSamplerIntl */
	virtual void executeFieldSampler(const ExecuteData& data);

private:
};

#if APEX_CUDA_SUPPORT

class AttractorFSActorGPU : public AttractorFSActorCPU
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	AttractorFSActorGPU(const AttractorFSActorParams&, AttractorFSAsset&, ResourceList&, BasicFSScene&);
	~AttractorFSActorGPU();

	/* FieldSamplerIntl */
	virtual bool updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
	{
		info.executeType = 2;
		info.executeParamsHandle = mParamsHandle;
	}

private:
	ApexCudaConstMemGroup				mConstMemGroup;
	InplaceHandle<AttractorFSParams>	mParamsHandle;

};

#endif

}
} // end namespace nvidia

#endif
