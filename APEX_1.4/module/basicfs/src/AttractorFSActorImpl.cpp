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

#include "AttractorFSActorImpl.h"
#include "AttractorFSAsset.h"
#include "BasicFSScene.h"
#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "RenderDebugInterface.h"

#include <PxScene.h>

#include <FieldSamplerManagerIntl.h>
#include "ApexResourceHelper.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

#include "PsMathUtils.h"

namespace nvidia
{
namespace basicfs
{

#define NUM_DEBUG_POINTS 512

AttractorFSActorImpl::AttractorFSActorImpl(const AttractorFSActorParams& params, AttractorFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: BasicFSActor(scene)
	, mAsset(&asset)
{
	WRITE_ZONE();
	mFieldWeight = asset.mParams->fieldWeight;

	mPose					= params.initialPose;
	mScale					= params.initialScale;
	mRadius					= mAsset->mParams->radius;
	mConstFieldStrength		= mAsset->mParams->constFieldStrength;
	mVariableFieldStrength	= mAsset->mParams->variableFieldStrength;

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
		fieldSamplerDesc.gridSupportType = FieldSamplerGridSupportTypeIntl::NONE;
		fieldSamplerDesc.samplerFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldSamplerFilterDataName ? params.fieldSamplerFilterDataName : mAsset->mParams->fieldSamplerFilterDataName);
		fieldSamplerDesc.boundaryFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldBoundaryFilterDataName ? params.fieldBoundaryFilterDataName : mAsset->mParams->fieldBoundaryFilterDataName);
		fieldSamplerDesc.boundaryFadePercentage = mAsset->mParams->boundaryFadePercentage;

		fieldSamplerManager->registerFieldSampler(this, fieldSamplerDesc, mScene);
		mFieldSamplerChanged = true;
	}
}

AttractorFSActorImpl::~AttractorFSActorImpl()
{
}

/* Must be defined inside CPP file, since they require knowledge of asset class */
Asset* 		AttractorFSActorImpl::getOwner() const
{
	READ_ZONE();
	return static_cast<Asset*>(mAsset);
}

BasicFSAsset* 	AttractorFSActorImpl::getAttractorFSAsset() const
{
	READ_ZONE();
	return mAsset;
}

void				AttractorFSActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	destroy();
} 

void AttractorFSActorImpl::destroy()
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

void AttractorFSActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	READ_ZONE();
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}

float AttractorFSActorImpl::getActiveLod() const
{
	READ_ZONE();
	APEX_INVALID_OPERATION("NxExampleActor does not support this operation");
	return -1.0f;
}

void AttractorFSActorImpl::forceLod(float lod)
{
	WRITE_ZONE();
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

// Called by game render thread
void AttractorFSActorImpl::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	WRITE_ZONE();
	PX_UNUSED(rewriteBuffers);
	PX_UNUSED(userRenderData);
}

// Called by game render thread
void AttractorFSActorImpl::dispatchRenderResources(UserRenderer& renderer)
{
	READ_ZONE();
	PX_UNUSED(renderer);
}

bool AttractorFSActorImpl::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	WRITE_ZONE();
	isEnabled = mFieldSamplerEnabled;
	if (mFieldSamplerChanged)
	{
		mExecuteParams.origin					= mPose.p;
		mExecuteParams.radius					= mRadius * mScale;
		mExecuteParams.constFieldStrength		= mConstFieldStrength * mScale;
		mExecuteParams.variableFieldStrength	= mVariableFieldStrength * mScale;

		shapeDesc.type = FieldShapeTypeIntl::SPHERE;
		shapeDesc.worldToShape.q = PxIdentity;
		shapeDesc.worldToShape.p = -mExecuteParams.origin;
		shapeDesc.dimensions = PxVec3(mExecuteParams.radius, 0, 0);
		shapeDesc.weight = mFieldWeight;

		mFieldSamplerChanged					= false;
		return true;
	}
	return false;
}

void AttractorFSActorImpl::simulate(float dt)
{
	WRITE_ZONE();
	PX_UNUSED(dt);
}

void AttractorFSActorImpl::setConstFieldStrength(float strength)
{
	WRITE_ZONE();
	mConstFieldStrength = strength;
	mFieldSamplerChanged = true;
}

void AttractorFSActorImpl::setVariableFieldStrength(float strength)
{
	WRITE_ZONE();
	mVariableFieldStrength = strength;
	mFieldSamplerChanged = true;
}

void AttractorFSActorImpl::visualize()
{
	WRITE_ZONE();
#ifndef WITHOUT_DEBUG_VISUALIZE
	if ( !mEnableDebugVisualization ) return;
	RenderDebugInterface* debugRender = mScene->mDebugRender;
	BasicFSDebugRenderParams* debugRenderParams = mScene->mBasicFSDebugRenderParams;
	const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(debugRender)->getPoseTyped();
	RENDER_DEBUG_IFACE(debugRender)->setIdentityPose();

	if (!debugRenderParams->VISUALIZE_ATTRACTOR_FS_ACTOR)
	{
		return;
	}

	if (debugRenderParams->VISUALIZE_ATTRACTOR_FS_ACTOR_NAME)
	{
		char buf[128];
		buf[sizeof(buf) - 1] = 0;
		APEX_SPRINTF_S(buf, sizeof(buf) - 1, " %s %s", mAsset->getObjTypeName(), mAsset->getName());

		PxVec3 textLocation = mPose.p;

		RENDER_DEBUG_IFACE(debugRender)->setCurrentTextScale(4.0f);
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(RENDER_DEBUG::DebugColors::Blue));
		RENDER_DEBUG_IFACE(debugRender)->debugText(textLocation, buf);
	}

	if (debugRenderParams->VISUALIZE_ATTRACTOR_FS_SHAPE)
	{
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(RENDER_DEBUG::DebugColors::Blue));
//		RENDER_DEBUG_IFACE(debugRender)->debugOrientedSphere(mExecuteParams.radius, 2, mDirToWorld);
		RENDER_DEBUG_IFACE(debugRender)->debugSphere(mExecuteParams.origin, mExecuteParams.radius);
	}

	if (debugRenderParams->VISUALIZE_ATTRACTOR_FS_POSE)
	{
		RENDER_DEBUG_IFACE(debugRender)->debugAxes(PxMat44(mPose), 1);
	}

	if (debugRenderParams->VISUALIZE_ATTRACTOR_FS_FIELD)
	{
		if (mDebugPoints.empty())
		{
			mDebugPoints.resize(NUM_DEBUG_POINTS);
			for (uint32_t i = 0; i < NUM_DEBUG_POINTS; ++i)
			{
				float rx, ry, rz;
				do
				{
					rx = physx::shdfnd::rand(-1.0f, +1.0f);
					ry = physx::shdfnd::rand(-1.0f, +1.0f);
					rz = physx::shdfnd::rand(-1.0f, +1.0f);
				}
				while (rx * rx + ry * ry + rz * rz > 1.0f);

				PxVec3& vec = mDebugPoints[i];

				vec.x = rx;
				vec.y = ry;
				vec.z = rz;
			}
		}

		uint32_t c1 = RENDER_DEBUG_IFACE(mScene->mDebugRender)->getDebugColor(RENDER_DEBUG::DebugColors::Blue);
		uint32_t c2 = RENDER_DEBUG_IFACE(mScene->mDebugRender)->getDebugColor(RENDER_DEBUG::DebugColors::Red);

		for (uint32_t i = 0; i < NUM_DEBUG_POINTS; ++i)
		{
			PxVec3 pos = mExecuteParams.origin + (mDebugPoints[i] * mExecuteParams.radius);
			PxVec3 fieldVec = executeAttractorFS(mExecuteParams, pos/*, totalElapsedMS*/);
			RENDER_DEBUG_IFACE(debugRender)->debugGradientLine(pos, pos + fieldVec, c1, c2);
		}
	}
	RENDER_DEBUG_IFACE(debugRender)->setPose(savedPose);
#endif
}

/******************************** CPU Version ********************************/

AttractorFSActorCPU::AttractorFSActorCPU(const AttractorFSActorParams& params, AttractorFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: AttractorFSActorImpl(params, asset, list, scene)
{
}

AttractorFSActorCPU::~AttractorFSActorCPU()
{
}

void AttractorFSActorCPU::executeFieldSampler(const ExecuteData& data)
{
	WRITE_ZONE();
	for (uint32_t iter = 0; iter < data.count; ++iter)
	{
		uint32_t i = data.indices[iter & data.indicesMask] + (iter & ~data.indicesMask);
		PxVec3* pos = (PxVec3*)((uint8_t*)data.position + i * data.positionStride);
		data.resultField[iter] = executeAttractorFS(mExecuteParams, *pos/*, totalElapsedMS*/);
	}
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT


AttractorFSActorGPU::AttractorFSActorGPU(const AttractorFSActorParams& params, AttractorFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: AttractorFSActorCPU(params, asset, list, scene)
	, mConstMemGroup(CUDA_OBJ(fieldSamplerStorage))
{
}

AttractorFSActorGPU::~AttractorFSActorGPU()
{
}

bool AttractorFSActorGPU::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	WRITE_ZONE();
	if (AttractorFSActorImpl::updateFieldSampler(shapeDesc, isEnabled))
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

