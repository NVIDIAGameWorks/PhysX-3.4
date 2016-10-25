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

#include "VortexFSActorImpl.h"
#include "VortexFSAsset.h"
#include "BasicFSScene.h"
#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "RenderDebugInterface.h"

#include <PxScene.h>

#include "PsMathUtils.h"

#include <FieldSamplerManagerIntl.h>
#include "ApexResourceHelper.h"

namespace nvidia
{
namespace basicfs
{

#define NUM_DEBUG_POINTS 512

VortexFSActorImpl::VortexFSActorImpl(const VortexFSActorParams& params, VortexFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: BasicFSActor(scene)
	, mAsset(&asset)
{
	mFieldWeight = asset.mParams->fieldWeight;

	mPose					= params.initialPose;
	mScale					= params.initialScale;
	mAxis					= mAsset->mParams->axis;
	mBottomSphericalForce   = mAsset->mParams->bottomSphericalForce;
	mTopSphericalForce      = mAsset->mParams->topSphericalForce;
	mHeight					= mAsset->mParams->height;
	mBottomRadius			= mAsset->mParams->bottomRadius;
	mTopRadius				= mAsset->mParams->topRadius;
	mRotationalStrength		= mAsset->mParams->rotationalStrength;
	mRadialStrength			= mAsset->mParams->radialStrength;
	mLiftStrength			= mAsset->mParams->liftStrength;

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
		fieldSamplerDesc.gridSupportType = FieldSamplerGridSupportTypeIntl::VELOCITY_PER_CELL;
		fieldSamplerDesc.samplerFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldSamplerFilterDataName ? params.fieldSamplerFilterDataName : mAsset->mParams->fieldSamplerFilterDataName);
		fieldSamplerDesc.boundaryFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldBoundaryFilterDataName ? params.fieldBoundaryFilterDataName : mAsset->mParams->fieldBoundaryFilterDataName);
		fieldSamplerDesc.boundaryFadePercentage = mAsset->mParams->boundaryFadePercentage;

		fieldSamplerManager->registerFieldSampler(this, fieldSamplerDesc, mScene);
		mFieldSamplerChanged = true;
	}
	mDebugShapeChanged = true;
}

VortexFSActorImpl::~VortexFSActorImpl()
{
}

/* Must be defined inside CPP file, since they require knowledge of asset class */
Asset* 		VortexFSActorImpl::getOwner() const
{
	return static_cast<Asset*>(mAsset);
}

BasicFSAsset* 	VortexFSActorImpl::getVortexFSAsset() const
{
	READ_ZONE();
	return mAsset;
}

void				VortexFSActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	destroy();
} 

void VortexFSActorImpl::destroy()
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

void VortexFSActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}

float VortexFSActorImpl::getActiveLod() const
{
	APEX_INVALID_OPERATION("NxExampleActor does not support this operation");
	return -1.0f;
}

void VortexFSActorImpl::forceLod(float lod)
{
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

// Called by game render thread
void VortexFSActorImpl::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	PX_UNUSED(rewriteBuffers);
	PX_UNUSED(userRenderData);
}

// Called by game render thread
void VortexFSActorImpl::dispatchRenderResources(UserRenderer& renderer)
{
	PX_UNUSED(renderer);
}

bool VortexFSActorImpl::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	isEnabled = mFieldSamplerEnabled;
	if (mFieldSamplerChanged)
	{
		mExecuteParams.bottomSphericalForce		= mBottomSphericalForce;
		mExecuteParams.topSphericalForce		= mTopSphericalForce;

		mExecuteParams.bottomRadius				= mScale * mBottomRadius;
		mExecuteParams.topRadius				= mScale * mTopRadius;
		mExecuteParams.height					= mScale * mHeight;
		mExecuteParams.rotationalStrength		= mScale * mRotationalStrength;
		mExecuteParams.radialStrength			= mScale * mRadialStrength;
		mExecuteParams.liftStrength				= mScale * mLiftStrength;

		const PxVec3 mUnit(0,1,0);
		const PxVec3 vecN = mPose.q.rotate(mAxis);

		const PxVec3 a = mUnit.cross(vecN);
		const PxQuat axisRot(a.x, a.y, a.z, sqrtf(mUnit.magnitudeSquared() * vecN.magnitudeSquared()) + mUnit.dot(vecN));

		mDirToWorld.q = axisRot.getNormalized();
		mDirToWorld.p = mPose.p;
		
		mExecuteParams.worldToDir = mDirToWorld.getInverse();

		shapeDesc.type = FieldShapeTypeIntl::CAPSULE;
		shapeDesc.dimensions = PxVec3(PxMax(mExecuteParams.bottomRadius, mExecuteParams.topRadius), mExecuteParams.height, 0);
		shapeDesc.worldToShape = mExecuteParams.worldToDir;
		shapeDesc.weight = mFieldWeight;

		mFieldSamplerChanged = false;
		return true;
	}
	return false;
}

void VortexFSActorImpl::simulate(float dt)
{
	PX_UNUSED(dt);
}

void VortexFSActorImpl::setRotationalStrength(float strength)
{
	WRITE_ZONE();
	mRotationalStrength = strength;
	mFieldSamplerChanged = true;
}

void VortexFSActorImpl::setRadialStrength(float strength)
{
	WRITE_ZONE();
	mRadialStrength = strength;
	mFieldSamplerChanged = true;
}

void VortexFSActorImpl::setLiftStrength(float strength)
{
	WRITE_ZONE();
	mLiftStrength = strength;
	mFieldSamplerChanged = true;
}

void VortexFSActorImpl::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if ( !mEnableDebugVisualization ) return;
	RenderDebugInterface* debugRender = mScene->mDebugRender;
	BasicFSDebugRenderParams* debugRenderParams = mScene->mBasicFSDebugRenderParams;

	using RENDER_DEBUG::DebugColors;

	if (!debugRenderParams->VISUALIZE_VORTEX_FS_ACTOR)
	{
		return;
	}

	const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(debugRender)->getPoseTyped();
	RENDER_DEBUG_IFACE(debugRender)->setIdentityPose();

	if (debugRenderParams->VISUALIZE_VORTEX_FS_ACTOR_NAME)
	{
		char buf[128];
		buf[sizeof(buf) - 1] = 0;
		APEX_SPRINTF_S(buf, sizeof(buf) - 1, " %s %s", mAsset->getObjTypeName(), mAsset->getName());

		PxVec3 textLocation = mPose.p;

		RENDER_DEBUG_IFACE(debugRender)->setCurrentTextScale(4.0f);
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::Blue));
		RENDER_DEBUG_IFACE(debugRender)->debugText(textLocation, buf);
	}

	if (debugRenderParams->VISUALIZE_VORTEX_FS_SHAPE)
	{
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::Blue));
		RENDER_DEBUG_IFACE(debugRender)->setPose(mDirToWorld);
		RENDER_DEBUG_IFACE(debugRender)->debugCapsuleTapered(mExecuteParams.topRadius, mExecuteParams.bottomRadius, mExecuteParams.height, 2);
		RENDER_DEBUG_IFACE(debugRender)->setPose(physx::PxMat44(physx::PxIdentity));
	}

	if (debugRenderParams->VISUALIZE_VORTEX_FS_POSE)
	{
		RENDER_DEBUG_IFACE(debugRender)->debugAxes(PxMat44(mPose), 1);
	}

	if (debugRenderParams->VISUALIZE_VORTEX_FS_FIELD)
	{
		if (mDebugShapeChanged || mDebugPoints.empty())
		{
			mDebugShapeChanged = false;
			mDebugPoints.resize(NUM_DEBUG_POINTS);
			for (uint32_t i = 0; i < NUM_DEBUG_POINTS; ++i)
			{
				float r1 = mBottomRadius;
				float r2 = mTopRadius;
				float h = mHeight;
				float maxR = PxMax(r1, r2);
				float rx, ry, rz;
				bool isInside = false;
				do
				{
					rx = physx::shdfnd::rand(-maxR, maxR);
					ry = physx::shdfnd::rand(-h/2 - r1, h/2 + r2);
					rz = physx::shdfnd::rand(-maxR, maxR);
					isInside = 2*ry <= h && -h <= 2*ry &&
						rx*rx + rz*rz <= physx::shdfnd::sqr(r1 + (ry / h + 0.5) * (r2-r1));
					isInside |= 2*ry < -h && rx*rx + rz*rz <= r1*r1 - (2*ry+h)*(2*ry+h)*0.25;
					isInside |= 2*ry > h && rx*rx + rz*rz <= r2*r2 - (2*ry-h)*(2*ry-h)*0.25;
				}
				while (!isInside);

				PxVec3& vec = mDebugPoints[i];

				// we need transform from local to world
				vec.x = rx;
				vec.y = ry;
				vec.z = rz;
			}
		}

		using RENDER_DEBUG::DebugColors;

		uint32_t c1 = RENDER_DEBUG_IFACE(mScene->mDebugRender)->getDebugColor(DebugColors::Blue);
		uint32_t c2 = RENDER_DEBUG_IFACE(mScene->mDebugRender)->getDebugColor(DebugColors::Red);

		for (uint32_t i = 0; i < NUM_DEBUG_POINTS; ++i)
		{
			PxVec3 localPos = mScale * mDebugPoints[i];
			PxVec3 pos = mDirToWorld.transform(localPos);
			PxVec3 fieldVec = executeVortexFS(mExecuteParams, pos/*, totalElapsedMS*/);
			RENDER_DEBUG_IFACE(debugRender)->debugGradientLine(pos, pos + fieldVec, c1, c2);
		}
	}
	RENDER_DEBUG_IFACE(debugRender)->setPose(savedPose);
#endif
}

/******************************** CPU Version ********************************/

VortexFSActorCPU::VortexFSActorCPU(const VortexFSActorParams& params, VortexFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: VortexFSActorImpl(params, asset, list, scene)
{
}

VortexFSActorCPU::~VortexFSActorCPU()
{
}

void VortexFSActorCPU::executeFieldSampler(const ExecuteData& data)
{
	for (uint32_t iter = 0; iter < data.count; ++iter)
	{
		uint32_t i = data.indices[iter & data.indicesMask] + (iter & ~data.indicesMask);
		PxVec3* pos = (PxVec3*)((uint8_t*)data.position + i * data.positionStride);
		data.resultField[iter] = executeVortexFS(mExecuteParams, *pos/*, totalElapsedMS*/);
	}
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT


VortexFSActorGPU::VortexFSActorGPU(const VortexFSActorParams& params, VortexFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: VortexFSActorCPU(params, asset, list, scene)
	, mConstMemGroup(CUDA_OBJ(fieldSamplerStorage))
{
}

VortexFSActorGPU::~VortexFSActorGPU()
{
}

bool VortexFSActorGPU::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	if (VortexFSActorImpl::updateFieldSampler(shapeDesc, isEnabled))
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

