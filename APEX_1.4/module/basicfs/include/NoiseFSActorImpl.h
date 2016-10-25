/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NOISE_FS_ACTOR_IMPL_H__
#define __NOISE_FS_ACTOR_IMPL_H__

#include "BasicFSActor.h"
#include "NoiseFSActor.h"
#include "ApexRWLockable.h"
#include "NoiseFSCommon.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace apex
{
class RenderMeshActor;	
}	
namespace basicfs
{

class NoiseFSAsset;
class BasicFSScene;
class NoiseFSActorParams;

class NoiseFSActorImpl : public BasicFSActor, public NoiseFSActor, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* NoiseFSActor methods */
	NoiseFSActorImpl(const NoiseFSActorParams&, NoiseFSAsset&, ResourceList&, BasicFSScene&);
	~NoiseFSActorImpl();

	BasicFSAsset* 		getNoiseFSAsset() const;

	PxMat44			getCurrentPose() const
	{
		READ_ZONE();
		return PxMat44(mPose);
	}

	void					setCurrentPose(const PxTransform& pose)
	{
		WRITE_ZONE();
		mPose = pose;
		mFieldSamplerChanged = true;
	}

	PxVec3			getCurrentPosition() const
	{		
		READ_ZONE();
		return mPose.p;
	}
	void					setCurrentPosition(const PxVec3& pos)
	{
		WRITE_ZONE();
		mPose.p = pos;
		mFieldSamplerChanged = true;
	}

	float			getCurrentScale() const
	{
		READ_ZONE();
		return mScale;
	}

	void					setCurrentScale(const float& scale)
	{
		WRITE_ZONE();
		mScale = scale;
		mFieldSamplerChanged = true;
	}

	void					setNoiseStrength(float strength);

	void					setEnabled(bool isEnabled)
	{
		WRITE_ZONE();
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

protected:
	NoiseFSAsset*				mAsset;

	NoiseFSParams				mExecuteParams; 

	friend class BasicFSScene;
};

class NoiseFSActorCPU : public NoiseFSActorImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	NoiseFSActorCPU(const NoiseFSActorParams&, NoiseFSAsset&, ResourceList&, BasicFSScene&);
	~NoiseFSActorCPU();

	/* FieldSamplerIntl */
	virtual void executeFieldSampler(const ExecuteData& data);

private:
};

#if APEX_CUDA_SUPPORT

class NoiseFSActorGPU : public NoiseFSActorCPU
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	NoiseFSActorGPU(const NoiseFSActorParams&, NoiseFSAsset&, ResourceList&, BasicFSScene&);
	~NoiseFSActorGPU();

	/* FieldSamplerIntl */
	virtual bool updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
	{
		info.executeType = 3;
		info.executeParamsHandle = mParamsHandle;
	}

private:
	ApexCudaConstMemGroup			mConstMemGroup;
	InplaceHandle<NoiseFSParams>	mParamsHandle;

};

#endif

}
} // end namespace nvidia

#endif
