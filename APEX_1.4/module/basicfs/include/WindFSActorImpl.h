/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __WIND_FS_ACTOR_IMPL_H__
#define __WIND_FS_ACTOR_IMPL_H__

#include "BasicFSActor.h"
#include "WindFSActor.h"

#include "WindFSCommon.h"
#include "ApexRWLockable.h"
#include "variable_oscillator.h"

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

class WindFSAsset;
class BasicFSScene;
class WindFSActorParams;

class WindFSActorImpl : public BasicFSActor, public WindFSActor, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	/* WindFSActor methods */
	WindFSActorImpl(const WindFSActorParams&, WindFSAsset&, ResourceList&, BasicFSScene&);
	~WindFSActorImpl();

	BasicFSAsset* 		getWindFSAsset() const;

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

	void					setFieldStrength(float strength);
	void					setFieldDirection(const PxVec3& direction);

	float			getFieldStrength() const
	{
		READ_ZONE();
		return mFieldStrength;
	}
	const PxVec3&	getFieldDirection() const
	{
		READ_ZONE();
		return mFieldDirBasis.column0;
	}

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

	virtual PxVec3 queryFieldSamplerVelocity() const
	{
		return mExecuteParams.fieldValue;
	}

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
	WindFSAsset*			mAsset;

	PxMat33			mFieldDirBasis;
	float			mFieldStrength;

	variableOscillator*		mFieldDirectionVO1;
	variableOscillator*		mFieldDirectionVO2;
	variableOscillator*		mFieldStrengthVO;

	float			mStrengthVar;
	PxVec3			mLocalDirVar;

	WindFSParams			mExecuteParams; 

	friend class BasicFSScene;
};

class WindFSActorCPU : public WindFSActorImpl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	WindFSActorCPU(const WindFSActorParams&, WindFSAsset&, ResourceList&, BasicFSScene&);
	~WindFSActorCPU();

	/* FieldSamplerIntl */
	virtual void executeFieldSampler(const ExecuteData& data);

private:
};

#if APEX_CUDA_SUPPORT

class WindFSActorGPU : public WindFSActorCPU
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	WindFSActorGPU(const WindFSActorParams&, WindFSAsset&, ResourceList&, BasicFSScene&);
	~WindFSActorGPU();

	/* FieldSamplerIntl */
	virtual bool updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled);

	virtual void getFieldSamplerCudaExecuteInfo(CudaExecuteInfo& info) const
	{
		info.executeType = 5;
		info.executeParamsHandle = mParamsHandle;
	}

private:
	ApexCudaConstMemGroup			mConstMemGroup;
	InplaceHandle<WindFSParams>		mParamsHandle;

};

#endif

}
} // end namespace nvidia

#endif
