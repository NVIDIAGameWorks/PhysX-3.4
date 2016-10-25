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
#include "NoiseFSActorImpl.h"
#include "NoiseFSAsset.h"
#include "BasicFSScene.h"
#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "RenderDebugInterface.h"

#include <PxScene.h>

#include <FieldSamplerManagerIntl.h>
#include "ApexResourceHelper.h"

namespace nvidia
{
namespace basicfs
{

NoiseFSActorImpl::NoiseFSActorImpl(const NoiseFSActorParams& params, NoiseFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: BasicFSActor(scene)
	, mAsset(&asset)
{
	mFieldWeight = asset.mParams->fieldWeight;

	mPose = params.initialPose;
	mScale = params.initialScale * asset.mParams->defaultScale;

	mExecuteParams.useLocalSpace = mAsset->mParams->useLocalSpace;

	mExecuteParams.noiseTimeFreq = 1.0f / mAsset->mParams->noiseTimePeriod;
	mExecuteParams.noiseOctaves = mAsset->mParams->noiseOctaves;
	mExecuteParams.noiseStrengthOctaveMultiplier = mAsset->mParams->noiseStrengthOctaveMultiplier;
	mExecuteParams.noiseSpaceFreqOctaveMultiplier = PxVec3(1.0f / mAsset->mParams->noiseSpacePeriodOctaveMultiplier.x, 1.0f / mAsset->mParams->noiseSpacePeriodOctaveMultiplier.y, 1.0f / mAsset->mParams->noiseSpacePeriodOctaveMultiplier.z);
	mExecuteParams.noiseTimeFreqOctaveMultiplier = 1.0f / mAsset->mParams->noiseTimePeriodOctaveMultiplier;

	if (nvidia::strcmp(mAsset->mParams->noiseType, "CURL") == 0)
	{
		mExecuteParams.noiseType = NoiseType::CURL;
	}
	else
	{
		mExecuteParams.noiseType = NoiseType::SIMPLEX;
	}
	mExecuteParams.noiseSeed = mAsset->mParams->noiseSeed;

	list.add(*this);			// Add self to asset's list of actors
	addSelfToContext(*scene.getApexScene().getApexContext());    // Add self to ApexScene
	addSelfToContext(scene);	// Add self to BasicFSScene's list of actors

	FieldSamplerManagerIntl* fieldSamplerManager = mScene->getInternalFieldSamplerManager();
	if (fieldSamplerManager != 0)
	{
		FieldSamplerDescIntl fieldSamplerDesc;

		fieldSamplerDesc.gridSupportType = FieldSamplerGridSupportTypeIntl::VELOCITY_PER_CELL;
		if (nvidia::strcmp(mAsset->mParams->fieldType, "FORCE") == 0)
		{
			fieldSamplerDesc.type = FieldSamplerTypeIntl::FORCE;
			fieldSamplerDesc.gridSupportType = FieldSamplerGridSupportTypeIntl::NONE;
		}
		else if (nvidia::strcmp(mAsset->mParams->fieldType, "VELOCITY_DRAG") == 0)
		{
			fieldSamplerDesc.type = FieldSamplerTypeIntl::VELOCITY_DRAG;
			fieldSamplerDesc.dragCoeff = mAsset->mParams->fieldDragCoeff;
		}
		else
		{
			fieldSamplerDesc.type = FieldSamplerTypeIntl::VELOCITY_DIRECT;
		}
		fieldSamplerDesc.samplerFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldSamplerFilterDataName ? params.fieldSamplerFilterDataName : mAsset->mParams->fieldSamplerFilterDataName);
		fieldSamplerDesc.boundaryFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldBoundaryFilterDataName ? params.fieldBoundaryFilterDataName : mAsset->mParams->fieldBoundaryFilterDataName);
		fieldSamplerDesc.boundaryFadePercentage = mAsset->mParams->boundaryFadePercentage;

		fieldSamplerManager->registerFieldSampler(this, fieldSamplerDesc, mScene);
		mFieldSamplerChanged = true;
	}
}

NoiseFSActorImpl::~NoiseFSActorImpl()
{
}

/* Must be defined inside CPP file, since they require knowledge of asset class */
Asset* 		NoiseFSActorImpl::getOwner() const
{
	return static_cast<Asset*>(mAsset);
}

BasicFSAsset* 	NoiseFSActorImpl::getNoiseFSAsset() const
{
	READ_ZONE();
	return mAsset;
}

void				NoiseFSActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	destroy();
} 

void NoiseFSActorImpl::destroy()
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
	}

	delete this;
}

void NoiseFSActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}

float NoiseFSActorImpl::getActiveLod() const
{
	APEX_INVALID_OPERATION("NxExampleActor does not support this operation");
	return -1.0f;
}

void NoiseFSActorImpl::forceLod(float lod)
{
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

// Called by game render thread
void NoiseFSActorImpl::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	PX_UNUSED(rewriteBuffers);
	PX_UNUSED(userRenderData);
}

// Called by game render thread
void NoiseFSActorImpl::dispatchRenderResources(UserRenderer& renderer)
{
	PX_UNUSED(renderer);
}

bool NoiseFSActorImpl::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	PX_UNUSED(shapeDesc);

	isEnabled = mFieldSamplerEnabled;
	if (mFieldSamplerChanged)
	{
		mExecuteParams.worldToShape = mPose.getInverse();

		PxVec3 noiseSpacePeriod = mAsset->mParams->noiseSpacePeriod * mScale;
		mExecuteParams.noiseSpaceFreq = PxVec3(1.0f / noiseSpacePeriod.x, 1.0f / noiseSpacePeriod.y, 1.0f / noiseSpacePeriod.z);
		mExecuteParams.noiseStrength = mAsset->mParams->noiseStrength * mScale;

		shapeDesc.type = FieldShapeTypeIntl::BOX;
		shapeDesc.worldToShape = mExecuteParams.worldToShape;
		shapeDesc.dimensions = mAsset->mParams->boundarySize * (mScale * 0.5f);
		shapeDesc.weight = mFieldWeight;

		mFieldSamplerChanged = false;
		return true;
	}
	return false;
}

void NoiseFSActorImpl::simulate(float )
{
}

void NoiseFSActorImpl::setNoiseStrength(float strength)
{
	WRITE_ZONE();
	mExecuteParams.noiseStrength = strength;
	mFieldSamplerChanged = true;
}

void NoiseFSActorImpl::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if ( !mEnableDebugVisualization ) return;
	RenderDebugInterface* debugRender = mScene->mDebugRender;
	BasicFSDebugRenderParams* debugRenderParams = mScene->mBasicFSDebugRenderParams;

	if (!debugRenderParams->VISUALIZE_NOISE_FS_ACTOR)
	{
		return;
	}

	const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(debugRender)->getPoseTyped();
	RENDER_DEBUG_IFACE(debugRender)->setIdentityPose();

	using RENDER_DEBUG::DebugColors;

	if (debugRenderParams->VISUALIZE_NOISE_FS_ACTOR_NAME)
	{
		char buf[128];
		buf[sizeof(buf) - 1] = 0;
		APEX_SPRINTF_S(buf, sizeof(buf) - 1, " %s %s", mAsset->getObjTypeName(), mAsset->getName());

		PxMat44 cameraFacingPose(mScene->mApexScene->getViewMatrix(0).inverseRT());
		PxVec3 textLocation = mPose.p;

		RENDER_DEBUG_IFACE(debugRender)->setCurrentTextScale(4.0f);
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::Blue));
		RENDER_DEBUG_IFACE(debugRender)->debugText(textLocation, buf);
	}

	if (debugRenderParams->VISUALIZE_NOISE_FS_SHAPE)
	{
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::Blue));

		PxVec3 shapeSides = mScale * mAsset->mParams->boundarySize;
		RENDER_DEBUG_IFACE(debugRender)->setPose(mPose);
		RENDER_DEBUG_IFACE(debugRender)->debugBound( PxBounds3(PxVec3(0.0f), shapeSides) );
	}
	if (debugRenderParams->VISUALIZE_NOISE_FS_POSE)
	{
		RENDER_DEBUG_IFACE(debugRender)->debugAxes(PxMat44(mPose), 1);
	}
	RENDER_DEBUG_IFACE(debugRender)->setPose(savedPose);
#endif
}

/******************************** CPU Version ********************************/

NoiseFSActorCPU::NoiseFSActorCPU(const NoiseFSActorParams& params, NoiseFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: NoiseFSActorImpl(params, asset, list, scene)
{
}

NoiseFSActorCPU::~NoiseFSActorCPU()
{
}

void NoiseFSActorCPU::executeFieldSampler(const ExecuteData& data)
{
	uint32_t totalElapsedMS = mScene->getApexScene().getTotalElapsedMS();
	for (uint32_t iter = 0; iter < data.count; ++iter)
	{
		uint32_t i = data.indices[iter & data.indicesMask] + (iter & ~data.indicesMask);
		PxVec3* pos = (PxVec3*)((uint8_t*)data.position + i * data.positionStride);
		data.resultField[iter] = executeNoiseFS(mExecuteParams, *pos, totalElapsedMS);
	}
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT


NoiseFSActorGPU::NoiseFSActorGPU(const NoiseFSActorParams& params, NoiseFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: NoiseFSActorCPU(params, asset, list, scene)
	, mConstMemGroup(CUDA_OBJ(fieldSamplerStorage))
{
}

NoiseFSActorGPU::~NoiseFSActorGPU()
{
}

bool NoiseFSActorGPU::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	if (NoiseFSActorImpl::updateFieldSampler(shapeDesc, isEnabled))
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

