/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __JET_FS_ACTOR_IMPL_H__
#define __JET_FS_ACTOR_IMPL_H__

#include "BasicFSActor.h"
#include "JetFSActor.h"
#include "ApexRWLockable.h"
#include "JetFSCommon.h"

#include "variable_oscillator.h"


namespace nvidia
{
namespace apex
{
class RenderMeshActor;
}
namespace basicfs
{

class JetFSAsset;
class BasicFSScene;
class JetFSActorParams;

class JetFSActorImpl : public BasicFSActor, public JetFSActor, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* JetFSActor methods */
	JetFSActorImpl(const JetFSActorParams&, JetFSAsset&, ResourceList&, BasicFSScene&);
	~JetFSActorImpl();

	BasicFSAsset* 		getJetFSAsset() const;

	PxMat44			getCurrentPose() const
	{
		return PxMat44(mPose);
	}

	void					setCurrentPose(const PxTransform& pose)
	{
		mPose = pose;
		mFieldSamplerChanged = true;
	}

	PxVec3			getCurrentPosition() const
	{		
		return mPose.p;
	}
	void					setCurrentPosition(const PxVec3& pos)
	{
		mPose.p = pos;
		mFieldSamplerChanged = true;
	}

	float			getCurrentScale() const
	{
		return mScale;
	}

	void					setCurrentScale(const float& scale)
	{
		mScale = scale;
		mFieldSamplerChanged = true;
	}

	void					setFieldStrength(float strength);
	void					setFieldDirection(const PxVec3& direction);

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

protected:
	JetFSAsset*				mAsset;

	PxVec3			mFieldDirection;
	variableOscillator*		mFieldDirectionVO1;
	variableOscillator*		mFieldDirectionVO2;

	float			mFieldStrength;
	variableOscillator*		mFieldStrengthVO;

	float			mStrengthVar;
	PxVec3			mLocalDirVar;
	PxTransform	mDirToWorld;

	JetFSParams				mExecuteParams; 

	nvidia::Array<PxVec3> mDebugPoints;

	friend class BasicFSScene;
};

class JetFSActorCPU : public JetFSActorImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	JetFSActorCPU(const JetFSActorParams&, JetFSAsset&, ResourceList&, BasicFSScene&);
	~JetFSActorCPU();

	/* FieldSamplerIntl */
	virtual void executeFieldSampler(const ExecuteData& data);

private:
};

#if APEX_CUDA_SUPPORT

class JetFSActorGPU : public JetFSActorCPU
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	JetFSActorGPU(const JetFSActorParams&, JetFSAsset&, ResourceList&, BasicFSScene&);
	~JetFSActorGPU();

	/* FieldSamplerIntl */
	virtual bool updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
	{
		info.executeType = 1;
		info.executeParamsHandle = mParamsHandle;
	}

private:
	ApexCudaConstMemGroup           mConstMemGroup;
	InplaceHandle<JetFSParams>		mParamsHandle;

};

#endif

}
} // end namespace nvidia

#endif
