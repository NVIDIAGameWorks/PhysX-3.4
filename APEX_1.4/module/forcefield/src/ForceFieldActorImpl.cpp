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
#include "Apex.h"
#include "ForceFieldActorImpl.h"
#include "ForceFieldAssetImpl.h"
#include "ForceFieldScene.h"
#include "ApexSDKIntl.h"
#include "SceneIntl.h"
#include "ModuleForceField.h"

namespace nvidia
{
namespace forcefield
{

ForceFieldActorImpl::ForceFieldActorImpl(const ForceFieldActorDesc& desc, ForceFieldAssetImpl& asset, ResourceList& list, ForceFieldScene& scene):
	mForceFieldScene(&scene),
	mName(desc.actorName),
	mAsset(&asset),
	mEnable(true),
	mElapsedTime(0)
{
	//not actually used
	//mScale = desc.scale * mAsset->getDefaultScale();

	list.add(*this);			// Add self to asset's list of actors
	addSelfToContext(*scene.mApexScene->getApexContext());    // Add self to ApexScene
	addSelfToContext(scene);	// Add self to FieldBoundaryScene's list of actors

	initActorParams(desc.initialPose);
	initFieldSampler(desc);
}

ForceFieldShapeType::Enum getIncludeShapeType(const ForceFieldAssetParams& assetParams)
{
	NvParameterized::Handle hEnum(assetParams);
	assetParams.getParameterHandle("includeShapeParameters.shape", hEnum);
	PX_ASSERT(hEnum.isValid());

	// assuming that enums in ForceFieldAssetParamSchema line up with ForceFieldShapeType::Enum
	int32_t shapeInt = hEnum.parameterDefinition()->enumValIndex(assetParams.includeShapeParameters.shape);
	if (-1 != shapeInt)
	{
		return (ForceFieldShapeType::Enum)shapeInt;
	}
	return ForceFieldShapeType::NONE;
}

ForceFieldCoordinateSystemType::Enum getCoordinateSystemType(const GenericForceFieldKernelParams& assetParams)
{
	NvParameterized::Handle hEnum(assetParams);
	assetParams.getParameterHandle("coordinateSystemParameters.type", hEnum);
	PX_ASSERT(hEnum.isValid());

	// assuming that enums in ForceFieldAssetParamSchema line up with ForceFieldShapeType::Enum
	int32_t shapeInt = hEnum.parameterDefinition()->enumValIndex(assetParams.coordinateSystemParameters.type);
	if (-1 != shapeInt)
	{
		return (ForceFieldCoordinateSystemType::Enum)shapeInt;
	}
	return ForceFieldCoordinateSystemType::CARTESIAN;
}

void ForceFieldActorImpl::initActorParams(const PxMat44& initialPose)
{
	mLifetime = mAsset->mParams->lifetime;

	//ForceFieldFSKernelParams initialization
	{
		ForceFieldFSKernelParams& params = mKernelParams.getForceFieldFSKernelParams();
		params.pose = initialPose;
		params.strength = mAsset->mParams->strength;
		params.includeShape.dimensions = mAsset->mParams->includeShapeParameters.dimensions;
		params.includeShape.forceFieldToShape = mAsset->mParams->includeShapeParameters.forceFieldToShape;
		params.includeShape.type = getIncludeShapeType(*mAsset->mParams);
	}

	if (mAsset->mGenericParams)
	{
		mKernelParams.kernelType = ForceFieldKernelType::GENERIC;
		GenericForceFieldFSKernelParams& params = mKernelParams.getGenericForceFieldFSKernelParams();
		
		params.cs.torusRadius = mAsset->mGenericParams->coordinateSystemParameters.torusRadius;
		params.cs.type = getCoordinateSystemType(*mAsset->mGenericParams);
		params.constant = mAsset->mGenericParams->constant;
		params.positionMultiplier = mAsset->mGenericParams->positionMultiplier;
		params.positionTarget = mAsset->mGenericParams->positionTarget;
		params.velocityMultiplier = mAsset->mGenericParams->velocityMultiplier;
		params.velocityTarget = mAsset->mGenericParams->velocityTarget;
		params.noise = mAsset->mGenericParams->noise;
		params.falloffLinear = mAsset->mGenericParams->falloffLinear;
		params.falloffQuadratic = mAsset->mGenericParams->falloffQuadratic;
	}
	else if (mAsset->mRadialParams)
	{
		mKernelParams.kernelType = ForceFieldKernelType::RADIAL;
		RadialForceFieldFSKernelParams& params = mKernelParams.getRadialForceFieldFSKernelParams();
		
		// falloff parameters TODO: these should move to  mAsset->mRadialParams->parameters()
		params.falloffTable.multiplier = mAsset->mFalloffParams->multiplier;
		params.falloffTable.x1 = mAsset->mFalloffParams->start;
		params.falloffTable.x2 = mAsset->mFalloffParams->end;
		setRadialFalloffType(mAsset->mFalloffParams->type);

		// noise parameters TODO: these should move to  mAsset->mRadialParams->parameters()
		params.noiseParams.strength = mAsset->mNoiseParams->strength;
		params.noiseParams.spaceScale = mAsset->mNoiseParams->spaceScale;
		params.noiseParams.timeScale = mAsset->mNoiseParams->timeScale;
		params.noiseParams.octaves = mAsset->mNoiseParams->octaves;

		params.radius = mAsset->mRadialParams->radius;
		PX_ASSERT(params.radius > 0.9e-3f);
	}
	else
	{
		PX_ASSERT(0 && "Invalid geometry type for APEX turbulence source.");
		return;
	}

	//do first copy of double buffered params
	memcpy(&mKernelExecutionParams, &mKernelParams, sizeof(ForceFieldFSKernelParamsUnion));
}


void ForceFieldActorImpl::setPhysXScene(PxScene* /*scene*/)
{
}

PxScene* ForceFieldActorImpl::getPhysXScene() const
{
	return NULL;
}

void ForceFieldActorImpl::getLodRange(float& min, float& max, bool& intOnly) const
{
	READ_ZONE();
	PX_UNUSED(min);
	PX_UNUSED(max);
	PX_UNUSED(intOnly);
	APEX_INVALID_OPERATION("not implemented");
}

float ForceFieldActorImpl::getActiveLod() const
{
	READ_ZONE();
	APEX_INVALID_OPERATION("ForceFieldActor does not support this operation");
	return -1.0f;
}

void ForceFieldActorImpl::forceLod(float lod)
{
	WRITE_ZONE();
	PX_UNUSED(lod);
	APEX_INVALID_OPERATION("not implemented");
}

/* Must be defined inside CPP file, since they require knowledge of asset class */
Asset* ForceFieldActorImpl::getOwner() const
{
	READ_ZONE();
	return (Asset*)mAsset;
}

ForceFieldAsset* ForceFieldActorImpl::getForceFieldAsset() const
{
	READ_ZONE();
	return mAsset;
}

void ForceFieldActorImpl::release()
{
	if (mInRelease)
	{
		return;
	}
	destroy();
}

void ForceFieldActorImpl::destroy()
{
	ApexActor::destroy();
	setPhysXScene(NULL);
	releaseFieldSampler();
	delete this;
}

bool ForceFieldActorImpl::enable()
{
	WRITE_ZONE();
	if (mEnable)
	{
		return true;
	}
	mEnable = true;
	mFieldSamplerChanged = true;
	return true;
}

bool ForceFieldActorImpl::disable()
{
	WRITE_ZONE();
	if (!mEnable)
	{
		return true;
	}
	mEnable = false;
	mFieldSamplerChanged = true;
	return true;
}

PxMat44 ForceFieldActorImpl::getPose() const
{
	READ_ZONE();
	const ForceFieldFSKernelParams& kernelParams = mKernelParams.getForceFieldFSKernelParams();
	return kernelParams.pose;
}

void ForceFieldActorImpl::setPose(const PxMat44& pose)
{
	WRITE_ZONE();
	ForceFieldFSKernelParams& kernelParams = mKernelParams.getForceFieldFSKernelParams();
	kernelParams.pose = pose;
	mFieldSamplerChanged = true;
}

//deprecated, has no effect
void ForceFieldActorImpl::setScale(float)
{
	WRITE_ZONE();
}

void ForceFieldActorImpl::setStrength(const float strength)
{
	WRITE_ZONE();
	ForceFieldFSKernelParams& kernelParams = mKernelParams.getForceFieldFSKernelParams();
	kernelParams.strength = strength;
	mFieldSamplerChanged = true;
}

void ForceFieldActorImpl::setLifetime(const float lifetime)
{
	WRITE_ZONE();
	mLifetime = lifetime;
	mFieldSamplerChanged = true;
}

//deprecated
void ForceFieldActorImpl::setFalloffType(const char* type)
{
	WRITE_ZONE();
	if (mKernelParams.kernelType == ForceFieldKernelType::RADIAL)
	{
		setRadialFalloffType(type);
	}
}

//deprecated
void ForceFieldActorImpl::setFalloffMultiplier(const float multiplier)
{
	WRITE_ZONE();
	if (mKernelParams.kernelType == ForceFieldKernelType::RADIAL)
	{
		setRadialFalloffMultiplier(multiplier);
	}
}

void ForceFieldActorImpl::setRadialFalloffType(const char* type)
{
	WRITE_ZONE();
	PX_ASSERT(mKernelParams.kernelType == ForceFieldKernelType::RADIAL);
	RadialForceFieldFSKernelParams& kernelParams = mKernelParams.getRadialForceFieldFSKernelParams();

	ForceFieldFalloffType::Enum falloffType;

	NvParameterized::Handle hEnum(mAsset->mFalloffParams);
	mAsset->mFalloffParams->getParameterHandle("type", hEnum);
	PX_ASSERT(hEnum.isValid());

	// assuming that enums in ForceFieldAssetParamSchema line up with ForceFieldFalloffType::Enum
	int32_t typeInt = hEnum.parameterDefinition()->enumValIndex(type);
	if (-1 != typeInt)
	{
		falloffType = (ForceFieldFalloffType::Enum)typeInt;
	}
	else
	{
		falloffType = ForceFieldFalloffType::NONE;
	}

	switch (falloffType)
	{
	case ForceFieldFalloffType::LINEAR:
		{
			kernelParams.falloffTable.applyStoredTable(TableName::LINEAR);
			break;
		}
	case ForceFieldFalloffType::STEEP:
		{
			kernelParams.falloffTable.applyStoredTable(TableName::STEEP);
			break;
		}
	case ForceFieldFalloffType::SCURVE:
		{
			kernelParams.falloffTable.applyStoredTable(TableName::SCURVE);
			break;
		}
	case ForceFieldFalloffType::CUSTOM:
		{
			kernelParams.falloffTable.applyStoredTable(TableName::CUSTOM);
			break;
		}
	case ForceFieldFalloffType::NONE:
		{
			kernelParams.falloffTable.applyStoredTable(TableName::CUSTOM);	// all-1 stored table
		}
	}

	kernelParams.falloffTable.buildTable();
	mFieldSamplerChanged = true;
}

void ForceFieldActorImpl::setRadialFalloffMultiplier(const float multiplier)
{
	PX_ASSERT(mKernelParams.kernelType == ForceFieldKernelType::RADIAL);
	RadialForceFieldFSKernelParams& kernelParams = mKernelParams.getRadialForceFieldFSKernelParams();

	kernelParams.falloffTable.multiplier = multiplier;
	kernelParams.falloffTable.buildTable();
	mFieldSamplerChanged = true;
}

void ForceFieldActorImpl::updateForceField(float dt)
{
	mElapsedTime += dt;

	if (mLifetime > 0.0f && mElapsedTime > mLifetime)
	{
		disable();
	}
}

// Called by ForceFieldScene::fetchResults()
void ForceFieldActorImpl::updatePoseAndBounds()
{
}

void ForceFieldActorImpl::visualize()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if ( !mEnableDebugVisualization ) return;
	if (mEnable)
	{
		visualizeIncludeShape();
	}
#endif
}

void ForceFieldActorImpl::visualizeIncludeShape()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (mEnable)
	{
		const physx::PxMat44& savedPose = *RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->getPoseTyped();
		RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setIdentityPose();
		RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->pushRenderState();
		RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setCurrentColor(0xFF0000);

		ForceFieldFSKernelParams& kernelParams = mKernelParams.getForceFieldFSKernelParams();
		PxMat44 debugPose = kernelParams.includeShape.forceFieldToShape * kernelParams.pose;

		switch (kernelParams.includeShape.type)
		{
		case ForceFieldShapeType::SPHERE:
			{
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(debugPose);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->debugSphere(PxVec3(0.0f), kernelParams.includeShape.dimensions.x, 2);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(PxIdentity);
				break;
			}
		case ForceFieldShapeType::CAPSULE:
			{
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(debugPose);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->debugCapsule(kernelParams.includeShape.dimensions.x, kernelParams.includeShape.dimensions.y * 2, 2);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(PxIdentity);
				break;
			}
		case ForceFieldShapeType::CYLINDER:
			{
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(debugPose);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->debugCylinder(kernelParams.includeShape.dimensions.x, kernelParams.includeShape.dimensions.y * 2, true, 2);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(PxIdentity);
				break;
			}
		case ForceFieldShapeType::CONE:
			{
				// using a cylinder to approximate a cone for debug rendering
				// TODO: draw a cone using lines
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(debugPose);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->debugCylinder(kernelParams.includeShape.dimensions.x, kernelParams.includeShape.dimensions.y * 2, true, 2);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(PxIdentity);
				break;
			}
		case ForceFieldShapeType::BOX:
			{
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(debugPose);
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->debugBound(PxBounds3(PxVec3(0.0f), kernelParams.includeShape.dimensions * 2));
				RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(PxIdentity);
				break;
			}
		default:
			{
			}
		}

		RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->setPose(savedPose);
		RENDER_DEBUG_IFACE(mForceFieldScene->mRenderDebug)->popRenderState();
	}
#endif
}

void ForceFieldActorImpl::visualizeForces()
{
#ifndef WITHOUT_DEBUG_VISUALIZE
	if (mEnable)
	{
	}
#endif
}

}
} // end namespace nvidia

