/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __VORTEX_FS_ACTOR_IMPL_H__
#define __VORTEX_FS_ACTOR_IMPL_H__

#include "BasicFSActor.h"
#include "VortexFSActor.h"

#include "VortexFSCommon.h"

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

class VortexFSAsset;
class BasicFSScene;
class VortexFSActorParams;

class VortexFSActorImpl : public BasicFSActor, public VortexFSActor, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* VortexFSActor methods */
	VortexFSActorImpl(const VortexFSActorParams&, VortexFSAsset&, ResourceList&, BasicFSScene&);
	~VortexFSActorImpl();

	BasicFSAsset* 		getVortexFSAsset() const;

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
	void					setAxis(const PxVec3& axis)
	{
		WRITE_ZONE();
		mAxis = axis;
		mFieldSamplerChanged = true;
	}
	void					setHeight(float height)
	{
		WRITE_ZONE();
		mHeight = height;
		mFieldSamplerChanged = true;
		mDebugShapeChanged = true;
	}
	void					setBottomRadius(float radius)
	{
		mBottomRadius = radius;
		mFieldSamplerChanged = true;
		mDebugShapeChanged = true;
	}
	void					setTopRadius(float radius)
	{
		WRITE_ZONE();
		mTopRadius = radius;
		mFieldSamplerChanged = true;
		mDebugShapeChanged = true;
	}

	void					setBottomSphericalForce(bool isEnabled)
	{
		WRITE_ZONE();
		mBottomSphericalForce = isEnabled;
		mFieldSamplerChanged = true;
	}
	void					setTopSphericalForce(bool isEnabled)
	{
		WRITE_ZONE();
		mTopSphericalForce = isEnabled;
		mFieldSamplerChanged = true;
	}

	void					setRotationalStrength(float strength);
	void					setRadialStrength(float strength);
	void					setLiftStrength(float strength);

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

	///Sets the uniform overall object scale
	virtual void				setCurrentScale(float scale)
	{
		WRITE_ZONE();
		mScale = scale;
		mFieldSamplerChanged = true;
	}

	//Retrieves the uniform overall object scale
	virtual float				getCurrentScale(void) const
	{
		READ_ZONE();
		return mScale;
	}

protected:
	VortexFSAsset*			mAsset;
	
	bool					mBottomSphericalForce;
	bool					mTopSphericalForce;

	PxVec3			mAxis;
	float			mHeight;
	float			mBottomRadius;
	float			mTopRadius;

	float			mRotationalStrength;
	float			mRadialStrength;
	float			mLiftStrength;

	VortexFSParams			mExecuteParams; 

	PxTransform	mDirToWorld;

	bool						mDebugShapeChanged;
	nvidia::Array<PxVec3> mDebugPoints;

	friend class BasicFSScene;
};

class VortexFSActorCPU : public VortexFSActorImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	VortexFSActorCPU(const VortexFSActorParams&, VortexFSAsset&, ResourceList&, BasicFSScene&);
	~VortexFSActorCPU();

	/* FieldSamplerIntl */
	virtual void executeFieldSampler(const ExecuteData& data);

private:
};

#if APEX_CUDA_SUPPORT

class VortexFSActorGPU : public VortexFSActorCPU
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	VortexFSActorGPU(const VortexFSActorParams&, VortexFSAsset&, ResourceList&, BasicFSScene&);
	~VortexFSActorGPU();

	/* FieldSamplerIntl */
	virtual bool updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
	{
		info.executeType = 4;
		info.executeParamsHandle = mParamsHandle;
	}

private:
	ApexCudaConstMemGroup			mConstMemGroup;
	InplaceHandle<VortexFSParams>	mParamsHandle;

};

#endif

}
} // end namespace nvidia

#endif
