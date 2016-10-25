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
#include "JetFSActorImpl.h"
#include "JetFSAsset.h"
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

#define NUM_DEBUG_POINTS 2048


JetFSActorImpl::JetFSActorImpl(const JetFSActorParams& params, JetFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: BasicFSActor(scene)
	, mAsset(&asset)
	, mFieldDirectionVO1(NULL)
	, mFieldDirectionVO2(NULL)
	, mFieldStrengthVO(NULL)
{
	WRITE_ZONE();
	mFieldWeight = asset.mParams->fieldWeight;

	mPose = params.initialPose;
	mScale = params.initialScale * asset.mParams->defaultScale;

	mFieldDirection = mAsset->mParams->fieldDirection.getNormalized();
	mFieldStrength = mAsset->mParams->fieldStrength;

	mStrengthVar = 0.0f;
	mLocalDirVar = PxVec3(0, 1, 0);

	mExecuteParams.noiseTimeScale = mAsset->mParams->noiseTimeScale;
	mExecuteParams.noiseOctaves = mAsset->mParams->noiseOctaves;

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
		fieldSamplerDesc.gridSupportType = FieldSamplerGridSupportTypeIntl::VELOCITY_PER_CELL;
		fieldSamplerDesc.samplerFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldSamplerFilterDataName ? params.fieldSamplerFilterDataName : mAsset->mParams->fieldSamplerFilterDataName);
		fieldSamplerDesc.boundaryFilterData = ApexResourceHelper::resolveCollisionGroup128(params.fieldBoundaryFilterDataName ? params.fieldBoundaryFilterDataName : mAsset->mParams->fieldBoundaryFilterDataName);
		fieldSamplerDesc.boundaryFadePercentage = mAsset->mParams->boundaryFadePercentage;

		fieldSamplerManager->registerFieldSampler(this, fieldSamplerDesc, mScene);
		mFieldSamplerChanged = true;
	}
}

JetFSActorImpl::~JetFSActorImpl()
{
}

/* Must be defined inside CPP file, since they require knowledge of asset class */
Asset* 		JetFSActorImpl::getOwner() const
{
	READ_ZONE();
	return static_cast<Asset*>(mAsset);
}

BasicFSAsset* 	JetFSActorImpl::getJetFSAsset() const
{
	READ_ZONE();
	return mAsset;
}

void				JetFSActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	destroy();
} 

void JetFSActorImpl::destroy()
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

void JetFSActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	READ_ZONE();
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}

float JetFSActorImpl::getActiveLod() const
{
	READ_ZONE();
	APEX_INVALID_OPERATION("NxExampleActor does not support this operation");
	return -1.0f;
}

void JetFSActorImpl::forceLod(float lod)
{
	WRITE_ZONE();
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

// Called by game render thread
void JetFSActorImpl::updateRenderResources(bool rewriteBuffers, void* userRenderData)
{
	WRITE_ZONE();
	PX_UNUSED(rewriteBuffers);
	PX_UNUSED(userRenderData);
}

// Called by game render thread
void JetFSActorImpl::dispatchRenderResources(UserRenderer& renderer)
{
	WRITE_ZONE();
	PX_UNUSED(renderer);
}

bool JetFSActorImpl::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	WRITE_ZONE();
	isEnabled = mFieldSamplerEnabled;
	if (mFieldSamplerChanged)
	{
		mExecuteParams.nearRadius = mAsset->mParams->nearRadius * mScale;
		mExecuteParams.pivotRadius = mAsset->mParams->pivotRadius * mScale;
		mExecuteParams.farRadius = mAsset->mParams->farRadius * mScale;
		mExecuteParams.directionalStretch = mAsset->mParams->directionalStretch;
		mExecuteParams.averageStartDistance = mAsset->mParams->averageStartDistance * mScale;
		mExecuteParams.averageEndDistance = mAsset->mParams->averageEndDistance * mScale;

		mExecuteParams.pivotRatio = (mExecuteParams.farRadius - mExecuteParams.pivotRadius) / (mExecuteParams.pivotRadius - mExecuteParams.nearRadius);

		const PxVec3 mUnit(0,1,0);
		{
			if ((mUnit + mFieldDirection).isZero())
			{
				mDirToWorld.q = PxQuat(physx::PxPi, PxVec3(1,0,0));
				mDirToWorld.p = mPose.p;
			}
			else
			{
				const PxVec3 vecN = mPose.q.rotate(mFieldDirection);

				const PxVec3 a = mUnit.cross(vecN);
				PxQuat axisRot(a.x, a.y, a.z, sqrtf(mUnit.magnitudeSquared() * vecN.magnitudeSquared()) + mUnit.dot(vecN));
				if (axisRot.w < FLT_EPSILON)
				{
					axisRot.w = 1.0f;
				}
				mDirToWorld.q = axisRot.getNormalized();
				mDirToWorld.p = mPose.p;
			}
				mExecuteParams.worldToDir = mDirToWorld.getInverse();
		}
		{
			const PxVec3 vecN = mDirToWorld.rotate(mLocalDirVar);

			const PxVec3 a = mUnit.cross(vecN);
			const PxQuat axisRot(a.x, a.y, a.z, sqrtf(mUnit.magnitudeSquared() * vecN.magnitudeSquared()) + mUnit.dot(vecN));

			PxTransform instDirToWorld;
			instDirToWorld.q = axisRot.getNormalized();
			instDirToWorld.p = mPose.p;
			mExecuteParams.worldToInstDir = instDirToWorld.getInverse();
		}
		mExecuteParams.strength = mFieldStrength * mScale;
		mExecuteParams.instStrength = mExecuteParams.strength * (1.0f + mStrengthVar);

		shapeDesc.type = FieldShapeTypeIntl::CAPSULE;
		shapeDesc.dimensions = PxVec3(mExecuteParams.farRadius, mExecuteParams.farRadius * mExecuteParams.directionalStretch, 0);
		shapeDesc.worldToShape = mExecuteParams.worldToDir;
		shapeDesc.weight = mFieldWeight;

		float gridShapeRadius = mAsset->mParams->gridShapeRadius * mScale;
		float gridShapeHeight = mAsset->mParams->gridShapeHeight * mScale;

		mExecuteParams.gridIncludeShape.type = FieldShapeTypeIntl::CAPSULE;
		mExecuteParams.gridIncludeShape.dimensions = PxVec3(gridShapeRadius, gridShapeHeight, 0);
		mExecuteParams.gridIncludeShape.worldToShape = mExecuteParams.worldToDir;
		mExecuteParams.gridIncludeShape.weight = mFieldWeight;
		mExecuteParams.gridIncludeShape.fade = mAsset->mParams->gridBoundaryFadePercentage;

		mExecuteParams.noiseStrength = mAsset->mParams->noisePercentage * mExecuteParams.strength;
		mExecuteParams.noiseSpaceScale = mAsset->mParams->noiseSpaceScale / mScale;

		mFieldSamplerChanged = false;
		return true;
	}
	return false;
}

void JetFSActorImpl::simulate(float dt)
{
	WRITE_ZONE();
	if (mFieldStrengthVO != NULL)
	{
		mStrengthVar = mFieldStrengthVO->updateVariableOscillator(dt);

		mFieldSamplerChanged = true;
	}
	if (mFieldDirectionVO1 != NULL && mFieldDirectionVO2 != NULL)
	{
		float theta = mFieldDirectionVO1->updateVariableOscillator(dt);
		float phi = mFieldDirectionVO2->updateVariableOscillator(dt);

		mLocalDirVar.x = PxCos(phi) * PxSin(theta);
		mLocalDirVar.y = PxCos(theta);
		mLocalDirVar.z = PxSin(phi) * PxSin(theta);

		mFieldSamplerChanged = true;
	}
}

void JetFSActorImpl::setFieldStrength(float strength)
{
	WRITE_ZONE();
	mFieldStrength = strength;
	mFieldSamplerChanged = true;
}

void JetFSActorImpl::setFieldDirection(const PxVec3& direction)
{
	WRITE_ZONE();
	mFieldDirection = direction.getNormalized();
	mFieldSamplerChanged = true;
}

void JetFSActorImpl::visualize()
{
	WRITE_ZONE();
#ifndef WITHOUT_DEBUG_VISUALIZE
	if ( !mEnableDebugVisualization ) return;
	RenderDebugInterface* debugRender = mScene->mDebugRender;
	BasicFSDebugRenderParams* debugRenderParams = mScene->mBasicFSDebugRenderParams;

	if (!debugRenderParams->VISUALIZE_JET_FS_ACTOR)
	{
		return;
	}

	using RENDER_DEBUG::DebugColors;

	if (debugRenderParams->VISUALIZE_JET_FS_ACTOR_NAME)
	{
		char buf[128];
		buf[sizeof(buf) - 1] = 0;
		APEX_SPRINTF_S(buf, sizeof(buf) - 1, " %s %s", mAsset->getObjTypeName(), mAsset->getName());

		PxVec3 textLocation = mPose.p;

		RENDER_DEBUG_IFACE(debugRender)->setCurrentTextScale(4.0f);
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::Blue));
		RENDER_DEBUG_IFACE(debugRender)->debugText(textLocation, buf);
	}

	if (debugRenderParams->VISUALIZE_JET_FS_SHAPE)
	{
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::Blue));
		RENDER_DEBUG_IFACE(debugRender)->setPose(mDirToWorld);
		RENDER_DEBUG_IFACE(debugRender)->debugCapsule(mExecuteParams.farRadius, mExecuteParams.farRadius * mExecuteParams.directionalStretch, 2);
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::DarkBlue));
		RENDER_DEBUG_IFACE(debugRender)->debugCapsule(mExecuteParams.gridIncludeShape.dimensions.x, mExecuteParams.gridIncludeShape.dimensions.y, 2);
		RENDER_DEBUG_IFACE(debugRender)->setPose(physx::PxIdentity);


		//draw torus
		const uint32_t NUM_PHI_SLICES = 16;
		const uint32_t NUM_THETA_SLICES = 16;

		const float torusRadius = mExecuteParams.farRadius / 2;

		float cosPhiLast = 1;
		float sinPhiLast = 0;
		for (uint32_t i = 1; i <= NUM_PHI_SLICES; ++i)
		{
			float phi = (i * PxTwoPi / NUM_PHI_SLICES);
			float cosPhi = PxCos(phi);
			float sinPhi = PxSin(phi);

			RENDER_DEBUG_IFACE(debugRender)->debugLine(
			    mDirToWorld.transform(PxVec3(cosPhiLast * mExecuteParams.pivotRadius, 0, sinPhiLast * mExecuteParams.pivotRadius)),
			    mDirToWorld.transform(PxVec3(cosPhi * mExecuteParams.pivotRadius, 0, sinPhi * mExecuteParams.pivotRadius)));

			RENDER_DEBUG_IFACE(debugRender)->debugLine(
			    mDirToWorld.transform(PxVec3(cosPhiLast * mExecuteParams.nearRadius, 0, sinPhiLast * mExecuteParams.nearRadius)),
			    mDirToWorld.transform(PxVec3(cosPhi * mExecuteParams.nearRadius, 0, sinPhi * mExecuteParams.nearRadius)));

			float cosThetaLast = 1;
			float sinThetaLast = 0;
			for (uint32_t j = 1; j <= NUM_THETA_SLICES; ++j)
			{
				float theta = (j * PxTwoPi / NUM_THETA_SLICES);
				float cosTheta = PxCos(theta);
				float sinTheta = PxSin(theta);

				float d = torusRadius * (1 + cosTheta);
				float h = torusRadius * sinTheta * mExecuteParams.directionalStretch;

				float dLast = torusRadius * (1 + cosThetaLast);
				float hLast = torusRadius * sinThetaLast * mExecuteParams.directionalStretch;

				RENDER_DEBUG_IFACE(debugRender)->debugLine(
				    mDirToWorld.transform(PxVec3(cosPhi * dLast, hLast, sinPhi * dLast)),
				    mDirToWorld.transform(PxVec3(cosPhi * d, h, sinPhi * d)));

				RENDER_DEBUG_IFACE(debugRender)->debugLine(
				    mDirToWorld.transform(PxVec3(cosPhiLast * d, h, sinPhiLast * d)),
				    mDirToWorld.transform(PxVec3(cosPhi * d, h, sinPhi * d)));

				RENDER_DEBUG_IFACE(debugRender)->debugLine(
				    mDirToWorld.transform(PxVec3(cosPhiLast * dLast, hLast, sinPhiLast * dLast)),
				    mDirToWorld.transform(PxVec3(cosPhi * dLast, hLast, sinPhi * dLast)));

				cosThetaLast = cosTheta;
				sinThetaLast = sinTheta;
			}

			cosPhiLast = cosPhi;
			sinPhiLast = sinPhi;
		}
	}

	const float spreadDistance = 0.5f * mExecuteParams.farRadius * mExecuteParams.directionalStretch;

	/*
		const uint32_t CircleDivCount = 8;
		for (float angle = 0; angle < PxTwoPi; angle += PxTwoPi / CircleDivCount)
		{
			float c1 = cos(angle);
			float s1 = sin(angle);

			//RENDER_DEBUG_IFACE(debugRender)->setCurrentColor( RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::LightBlue) );
			RENDER_DEBUG_IFACE(debugRender)->debugOrientedLine(
				PxVec3(innerRadius * c1, 0, innerRadius * s1),
				PxVec3(outerRadius * c1, -spreadDistance, outerRadius * s1),
				mLocalToWorld);
			RENDER_DEBUG_IFACE(debugRender)->debugOrientedLine(
				PxVec3(innerRadius * c1, 0, innerRadius * s1),
				PxVec3(outerRadius * c1, +spreadDistance, outerRadius * s1),
				mLocalToWorld);
		}
	*/
	using RENDER_DEBUG::DebugColors;

	if (debugRenderParams->VISUALIZE_JET_FS_FIELD)
	{
		float fieldScale = debugRenderParams->JET_FS_FIELD_SCALE;

		PxVec3 rayBeg0 = mPose.p;
		PxVec3 rayEnd0 = rayBeg0 + (mFieldStrength * fieldScale) * mFieldDirection;
		RENDER_DEBUG_IFACE(debugRender)->setCurrentColor(RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::DarkBlue));
		RENDER_DEBUG_IFACE(debugRender)->debugRay(rayBeg0, rayEnd0);


		//PxVec3 rayBeg = mPose.p;
		//PxVec3 rayEnd = rayBeg + (mExecuteParams.strength * fieldScale) * mExecuteParams.direction;
		//RENDER_DEBUG_IFACE(debugRender)->setCurrentColor( RENDER_DEBUG_IFACE(debugRender)->getDebugColor(DebugColors::Blue) );
		//RENDER_DEBUG_IFACE(debugRender)->debugRay(rayBeg, rayEnd);
	}
	if (debugRenderParams->VISUALIZE_JET_FS_POSE)
	{
		RENDER_DEBUG_IFACE(debugRender)->debugAxes(PxMat44(mPose), 1);
	}

	if (debugRenderParams->VISUALIZE_JET_FS_FIELD)
	{
		if (mDebugPoints.empty())
		{
			mDebugPoints.resize(NUM_DEBUG_POINTS);

			for (uint32_t i = 0; i < NUM_DEBUG_POINTS; ++i)
			{
				float rx, ry;
				do
				{
					rx = physx::shdfnd::rand(-1.0f, +1.0f);
					ry = physx::shdfnd::rand(-1.0f, +1.0f);
				}
				while (rx * rx + ry * ry > 1.0f);

				PxVec3& vec = mDebugPoints[i];

				vec.x = rx;
				vec.y = physx::shdfnd::rand(-1.0f, +1.0f);
				vec.z = ry;
			}
		}
		using RENDER_DEBUG::DebugColors;
		uint32_t c1 = RENDER_DEBUG_IFACE(mScene->mDebugRender)->getDebugColor(DebugColors::Blue);
		uint32_t c2 = RENDER_DEBUG_IFACE(mScene->mDebugRender)->getDebugColor(DebugColors::Red);

		uint32_t totalElapsedMS = mScene->getApexScene().getTotalElapsedMS();

		for (uint32_t i = 0; i < NUM_DEBUG_POINTS; ++i)
		{
			PxVec3 localPos = mDebugPoints[i];
			localPos.x *= mExecuteParams.farRadius;
			localPos.y *= spreadDistance;
			localPos.z *= mExecuteParams.farRadius;

			PxVec3 pos = mDirToWorld.transform(localPos);
			PxVec3 fieldVec = executeJetFS(mExecuteParams, pos, totalElapsedMS);
			fieldVec *= debugRenderParams->JET_FS_FIELD_SCALE;
			RENDER_DEBUG_IFACE(debugRender)->debugGradientLine(pos, pos + fieldVec, c1, c2);
		}
	}
#endif
}

/******************************** CPU Version ********************************/

JetFSActorCPU::JetFSActorCPU(const JetFSActorParams& params, JetFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: JetFSActorImpl(params, asset, list, scene)
{
}

JetFSActorCPU::~JetFSActorCPU()
{
}

void JetFSActorCPU::executeFieldSampler(const ExecuteData& data)
{
	WRITE_ZONE();
	uint32_t totalElapsedMS = mScene->getApexScene().getTotalElapsedMS();
	for (uint32_t iter = 0; iter < data.count; ++iter)
	{
		uint32_t i = data.indices[iter & data.indicesMask] + (iter & ~data.indicesMask);
		PxVec3* pos = (PxVec3*)((uint8_t*)data.position + i * data.positionStride);
		data.resultField[iter] = executeJetFS(mExecuteParams, *pos, totalElapsedMS);
	}
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT


JetFSActorGPU::JetFSActorGPU(const JetFSActorParams& params, JetFSAsset& asset, ResourceList& list, BasicFSScene& scene)
	: JetFSActorCPU(params, asset, list, scene)
	, mConstMemGroup(CUDA_OBJ(fieldSamplerStorage))
{
}

JetFSActorGPU::~JetFSActorGPU()
{
}

bool JetFSActorGPU::updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled)
{
	WRITE_ZONE();
	if (JetFSActorImpl::updateFieldSampler(shapeDesc, isEnabled))
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

