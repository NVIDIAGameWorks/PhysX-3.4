/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "RenderMeshActorDesc.h"
#include "RenderMeshActor.h"
#include "RenderMeshAsset.h"
#include "Apex.h"
#include "WindFSActorImpl.h"
#include "WindFSAsset.h"
#include "BasicFSScene.h"
#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "RenderDebugInterface.h"

#include <PxScene.h>

#include <FieldSamplerManagerIntl.h>
#include "ApexResourceHelper.h"

#include "PsMathUtils.h"

namespace nvidia
{
namespace basicfs
{

WindFSActorImpl::WindFSActorImpl(const WindFSActorParams& params, WindFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: BasicFSActor(scene)
	, mAsset(&asset)
	, mFieldDirectionVO1(NULL)
	, mFieldDirectionVO2(NULL)
	, mFieldStrengthVO(NULL)
{
	mFieldWeight = asset.mParams->fieldWeight;

	mPose = params.initialPose;
	mScale = params.initialScale;
	setFieldDirection(mAsset->mParams->fieldDirection);
	setFieldStrength(mAsset->mParams->fieldStrength);

	mExecuteParams.fieldValue = getFieldDirection() * (getFieldStrength() * mScale);

	mStrengthVar = 0.0f;
	mLocalDirVar = PxVec3(1, 0, 0);

	if (mAsset->mParams->fieldStrengthDeviationPercentage > 0 && mAsset->mParams->fieldStrengthOscillationPeriod > 0)
	{
		mFieldStrengthVO = PX_NEW(variableOscillator)(-mAsset->mParams->fieldStrengthDeviationPercentage,
													   +mAsset->mParams->fieldStrengthDeviationPercentage,
													   0.0f,
													   mAsset->mParams->fieldStrengthOscillationPeriod);
	}

	float diviationAngle = physx::shdfnd::degToRad(mAsset->mParams->fieldDirectionDeviationAngle);
	if (diviationAngle > 0 && mAsset->mParams->fieldDirectionOscillationPeriod > 0)
	{
		mFieldDirectionVO1 = PX_NEW(variableOscillator)(-diviationAngle,
														 +diviationAngle,
														 0,
														 mAsset->mParams->fieldDirectionOscillationPeriod);

		mFieldDirectionVO2 = PX_NEW(variableOscillator)(-PxTwoPi,
														 +PxTwoPi,
														 0,
														 mAsset->mParams->fieldDirectionOscillationPeriod);
	}

	list.add(*this);			// Add self to asset's list of actors
	addSelfToContext(*scene.getApexScene().getApexContext());    // Add self to ApexScene
	addSelfToContext(scene);	// Add self to BasicFSScene's list of actors

	FieldSamplerManagerIntl* fieldSamplerManager = mScene->getInternalFieldSamplerManager();
	if (fieldSamplerManager != 0)
	{
		FieldSamplerDescIntl fieldSamplerDesc;
		if (asset.mParams->fieldDragCoeff > 0)
		{
			fieldSamplerDesc.type = FieldSamplerTypeIntl::VELOCITY_DRAG;
			fieldSamplerDesc.dragCoeff = asset.mParams->fieldDragCoeff;
		}
		else
		{
			fieldSamplerDesc.type = FieldSamplerTypeIntl::VELOCITY_DIRECT;
		}
		fieldSamplerDesc.gridSupportType = FieldSamplerGridSupportTypeIntl::SINGLE_VELOCITY;
		fieldSamplerDesc.samplerFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldSamplerFilterDataName ? params.fieldSamplerFilterDataName : mAsset->mParams->fieldSamplerFilterDataName);
		fieldSamplerDesc.boundaryFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldBoundaryFilterDataName ? params.fieldBoundaryFilterDataName : mAsset->mParams->fieldBoundaryFilterDataName);
		fieldSamplerDesc.boundaryFadePercentage = 0;

		fieldSamplerManager->registerFieldSampler(this, fieldSamplerDesc, mScene);
		mFieldSamplerChanged = true;
	}
}

WindFSActorImpl::~WindFSActorImpl()
{
}

/* Must be defined inside CPP file, since they require knowledge of asset class */
Asset* 		WindFSActorImpl::getOwner() const
{
	return static_cast<Asset*>(mAsset);
}

BasicFSAsset* 	WindFSActorImpl::getWindFSAsset() const
{
	READ_ZONE();
	return mAsset;
}

void				WindFSActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	destroy();
} 

void WindFSActorImpl::destroy()
{
	{
		WRITE_ZONE();
		ApexActor::destroy();

		setPhysXScene(NULL);

		FieldSamplerManagerIntl* fieldSamplerManager = mScene->getInternalFieldSamplerManager();
		if (fieldSamplerManager != 0)
		{
			fieldSamplerManager->unregisterFieldSampler(this);
		}

		if (mFieldStrengthVO)
		{
			PX_DELETE_AND_RESET(mFieldStrengthVO);
		}
		if (mFieldDirectionVO1)
		{
			PX_DELETE_AND_RESET(mFieldDirectionVO1);
		}
		if (mFieldDirectionVO2)
		{
			PX_DELETE_AND_RESET(mFieldDirectionVO2);
		}
	}
	delete this;
}

void WindFSActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}

float WindFSActorImpl::getActiveLod() const
{
	APEX_INVALID_OPERATION("NxExampleActor does not support this operation");
	return -1.0f;
}

void WindFSActorImpl::forceLod(float lod)
{
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

// Called by game render thread
void WindFSActorImpl::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	PX_UNUSED(rewriteBuffers);
	PX_UNUSED(userRenderData);
}

// Called by game render thread
void WindFSActorImpl::dispatchRenderResources(UserRenderer& renderer)
{
	PX_UNUSED(renderer);
}

bool WindFSActorImpl::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	PX_UNUSED(shapeDesc);

	isEnabled = mFieldSamplerEnabled;
	if (mFieldSamplerChanged)
	{
		PxVec3 instDirection = mFieldDirBasis.transform(mLocalDirVar);
		float instStrength = mScale * mFieldStrength * (1.0f + mStrengthVar);

		mExecuteParams.fieldValue = instDirection * instStrength;

		shapeDesc.type = FieldShapeTypeIntl::NONE;
		shapeDesc.worldToShape = PxTransform(PxIdentity);
		shapeDesc.dimensions = PxVec3(0.0f);
		shapeDesc.weight = mFieldWeight;

		mFieldSamplerChanged = false;
		return true;
	}
	return false;
}

void WindFSActorImpl::simulate(float dt)
{
	if (mFieldStrengthVO != NULL)
	{
		mStrengthVar = mFieldStrengthVO->updateVariableOscillator(dt);

		mFieldSamplerChanged = true;
	}
	if (mFieldDirectionVO1 != NULL && mFieldDirectionVO2 != NULL)
	{
		float theta = mFieldDirectionVO1->updateVariableOscillator(dt);
		float phi = mFieldDirectionVO2->updateVariableOscillator(dt);

		mLocalDirVar.x = PxCos(theta);
		mLocalDirVar.y = PxSin(theta) * PxCos(phi);
		mLocalDirVar.z = PxSin(theta) * PxSin(phi);

		mFieldSamplerChanged = true;
	}
}

void WindFSActorImpl::setFieldStrength(float strength)
{
	WRITE_ZONE();
	mFieldStrength = strength;
	mFieldSamplerChanged = true;
}

void WindFSActorImpl::setFieldDirection(const PxVec3& direction)
{
	WRITE_ZONE();
	mFieldDirBasis.column0 = direction.getNormalized();
	BuildPlaneBasis(mFieldDirBasis.column0, mFieldDirBasis.column1, mFieldDirBasis.column2);

	mFieldSamplerChanged = true;
}

void WindFSActorImpl::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if ( !mEnableDebugVisualization ) return;
	RenderDebugInterface* debugRender = mScene->mDebugRender;
	BasicFSDebugRenderParams* debugRenderParams = mScene->mBasicFSDebugRenderParams;

	if (!debugRenderParams->VISUALIZE_WIND_FS_ACTOR)
	{
		return;
	}

	const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(debugRender)->getPoseTyped();
	RENDER_DEBUG_IFACE(debugRender)->setIdentityPose();
	if (debugRenderParams->VISUALIZE_WIND_FS_ACTOR_NAME)
	{
		char buf[128];
		buf[sizeof(buf) - 1] = 0;
		APEX_SPRINTF_S(buf, sizeof(buf) - 1, " %s %s", mAsset->getObjTypeName(), mAsset->getName());

		PxVec3 textLocation = mPose.p;

		RENDER_DEBUG_IFACE(debugRender)->setCurrentTextScale(4.0f);
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(RENDER_DEBUG::DebugColors::Blue));
		RENDER_DEBUG_IFACE(debugRender)->debugText(textLocation, buf);
	}
	RENDER_DEBUG_IFACE(debugRender)->setPose(savedPose);
#endif
}

/******************************** CPU Version ********************************/

WindFSActorCPU::WindFSActorCPU(const WindFSActorParams& params, WindFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: WindFSActorImpl(params, asset, list, scene)
{
}

WindFSActorCPU::~WindFSActorCPU()
{
}

void WindFSActorCPU::executeFieldSampler(const ExecuteData& data)
{
	for (uint32_t iter = 0; iter < data.count; ++iter)
	{
		uint32_t i = data.indices[iter & data.indicesMask] + (iter & ~data.indicesMask);
		PxVec3* pos = (PxVec3*)((uint8_t*)data.position + i * data.positionStride);
		data.resultField[iter] = executeWindFS(mExecuteParams, *pos);
	}
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT


WindFSActorGPU::WindFSActorGPU(const WindFSActorParams& params, WindFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: WindFSActorCPU(params, asset, list, scene)
	, mConstMemGroup(CUDA_OBJ(fieldSamplerStorage))
{
}

WindFSActorGPU::~WindFSActorGPU()
{
}

bool WindFSActorGPU::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	if (WindFSActorImpl::updateFieldSampler(shapeDesc, isEnabled))
	{
		APEX_CUDA_CONST_MEM_GROUP_SCOPE(mConstMemGroup);

		if (mParamsHandle.isNull())
		{
			mParamsHandle.alloc(_storage_);
		}
		mParamsHandle.update(_storage_, mExecuteParams);
		return true;
	}
	return false;
}


#endif

}
} // end namespace nvidia::apex

