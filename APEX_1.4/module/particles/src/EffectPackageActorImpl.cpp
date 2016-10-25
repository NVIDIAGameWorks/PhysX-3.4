/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "EffectPackageActorImpl.h"
#include "FloatMath.h"
#include "ParticlesScene.h"
#include "SceneIntl.h"
#include "EmitterActor.h"
#include "EmitterAsset.h"
#include "TurbulenceFSActor.h"
#include "AttractorFSActor.h"
#include "JetFSActor.h"
#include "WindFSActor.h"
#include "NoiseFSActor.h"
#include "VortexFSActor.h"
#include "nvparameterized/NvParamUtils.h"
#include "ImpactEmitterActor.h"
#include "GroundEmitterActor.h"
#include "TurbulenceFSAsset.h"
#include "BasicIosAsset.h"
#include "RenderDebugInterface.h"
#include "BasicFSAsset.h"
#include "RenderDebugInterface.h"
#include "SceneIntl.h"
#include "ForceFieldAsset.h"
#include "ForceFieldActor.h"
#include "HeatSourceAsset.h"
#include "HeatSourceActor.h"
#include "HeatSourceAssetParams.h"

#include "ApexEmitterActorParameters.h"
#include "ApexEmitterAssetParameters.h"

#include "SubstanceSourceAsset.h"
#include "SubstanceSourceActor.h"
#include "SubstanceSourceAsset.h"
#include "SubstanceSourceAssetParams.h"

#include "VelocitySourceAsset.h"
#include "VelocitySourceActor.h"
#include "VelocitySourceAssetParams.h"

#include "FlameEmitterAsset.h"
#include "FlameEmitterActor.h"
#include "FlameEmitterAssetParams.h"

#include "TurbulenceFSAssetParams.h"

#include "ScopedPhysXLock.h"

#include "PxPhysics.h"
#include "PxScene.h"
#include "ApexResourceHelper.h"
#include "PsMathUtils.h"


#pragma warning(disable:4100)

namespace nvidia
{
namespace particles
{
	using namespace physx;

static PxTransform _getPose(float x, float y, float z, float rotX, float rotY, float rotZ)
{
	PxTransform ret;
	ret.p = PxVec3(x, y, z);
	fm_eulerToQuat(rotX * FM_DEG_TO_RAD, rotY * FM_DEG_TO_RAD, rotZ * FM_DEG_TO_RAD, &ret.q.x);
	return ret;
}

void _getRot(const PxQuat& q, float& rotX, float& rotY, float& rotZ)
{
	fm_quatToEuler((float*)&q, rotX, rotY, rotZ);
	rotX *= FM_RAD_TO_DEG;
	rotY *= FM_RAD_TO_DEG;
	rotZ *= FM_RAD_TO_DEG;
}

static float ranf(void)
{
	uint32_t r = (uint32_t)::rand();
	r &= 0x7FFF;
	return (float)r * (1.0f / 32768.0f);
}

static float ranf(float min, float max)
{
	return ranf() * (max - min) + min;
}

EffectPackageActorImpl::EffectPackageActorImpl(EffectPackageAsset* apexAsset,
                                       const EffectPackageAssetParams* assetParams,
                                       const EffectPackageActorParams* actorParams,
                                       nvidia::apex::ApexSDK& sdk,
                                       nvidia::apex::Scene& scene,
                                       ParticlesScene& dynamicSystemScene,
									   ModuleTurbulenceFS* moduleTurbulenceFS)
{
	mRigidBodyChange = false;
	mRenderVolume = NULL;
	mEmitterValidateCallback = NULL;
	mAlive = true;
	mSimTime = 0;
	mCurrentLifeTime = 0;
	mFadeIn = false;
	mFadeInTime = 0;
	mFadeInDuration = 0;
	mFadeOut = false;
	mFadeOutTime = 0;
	mFadeOutDuration = 0;
	mFirstFrame = true;
	mData = assetParams;
	mAsset = apexAsset;
	mScene = &scene;
	mModuleTurbulenceFS = moduleTurbulenceFS;
	mEnabled = actorParams->Enabled;
	mVisible = false;
	mEverVisible = false;
	mOffScreenTime = 0;
	mVisState = VS_ON_SCREEN;
	mFadeTime = 0;
	mNotVisibleTime = 0;
	mPose = actorParams->InitialPose;
	mObjectScale = actorParams->objectScale;
	mEffectPath = PX_NEW(EffectPath);
	{
		RigidBodyEffectNS::EffectPath_Type *path = (RigidBodyEffectNS::EffectPath_Type *)&mData->Path;
		if ( !mEffectPath->init(*path))
		{
			delete mEffectPath;
			mEffectPath = NULL;
		}
	}

	for (int32_t i = 0; i < mData->Effects.arraySizes[0]; i++)
	{
		NvParameterized::Interface* iface = mData->Effects.buf[i];
		PX_ASSERT(iface);
		if (iface)
		{
			EffectType type = getEffectType(iface);
			switch (type)
			{
			case ET_EMITTER:
			{
				EmitterEffect* ed = static_cast< EmitterEffect*>(iface);
				if (ed->Emitter)
				{
					EffectEmitter* ee = PX_NEW(EffectEmitter)(ed->Emitter->name(), ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
					ee->setCurrentScale(mObjectScale,mEffectPath);
					mEffects.pushBack(static_cast< EffectData*>(ee));
				}
			}
			break;
			case ET_HEAT_SOURCE:
			{
				HeatSourceEffect* ed = static_cast< HeatSourceEffect*>(iface);
				EffectHeatSource* ee = PX_NEW(EffectHeatSource)(ed->HeatSource->name(), ed, sdk, scene, dynamicSystemScene, moduleTurbulenceFS, mPose, mEnabled);
				ee->setCurrentScale(mObjectScale,mEffectPath);
				mEffects.pushBack(static_cast< EffectData*>(ee));
			}
			break;
			case ET_SUBSTANCE_SOURCE:
				{
					SubstanceSourceEffect* ed = static_cast< SubstanceSourceEffect*>(iface);
					EffectSubstanceSource* ee = PX_NEW(EffectSubstanceSource)(ed->SubstanceSource->name(), ed, sdk, scene, dynamicSystemScene, moduleTurbulenceFS, mPose, mEnabled);
					ee->setCurrentScale(mObjectScale,mEffectPath);
					mEffects.pushBack(static_cast< EffectData*>(ee));
				}
				break;
			case ET_VELOCITY_SOURCE:
				{
					VelocitySourceEffect* ed = static_cast< VelocitySourceEffect*>(iface);
					EffectVelocitySource* ee = PX_NEW(EffectVelocitySource)(ed->VelocitySource->name(), ed, sdk, scene, dynamicSystemScene, moduleTurbulenceFS, mPose, mEnabled);
					ee->setCurrentScale(mObjectScale,mEffectPath);
					mEffects.pushBack(static_cast< EffectData*>(ee));
				}
				break;
			case ET_FLAME_EMITTER:
				{
					FlameEmitterEffect* ed = static_cast< FlameEmitterEffect*>(iface);
					EffectFlameEmitter* ee = PX_NEW(EffectFlameEmitter)(ed->FlameEmitter->name(), ed, sdk, scene, dynamicSystemScene, moduleTurbulenceFS, mPose, mEnabled);
					ee->setCurrentScale(mObjectScale,mEffectPath);
					mEffects.pushBack(static_cast< EffectData*>(ee));
				}
				break;
			case ET_TURBULENCE_FS:
			{
				TurbulenceFieldSamplerEffect* ed = static_cast< TurbulenceFieldSamplerEffect*>(iface);
				EffectTurbulenceFS* ee = PX_NEW(EffectTurbulenceFS)(ed->TurbulenceFieldSampler->name(), ed, sdk, scene, dynamicSystemScene, moduleTurbulenceFS, mPose, mEnabled);
				ee->setCurrentScale(mObjectScale,mEffectPath);
				mEffects.pushBack(static_cast< EffectData*>(ee));
			}
			break;
			case ET_JET_FS:
			{
				JetFieldSamplerEffect* ed = static_cast< JetFieldSamplerEffect*>(iface);
				EffectJetFS* ee = PX_NEW(EffectJetFS)(ed->JetFieldSampler->name(), ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
				ee->setCurrentScale(mObjectScale,mEffectPath);
				mEffects.pushBack(static_cast< EffectData*>(ee));
			}
			break;
			case ET_WIND_FS:
				{
					WindFieldSamplerEffect* ed = static_cast< WindFieldSamplerEffect*>(iface);
					EffectWindFS* ee = PX_NEW(EffectWindFS)(ed->WindFieldSampler->name(), ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
					ee->setCurrentScale(mObjectScale,mEffectPath);
					mEffects.pushBack(static_cast< EffectData*>(ee));
				}
				break;
			case ET_RIGID_BODY:
				{
					RigidBodyEffect* ed = static_cast< RigidBodyEffect*>(iface);
					EffectRigidBody* ee = PX_NEW(EffectRigidBody)("RigidBody", ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
					ee->setCurrentScale(mObjectScale,mEffectPath);
					mEffects.pushBack(static_cast< EffectData*>(ee));
				}
				break;
			case ET_NOISE_FS:
			{
				NoiseFieldSamplerEffect* ed = static_cast< NoiseFieldSamplerEffect*>(iface);
				EffectNoiseFS* ee = PX_NEW(EffectNoiseFS)(ed->NoiseFieldSampler->name(), ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
				ee->setCurrentScale(mObjectScale,mEffectPath);
				mEffects.pushBack(static_cast< EffectData*>(ee));
			}
			break;
			case ET_VORTEX_FS:
			{
				VortexFieldSamplerEffect* ed = static_cast< VortexFieldSamplerEffect*>(iface);
				EffectVortexFS* ee = PX_NEW(EffectVortexFS)(ed->VortexFieldSampler->name(), ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
				ee->setCurrentScale(mObjectScale,mEffectPath);
				mEffects.pushBack(static_cast< EffectData*>(ee));
			}
			break;
			case ET_ATTRACTOR_FS:
			{
				AttractorFieldSamplerEffect* ed = static_cast< AttractorFieldSamplerEffect*>(iface);
				EffectAttractorFS* ee = PX_NEW(EffectAttractorFS)(ed->AttractorFieldSampler->name(), ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
				ee->setCurrentScale(mObjectScale,mEffectPath);
				mEffects.pushBack(static_cast< EffectData*>(ee));
			}
			break;
			case ET_FORCE_FIELD:
			{
				ForceFieldEffect* ed = static_cast< ForceFieldEffect*>(iface);
				EffectForceField* ee = PX_NEW(EffectForceField)(ed->ForceField->name(), ed, sdk, scene, dynamicSystemScene, mPose, mEnabled);
				ee->setCurrentScale(mObjectScale,mEffectPath);
				mEffects.pushBack(static_cast< EffectData*>(ee));
			}
			break;
			default:
				PX_ALWAYS_ASSERT();
				break;
			}
		}
	}
	addSelfToContext(*dynamicSystemScene.mApexScene->getApexContext());    // Add self to ApexScene
	addSelfToContext(dynamicSystemScene);	// Add self to ParticlesScene's list of actors
}

EffectPackageActorImpl::~EffectPackageActorImpl(void)
{
	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		EffectData* ed = mEffects[i];
		delete ed;
	}
	delete mEffectPath;
}

EffectType EffectPackageActorImpl::getEffectType(const NvParameterized::Interface* iface)
{
	EffectType ret = ET_LAST;

	if (nvidia::strcmp(iface->className(), EmitterEffect::staticClassName()) == 0)
	{
		ret = ET_EMITTER;
	}
	else if (nvidia::strcmp(iface->className(), HeatSourceEffect::staticClassName()) == 0)
	{
		ret = ET_HEAT_SOURCE;
	}
	else if (nvidia::strcmp(iface->className(), SubstanceSourceEffect::staticClassName()) == 0)
	{
		ret = ET_SUBSTANCE_SOURCE;
	}
	else if (nvidia::strcmp(iface->className(), VelocitySourceEffect::staticClassName()) == 0)
	{
		ret = ET_VELOCITY_SOURCE;
	}
	else if (nvidia::strcmp(iface->className(), FlameEmitterEffect::staticClassName()) == 0)
	{
		ret = ET_FLAME_EMITTER;
	}
	else if (nvidia::strcmp(iface->className(), ForceFieldEffect::staticClassName()) == 0)
	{
		ret = ET_FORCE_FIELD;
	}
	else if (nvidia::strcmp(iface->className(), JetFieldSamplerEffect::staticClassName()) == 0)
	{
		ret = ET_JET_FS;
	}
	else if (nvidia::strcmp(iface->className(), WindFieldSamplerEffect::staticClassName()) == 0)
	{
		ret = ET_WIND_FS;
	}
	else if (nvidia::strcmp(iface->className(), RigidBodyEffect::staticClassName()) == 0)
	{
		ret = ET_RIGID_BODY;
	}
	else if (nvidia::strcmp(iface->className(), NoiseFieldSamplerEffect::staticClassName()) == 0)
	{
		ret = ET_NOISE_FS;
	}
	else if (nvidia::strcmp(iface->className(), VortexFieldSamplerEffect::staticClassName()) == 0)
	{
		ret = ET_VORTEX_FS;
	}
	else if (nvidia::strcmp(iface->className(), AttractorFieldSamplerEffect::staticClassName()) == 0)
	{
		ret = ET_ATTRACTOR_FS;
	}
	else if (nvidia::strcmp(iface->className(), TurbulenceFieldSamplerEffect::staticClassName()) == 0)
	{
		ret = ET_TURBULENCE_FS;
	}
	else
	{
		PX_ALWAYS_ASSERT();
	}

	return ret;
}

const PxTransform& EffectPackageActorImpl::getPose(void) const
{
	READ_ZONE();
	return mPose;
}

void EffectPackageActorImpl::visualize(RenderDebugInterface* callback, bool solid) const
{
	using RENDER_DEBUG::DebugRenderState;
	if ( !mEnableDebugVisualization ) return;
	RENDER_DEBUG_IFACE(callback)->pushRenderState();
	
	RENDER_DEBUG_IFACE(callback)->addToCurrentState(DebugRenderState::CameraFacing);
	RENDER_DEBUG_IFACE(callback)->addToCurrentState(DebugRenderState::CenterText);

	RENDER_DEBUG_IFACE(callback)->debugText(mPose.p - PxVec3(0, 0.35f, 0), mAsset->getName());

	RENDER_DEBUG_IFACE(callback)->debugAxes(PxMat44(mPose));

	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		mEffects[i]->visualize(callback, solid);
	}

	RENDER_DEBUG_IFACE(callback)->popRenderState();
}

void EffectPackageActorImpl::setPose(const PxTransform& pose)
{
	WRITE_ZONE();
	mPose = pose;

	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		mEffects[i]->refresh(mPose, mEnabled, true, mRenderVolume,mEmitterValidateCallback);
	}
}

void EffectPackageActorImpl::refresh(void)
{
	WRITE_ZONE();
	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		EffectData* ed = mEffects[i];

		if (ed->getEffectActor() )
		{
			ed->refresh(mPose, mEnabled, true, mRenderVolume,mEmitterValidateCallback);
		}
		else if ( ed->getType() == ET_RIGID_BODY )
		{
			EffectRigidBody *erb = static_cast< EffectRigidBody *>(ed);
			if ( erb->mRigidDynamic )
			{
				ed->refresh(mPose, mEnabled, true, mRenderVolume,mEmitterValidateCallback);
			}
		}
	}
}

void EffectPackageActorImpl::release(void)
{
	delete this;
}

const char* EffectPackageActorImpl::getName(void) const
{
	READ_ZONE();
	return mAsset ? mAsset->getName() : NULL;
}


uint32_t EffectPackageActorImpl::getEffectCount(void) const // returns the number of effects in the effect package
{
	READ_ZONE();
	return mEffects.size();
}

EffectType EffectPackageActorImpl::getEffectType(uint32_t effectIndex) const // return the type of effect.
{
	READ_ZONE();
	EffectType ret = ET_LAST;

	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];
		ret = ed->getType();
	}

	return ret;
}

Actor* EffectPackageActorImpl::getEffectActor(uint32_t effectIndex) const // return the base Actor pointer
{
	READ_ZONE();
	Actor* ret = NULL;

	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];
		ret = ed->getEffectActor();
	}


	return ret;
}
void EffectPackageActorImpl::setEmitterState(bool state) // set the state for all emitters in this effect package.
{
	WRITE_ZONE();
	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		if (mEffects[i]->getType() == ET_EMITTER)
		{
			Actor* a = mEffects[i]->getEffectActor();
			if (a)
			{
				EmitterActor* ae = static_cast< EmitterActor*>(a);
				if (state)
				{
					ae->startEmit(false);
				}
				else
				{
					ae->stopEmit();
				}
			}
		}
	}
}

uint32_t EffectPackageActorImpl::getActiveParticleCount(void) const // return the total number of particles still active in this effect package.
{
	READ_ZONE();
	uint32_t ret = 0;

	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		if (mEffects[i]->getType() == ET_EMITTER)
		{
			Actor* a = mEffects[i]->getEffectActor();
			if (a)
			{
				EmitterActor* ae = static_cast< EmitterActor*>(a);
				ret += ae->getActiveParticleCount();
			}
		}
	}
	return ret;
}

bool EffectPackageActorImpl::isStillEmitting(void) const // return true if any emitters are still actively emitting particles.
{
	READ_ZONE();
	bool ret = false;

	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		if (mEffects[i]->getType() == ET_EMITTER)
		{
			Actor* a = mEffects[i]->getEffectActor();
			if (a)
			{
				EmitterActor* ae = static_cast< EmitterActor*>(a);
				if (ae->isEmitting())
				{
					ret = true;
					break;
				}
			}
		}
	}
	return ret;
}

// Effect class implementations

EffectEmitter::EffectEmitter(const char* parentName,
                             const EmitterEffect* data,
                             ApexSDK& sdk,
                             Scene& scene,
                             ParticlesScene& dscene,
                             const PxTransform& rootPose,
                             bool parentEnabled) : mData(data), EffectData(ET_EMITTER, &sdk,
	                                     &scene,
	                                     &dscene,
	                                     parentName,
	                                     EMITTER_AUTHORING_TYPE_NAME,
	                                     *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
	mEmitterVelocity = PxVec3(0, 0, 0);
	mFirstVelocityFrame = true;
	mHaveSetPosition = false;
	mVelocityTime = 0;
	mLastEmitterPosition = PxVec3(0, 0, 0);
}

EffectEmitter::~EffectEmitter(void)
{
}

EffectTurbulenceFS::EffectTurbulenceFS(const char* parentName,
                                       TurbulenceFieldSamplerEffect* data,
                                       ApexSDK& sdk,
                                       Scene& scene,
                                       ParticlesScene& dscene,
									   ModuleTurbulenceFS* moduleTurbulenceFS,
                                       const PxTransform& rootPose,
                                       bool parentEnabled) :
	mData(data),
	mModuleTurbulenceFS(moduleTurbulenceFS),
	EffectData(ET_TURBULENCE_FS, &sdk, &scene, &dscene, parentName, TURBULENCE_FS_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectTurbulenceFS::~EffectTurbulenceFS(void)
{
}

EffectJetFS::EffectJetFS(const char* parentName,
                         JetFieldSamplerEffect* data,
                         ApexSDK& sdk,
                         Scene& scene,
                         ParticlesScene& dscene,
                         const PxTransform& rootPose,
                         bool parentEnabled) : mData(data), EffectData(ET_JET_FS, &sdk, &scene, &dscene, parentName, JET_FS_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectJetFS::~EffectJetFS(void)
{
}

EffectWindFS::EffectWindFS(const char* parentName,
	WindFieldSamplerEffect* data,
	ApexSDK& sdk,
	Scene& scene,
	ParticlesScene& dscene,
	const PxTransform& rootPose,
	bool parentEnabled) : mData(data), EffectData(ET_WIND_FS, &sdk, &scene, &dscene, parentName, WIND_FS_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectWindFS::~EffectWindFS(void)
{
}



EffectAttractorFS::EffectAttractorFS(const char* parentName,
                                     AttractorFieldSamplerEffect* data,
                                     ApexSDK& sdk,
                                     Scene& scene,
                                     ParticlesScene& dscene,
                                     const PxTransform& rootPose,
                                     bool parentEnabled) : mData(data), EffectData(ET_ATTRACTOR_FS, &sdk, &scene, &dscene, parentName, ATTRACTOR_FS_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectAttractorFS::~EffectAttractorFS(void)
{
}

void EffectEmitter::visualize(RenderDebugInterface* callback, bool solid) const
{
	RENDER_DEBUG_IFACE(callback)->debugText(mPose.p, mData->Emitter->name());
	RENDER_DEBUG_IFACE(callback)->debugAxes(PxMat44(mPose));

}



EffectHeatSource::EffectHeatSource(const char* parentName,
                                   HeatSourceEffect* data,
                                   ApexSDK& sdk,
                                   Scene& scene,
                                   ParticlesScene& dscene,
                                   ModuleTurbulenceFS* moduleTurbulenceFS,
                                   const PxTransform& rootPose,
                                   bool parentEnabled) : mData(data),
	mModuleTurbulenceFS(moduleTurbulenceFS),
	EffectData(ET_HEAT_SOURCE, &sdk, &scene, &dscene, parentName, HEAT_SOURCE_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectHeatSource::~EffectHeatSource(void)
{
}

void EffectHeatSource::visualize(RenderDebugInterface* callback, bool solid) const
{
}

bool EffectHeatSource::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				PxTransform pose(myPose);
				bool ok = NvParameterized::setParamTransform(*descParams, "initialPose", pose);
				PX_ASSERT(ok);
				ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				ret = true;
			}
			const NvParameterized::Interface* iface = mAsset->getAssetNvParameterized();
			const turbulencefs::HeatSourceAssetParams* hap = static_cast< const turbulencefs::HeatSourceAssetParams*>(iface);
			mAverageTemperature = hap->averageTemperature;
			mStandardDeviationTemperature = hap->stdTemperature;
		}
		else if (mActor)
		{
			HeatSourceActor* a = static_cast< HeatSourceActor*>(mActor);
			nvidia::apex::Shape *s = a->getShape();
			s->setPose(myPose);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}

EffectSubstanceSource::EffectSubstanceSource(const char* parentName,
								   SubstanceSourceEffect* data,
								   ApexSDK& sdk,
								   Scene& scene,
								   ParticlesScene& dscene,
								   ModuleTurbulenceFS* moduleTurbulenceFS,
								   const PxTransform& rootPose,
								   bool parentEnabled) : mData(data),
								   mModuleTurbulenceFS(moduleTurbulenceFS),
								   EffectData(ET_SUBSTANCE_SOURCE, &sdk, &scene, &dscene, parentName, SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectSubstanceSource::~EffectSubstanceSource(void)
{
}

void EffectSubstanceSource::visualize(RenderDebugInterface* callback, bool solid) const
{
}

bool EffectSubstanceSource::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				PxTransform pose(myPose);
				bool ok = NvParameterized::setParamTransform(*descParams, "initialPose", pose);
				PX_ASSERT(ok);
				ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				ret = true;
			}
			const NvParameterized::Interface* iface = mAsset->getAssetNvParameterized();
			const turbulencefs::SubstanceSourceAssetParams* hap = static_cast< const turbulencefs::SubstanceSourceAssetParams*>(iface);
			mAverageDensity = hap->averageDensity;
			mStandardDeviationDensity = hap->stdDensity;
		}
		else if (mActor)
		{
			SubstanceSourceActor* a = static_cast< SubstanceSourceActor*>(mActor);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
			nvidia::apex::Shape *s = a->getShape();
			s->setPose(myPose);
		}
	}
	else
	{
		if ( mActor )
		{
			ret = true;
			releaseActor();
		}
	}
	return ret;
}

EffectVelocitySource::EffectVelocitySource(const char* parentName,
	VelocitySourceEffect* data,
	ApexSDK& sdk,
	Scene& scene,
	ParticlesScene& dscene,
	ModuleTurbulenceFS* moduleTurbulenceFS,
	const PxTransform& rootPose,
	bool parentEnabled) : mData(data),
	mModuleTurbulenceFS(moduleTurbulenceFS),
	EffectData(ET_VELOCITY_SOURCE, &sdk, &scene, &dscene, parentName, VELOCITY_SOURCE_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectVelocitySource::~EffectVelocitySource(void)
{
}

void EffectVelocitySource::visualize(RenderDebugInterface* callback, bool solid) const
{
}

bool EffectVelocitySource::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				PxTransform pose(myPose);
				bool ok = NvParameterized::setParamTransform(*descParams, "initialPose", pose);
				PX_ASSERT(ok);
				ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				ret = true;
			}
			const NvParameterized::Interface* iface = mAsset->getAssetNvParameterized();
			const turbulencefs::VelocitySourceAssetParams* hap = static_cast< const turbulencefs::VelocitySourceAssetParams*>(iface);
			mAverageVelocity = hap->averageVelocity;
			mStandardDeviationVelocity = hap->stdVelocity;
		}
		else if (mActor)
		{
			VelocitySourceActor* a = static_cast< VelocitySourceActor*>(mActor);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
			nvidia::apex::Shape *s = a->getShape();
			s->setPose(myPose);
		}
	}
	else
	{
		if ( mActor )
		{
			ret = true;
			releaseActor();
		}
	}
	return ret;
}


EffectFlameEmitter::EffectFlameEmitter(const char* parentName,
	FlameEmitterEffect* data,
	ApexSDK& sdk,
	Scene& scene,
	ParticlesScene& dscene,
	ModuleTurbulenceFS* moduleTurbulenceFS,
	const PxTransform& rootPose,
	bool parentEnabled) : mData(data),
	mModuleTurbulenceFS(moduleTurbulenceFS),
	EffectData(ET_FLAME_EMITTER, &sdk, &scene, &dscene, parentName, FLAME_EMITTER_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectFlameEmitter::~EffectFlameEmitter(void)
{
}

void EffectFlameEmitter::visualize(RenderDebugInterface* callback, bool solid) const
{
}

bool EffectFlameEmitter::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				PxTransform pose(myPose);
				bool ok = NvParameterized::setParamTransform(*descParams, "initialPose", pose);
				PX_ASSERT(ok);
				ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				ret = true;
			}
		}
		else if (mActor)
		{
			FlameEmitterActor* a = static_cast< FlameEmitterActor*>(mActor);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
			a->setPose(myPose);
		}
	}
	else
	{
		if ( mActor )
		{
			ret = true;
			releaseActor();
		}
	}
	return ret;
}


void EffectTurbulenceFS::visualize(RenderDebugInterface* callback, bool solid) const
{

}

void EffectJetFS::visualize(RenderDebugInterface* callback, bool solid) const
{

}

void EffectWindFS::visualize(RenderDebugInterface* callback, bool solid) const
{

}

void EffectAttractorFS::visualize(RenderDebugInterface* callback, bool solid) const
{

}

void EffectEmitter::computeVelocity(float dtime)
{
	mVelocityTime += dtime;
	if (mFirstVelocityFrame)
	{
		if (mActor)
		{
			mFirstVelocityFrame = false;
			mVelocityTime = 0;
			EmitterActor* ea = static_cast< EmitterActor*>(mActor);
			mLastEmitterPosition = ea->getGlobalPose().getPosition();
		}
	}
	else if (mHaveSetPosition && mActor)
	{
		mHaveSetPosition = false;
		EmitterActor* ea = static_cast< EmitterActor*>(mActor);
		PxVec3 newPos = ea->getGlobalPose().getPosition();
		mEmitterVelocity = (newPos - mLastEmitterPosition) * (1.0f / mVelocityTime);
		mLastEmitterPosition = newPos;
		mVelocityTime = 0;
	}
}

bool EffectEmitter::refresh(const PxTransform& parent,
							bool parentEnabled,
							bool fromSetPose,
							RenderVolume* renderVolume,
							EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();

		mPose = parent * localPose;
		getSamplePoseSpline(mPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			mFirstVelocityFrame = true;
			mHaveSetPosition = false;
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				bool ok = NvParameterized::setParamTransform(*descParams, "initialPose", mPose);
				PX_UNUSED(ok);
				PX_ASSERT(ok);
				ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				ok = NvParameterized::setParamBool(*descParams, "emitAssetParticles", true);
				PX_ASSERT(ok);
				EmitterAsset* easset = static_cast< EmitterAsset*>(mAsset);
				EmitterActor* ea = mParticlesScene->getEmitterFromPool(easset);
				if (ea)
				{
					ea->setCurrentPose(mPose);
					ea->setCurrentScale(mObjectScale*getSampleScaleSpline());
					mActor = static_cast< Actor*>(ea);
				}
				else
				{
					mActor = mAsset->createApexActor(*descParams, *mApexScene);
				}
				if (mActor)
				{
					const NvParameterized::Interface* iface = mAsset->getAssetNvParameterized();
					const char* className = iface->className();
					if (nvidia::strcmp(className, "ApexEmitterAssetParameters") == 0)
					{						
						const emitter::ApexEmitterAssetParameters* ap = static_cast<const  emitter::ApexEmitterAssetParameters*>(iface);
						mRate = ap->rate;
						mLifetimeLow = ap->lifetimeLow;
						mLifetimeHigh = ap->lifetimeHigh;
						EmitterActor* ea = static_cast< EmitterActor*>(mActor);
						ea->setRate(mRate);
						ea->setLifetimeLow(mLifetimeLow);
						ea->setLifetimeHigh(mLifetimeHigh);
						ea->startEmit(false);
						ea->setPreferredRenderVolume(renderVolume);
						ea->setApexEmitterValidateCallback(callback);
					}
					ret = true;
				}
			}
		}
		else if (mActor)
		{
			EmitterActor* ea = static_cast< EmitterActor*>(mActor);
			mHaveSetPosition = true; // set semaphore for computing the velocity.
			ea->setCurrentPose(mPose);
			ea->setObjectScale(mObjectScale*getSampleScaleSpline());
			ea->setPreferredRenderVolume(renderVolume);
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}


bool EffectTurbulenceFS::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				bool ok = NvParameterized::setParamTransform(*descParams, "initialPose", myPose);
				PX_UNUSED(ok);
				PX_ASSERT(ok);
				ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				ret = true;
			}
		}
		else if (mActor)
		{
			TurbulenceFSActor* a = static_cast< TurbulenceFSActor*>(mActor);
			a->setPose(myPose);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}

bool EffectJetFS::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();

		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				bool ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				if (mActor)
				{
					JetFSActor* fs = static_cast< JetFSActor*>(mActor);
					if (fs)
					{
						fs->setCurrentPose(myPose);
						fs->setCurrentScale(mObjectScale*getSampleScaleSpline());
					}
					ret = true;
				}
			}
		}
		else if (mActor)
		{
			JetFSActor* a = static_cast< JetFSActor*>(mActor);
			a->setCurrentPose(myPose);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}

bool EffectWindFS::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				bool ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				if (mActor)
				{
					WindFSActor* fs = static_cast< WindFSActor*>(mActor);
					if (fs)
					{
						fs->setCurrentPose(myPose);
						fs->setCurrentScale(mObjectScale*getSampleScaleSpline());
					}
					ret = true;
				}
			}
		}
		else if (mActor)
		{
			WindFSActor* a = static_cast< WindFSActor*>(mActor);
			a->setCurrentPose(myPose);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}

bool EffectAttractorFS::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform initialPose = _getPose(mData->EffectProperties.Position.TranslateX*mObjectScale*getSampleScaleSpline(),
		                                   mData->EffectProperties.Position.TranslateY*mObjectScale*getSampleScaleSpline(),
		                                   mData->EffectProperties.Position.TranslateZ*mObjectScale*getSampleScaleSpline(),
		                                   mData->EffectProperties.Orientation.RotateX,
		                                   mData->EffectProperties.Orientation.RotateY,
		                                   mData->EffectProperties.Orientation.RotateZ);

		PxTransform myPose(parent * initialPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				
				bool ok = NvParameterized::setParamTransform(*descParams, "initialPose", myPose);
				PX_UNUSED(ok);
				PX_ASSERT(ok);
				ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				ret = true;
			}
		}
		else if (mActor)
		{
			AttractorFSActor* a = static_cast< AttractorFSActor*>(mActor);
			a->setCurrentPosition(myPose.p);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}


EffectForceField::EffectForceField(const char* parentName, ForceFieldEffect* data, ApexSDK& sdk, Scene& scene, ParticlesScene& dscene, const PxTransform& rootPose, bool parentEnabled) : mData(data), EffectData(ET_FORCE_FIELD, &sdk, &scene, &dscene, parentName, FORCEFIELD_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectForceField::~EffectForceField(void)
{
}

void EffectForceField::visualize(RenderDebugInterface* callback, bool solid) const
{

}

bool EffectForceField::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform initialPose = _getPose(mData->EffectProperties.Position.TranslateX*mObjectScale*getSampleScaleSpline(),
		                                   mData->EffectProperties.Position.TranslateY*mObjectScale*getSampleScaleSpline(),
		                                   mData->EffectProperties.Position.TranslateZ*mObjectScale*getSampleScaleSpline(),
		                                   mData->EffectProperties.Orientation.RotateX,
		                                   mData->EffectProperties.Orientation.RotateY,
		                                   mData->EffectProperties.Orientation.RotateZ);
		PxTransform myPose(parent * initialPose);
		getSamplePoseSpline(myPose);
		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				bool ok = NvParameterized::setParamF32(*descParams, "scale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				if (mActor)
				{
					ForceFieldActor* fs = static_cast< ForceFieldActor*>(mActor);
					if (fs)
					{
						fs->setPose(myPose);
						fs->setCurrentScale(mObjectScale*getSampleScaleSpline());
					}
					ret = true;
				}
			}
		}
		else if (mActor)
		{
			ForceFieldActor* a = static_cast< ForceFieldActor*>(mActor);
			a->setPose(myPose);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}

// Returns true if this is a type which has a named resource asset which needs to be resolved.  RigidBody effects do not; all of their properties are embedded
static bool isNamedResourceType(EffectType type)
{
	bool ret = true;

	if ( type == ET_RIGID_BODY )
	{
		ret = false;
	}

	return ret;
}

EffectData::EffectData(EffectType type,
                       ApexSDK* sdk,
                       Scene* scene,
                       ParticlesScene* dscene,
                       const char* assetName,
                       const char* nameSpace,
					   RigidBodyEffectNS::EffectProperties_Type &effectProperties)
{
	mEffectPath = NULL;
	mParentPath = NULL;
	mObjectScale = 1;

	mLocalPose = _getPose(effectProperties.Position.TranslateX,
		effectProperties.Position.TranslateY,
		effectProperties.Position.TranslateZ,
		effectProperties.Orientation.RotateX,
		effectProperties.Orientation.RotateY,
		effectProperties.Orientation.RotateZ);

	mEffectPath = PX_NEW(EffectPath);
	if ( !mEffectPath->init(effectProperties.Path) )
	{
		delete mEffectPath;
		mEffectPath = NULL;
	}

	mEnabled = effectProperties.Enable;
	mRandomDeviation = effectProperties.RandomizeRepeatTime;
	mForceRenableEmitter = false;
	mUseEmitterPool = dscene->getModuleParticles()->getUseEmitterPool();
	mFirstRate = true;
	mType = type;
	mState = ES_INITIAL_DELAY;
	mStateTime = getRandomTime(effectProperties.InitialDelayTime);
	mSimulationTime = 0;
	mStateCount = 0;
	mParticlesScene = dscene;
	mInitialDelayTime = effectProperties.InitialDelayTime;
	mDuration = effectProperties.Duration;
	mRepeatCount = effectProperties.RepeatCount;
	mRepeatDelay = effectProperties.RepeatDelay;
	mApexSDK = sdk;
	mApexScene = scene;
	mActor = NULL;
	mNameSpace = nameSpace;
	mAsset = NULL;
	if ( isNamedResourceType(mType) ) 
	{
		mAsset = (nvidia::apex::Asset*)mApexSDK->getNamedResourceProvider()->getResource(mNameSpace, assetName);
		if (mAsset)
		{
			if (mType == ET_EMITTER)
			{
				if (!mUseEmitterPool)
				{
					mApexSDK->getNamedResourceProvider()->setResource(mNameSpace, assetName, mAsset, true);
				}
			}
			else
			{
				mApexSDK->getNamedResourceProvider()->setResource(mNameSpace, assetName, mAsset, true);
			}
		}
	}
}

EffectData::~EffectData(void)
{
	releaseActor();

	delete mEffectPath;

	if (mAsset)
	{
		if (mType == ET_EMITTER)
		{
			if (!mUseEmitterPool)
			{
				mApexSDK->getNamedResourceProvider()->releaseResource(mNameSpace, mAsset->getName());
			}
		}
		else
		{
			mApexSDK->getNamedResourceProvider()->releaseResource(mNameSpace, mAsset->getName());
		}
	}
}

void EffectData::releaseActor(void)
{
	if (mActor)
	{
		if (mType == ET_EMITTER && mUseEmitterPool)
		{
			EmitterActor* ae = static_cast< EmitterActor*>(mActor);
			mParticlesScene->addToEmitterPool(ae);
		}
		else
		{
			mActor->release();
		}
		mActor = NULL;
	}
}

float EffectData::getRandomTime(float baseTime)
{
	float deviation = baseTime * mRandomDeviation;
	return ranf(baseTime - deviation, baseTime + deviation);
}

bool EffectData::simulate(float dtime, bool& reset)
{
	bool ret = false;

	if (!mEnabled)
	{
		return false;
	}

	switch (mState)
	{
		case ES_INITIAL_DELAY:
			mStateTime -= dtime;
			if (mStateTime <= 0)
			{
				mState = ES_ACTIVE;		// once past the initial delay, it is now active
				mStateTime = getRandomTime(mDuration);	// Time to the next state change...
				ret = true;	// set ret to true because the effect is alive
			}
			break;
		case ES_ACTIVE:
			ret = true;		// it's still active..
			mStateTime -= dtime;	// decrement delta time to next state change.
			mSimulationTime+=dtime;
			if (mStateTime <= 0)   // if it's time for a state change.
			{
				if (mDuration == 0)   // if the
				{
					if (mRepeatDelay > 0)   // if there is a delay until the time we repeate
					{
						mStateTime = getRandomTime(mRepeatDelay);	// set time until repeat delay
						mState = ES_REPEAT_DELAY; // change state to repeat delay
					}
					else
					{
						mStateTime = getRandomTime(mDuration); // if there is no repeat delay; just continue
						if (mRepeatCount > 1)
						{
							reset = true; // looped..
						}
					}
				}
				else
				{
					mStateCount++;	// increment the state change counter.
					if (mStateCount >= mRepeatCount && mRepeatCount != 9999)   // have we hit the total number repeat counts.
					{
						mState = ES_DONE; // then we are completely done; the actor is no longer alive
						ret = false;
					}
					else
					{
						if (mRepeatDelay > 0)   // is there a repeat delay?
						{
							mStateTime = getRandomTime(mRepeatDelay);
							mState = ES_REPEAT_DELAY;
						}
						else
						{
							mStateTime = getRandomTime(mDuration);
							reset = true;
						}
					}
				}
			}
			else
			{
				if ( mEffectPath && mDuration != 0 )
				{
					mEffectPath->computeSampleTime(mStateTime,mDuration);
				}
			}
			if ( mDuration == 0 && mEffectPath )
			{
				mEffectPath->computeSampleTime(mSimulationTime,mEffectPath->getPathDuration());
			}
			break;
		case ES_REPEAT_DELAY:
			mStateTime -= dtime;
			if (mStateTime < 0)
			{
				mState = ES_ACTIVE;
				mStateTime = getRandomTime(mDuration);
				reset = true;
				ret = true;
			}
			break;
		case ES_DONE:
			break;
		default:
			//PX_ASSERT(0);
			break;
	}

	return ret;
}

void EffectPackageActorImpl::updateParticles(float dtime)
{
	mSimTime = dtime;
	mCurrentLifeTime += mSimTime;

	// If there has been some state change on one of the emitters, then we rebuild the emitter list!
	if ( mRigidBodyChange )
	{
		mRigidBodyChange = false;
	}
}

float EffectPackageActorImpl::internalGetDuration(void)
{
	float duration = 1000;
	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		EffectData* ed = mEffects[i];
		if (ed->getDuration() < duration)
		{
			duration = ed->getDuration();
		}
	}
	return duration;
}

// applies velocity adjustment to this range
static float processVelocityAdjust(const particles::EmitterEffectNS::EmitterVelocityAdjust_Type& vprops,
                                  const PxVec3& velocity)
{
	
	float r = 1;
	float v = velocity.magnitude();	// compute the absolute magnitude of the current emitter velocity
	if (v <= vprops.VelocityLow)   // if the velocity is less than the minimum velocity adjustment range
	{
		r = vprops.LowValue;	// Use the 'low-value' ratio adjustment
	}
	else if (v >= vprops.VelocityHigh)   // If the velocity is greater than the high range
	{
		r = vprops.HighValue; // then clamp tot he high value adjustment
	}
	else
	{
		float ratio = 1;
		float diff = vprops.VelocityHigh - vprops.VelocityLow; // Compute the velocity differntial
		if (diff > 0)
		{
			ratio = 1.0f / diff;	// compute the inverse velocity differential
		}
		float l = (v - vprops.VelocityLow) * ratio;	// find out the velocity lerp rate
		r = (vprops.HighValue - vprops.LowValue) * l + vprops.LowValue;
	}
	return r;
}


void EffectPackageActorImpl::updatePoseAndBounds(bool screenCulling, bool znegative)
{
	if (!mEnabled)
	{
		for (uint32_t i = 0; i < mEffects.size(); i++)
		{
			EffectData* ed = mEffects[i];
			if (ed->getEffectActor())
			{
				ed->releaseActor();
			}
			else if ( ed->getType() == ET_RIGID_BODY )
			{
				EffectRigidBody *erb = static_cast< EffectRigidBody *>(ed);
				erb->releaseRigidBody();
			}
		}
		mAlive = false;
		return;
	}

	float ZCOMPARE = -1;
	if (znegative)
	{
		ZCOMPARE *= -1;
	}

	//
	bool prevVisible = mVisible;
	mVisible = true;  // default visibile state is based on whether or not this effect is enabled.
	float emitterRate = 1;
	mVisState = VS_ON_SCREEN; // default value
	if (mVisible)   // if it's considered visible/enabled then let's do the LOD cacluation
	{
		const PxMat44& viewMatrix = mScene->getViewMatrix();
		PxVec3 pos = viewMatrix.transform(mPose.p);
		float magnitudeSquared = pos.magnitudeSquared();
		if (mData->LODSettings.CullByDistance)   // if distance culling is enabled
		{

			// If the effect is past the maximum distance then mark it is no longer visible.
			if (magnitudeSquared > mData->LODSettings.FadeDistanceEnd * mData->LODSettings.FadeDistanceEnd)
			{
				mVisible = false;
				mVisState = VS_OFF_SCREEN;
			} // if the effect is within the fade range; then compute the lerp value for it along that range as 'emitterRate'
			else if (magnitudeSquared > mData->LODSettings.FadeDistanceEnd * mData->LODSettings.FadeDistanceBegin)
			{
				float distance = PxSqrt(magnitudeSquared);
				float delta = mData->LODSettings.FadeDistanceEnd - mData->LODSettings.FadeDistanceBegin;
				if (delta > 0)
				{
					emitterRate = 1.0f - ((distance - mData->LODSettings.FadeDistanceBegin) / delta);
				}
			}
		}
		// If it's still considered visible (i.e. in range) and off screen culling is enabled; let's test it's status on/off screen
		if (mVisible && mData->LODSettings.CullOffScreen && screenCulling)
		{
			if (magnitudeSquared < (mData->LODSettings.ScreenCullDistance * mData->LODSettings.ScreenCullDistance))
			{
				mVisState = VS_TOO_CLOSE;
			}
			else if (pos.z * ZCOMPARE > 0)
			{
				mVisState = VS_BEHIND_SCREEN;
			}
			else
			{
				const PxMat44& projMatrix = mScene->getProjMatrix();
				PxVec4 p(pos.x, pos.y, pos.z, 1);
				p = projMatrix.transform(p);
				float recipW = 1.0f / p.w;

				p.x = p.x * recipW;
				p.y = p.y * recipW;
				p.z = p.z * recipW;

				float smin = -1 - mData->LODSettings.ScreenCullSize;
				float smax = 1 + mData->LODSettings.ScreenCullSize;

				if (p.x >= smin && p.x <= smax && p.y >= smin && p.y <= smax)
				{
					mVisState = VS_ON_SCREEN;
				}
				else
				{
					mVisState = VS_OFF_SCREEN;
				}
			}
		}
	}
	if (mVisState == VS_ON_SCREEN || mVisState == VS_TOO_CLOSE)
	{
		mOffScreenTime = 0;
	}
	else
	{
		mOffScreenTime += mSimTime;
		if (mOffScreenTime > mData->LODSettings.OffScreenCullTime)
		{
			mVisible = false; // mark it as non-visible due to it being off sceen too long.
			mAlive = false;
		}
		else
		{
			mVisible = mEverVisible; // was it ever visible?
		}
	}

	if ( mEffectPath )
	{
		mEffectPath->computeSampleTime(mCurrentLifeTime,mData->Path.PathDuration);
	}

	if (mFirstFrame && !mVisible && screenCulling)
	{
		if (getDuration() != 0)
		{
			mEnabled = false;
			return;
		}
	}


	if (mVisible)
	{
		mEverVisible = true;
	}


	bool aliveState = mVisible;

	// do the fade in/fade out over time logic...
	if (mData->LODSettings.FadeOutRate > 0)   // If there is a fade in/out time.
	{
		if (aliveState)    // if the effect is considered alive/visible then attenuate the emitterRate based on that fade in time value
		{
			mFadeTime += mSimTime;
			if (mFadeTime < mData->LODSettings.FadeOutRate)
			{
				emitterRate = emitterRate * mFadeTime / mData->LODSettings.FadeOutRate;
			}
			else
			{
				mFadeTime = mData->LODSettings.FadeOutRate;
			}
		}
		else // if the effect is not visible then attenuate it based on the fade out time
		{
			mFadeTime -= mSimTime;
			if (mFadeTime > 0)
			{
				emitterRate = emitterRate * mFadeTime / mData->LODSettings.FadeOutRate;
				aliveState = true; // still alive because it hasn't finsihed fading out...
			}
			else
			{
				mFadeTime = 0;
			}
		}
	}

	if (mFadeIn)
	{
		mFadeInDuration += mSimTime;
		if (mFadeInDuration > mFadeInTime)
		{
			mFadeIn = false;
		}
		else
		{
			float fadeScale = (mFadeInDuration / mFadeInTime);
			emitterRate *= fadeScale;
		}
	}

	if (mFadeOut)
	{
		mFadeOutDuration += mSimTime;
		if (mFadeOutDuration > mFadeOutTime)
		{
			aliveState = mVisible = false;
		}
		else
		{
			float fadeScale = 1.0f - (mFadeOutDuration / mFadeOutTime);
			emitterRate *= fadeScale;
		}
	}

	if (mVisible)
	{
		mNotVisibleTime = 0;
	}
	else
	{
		mNotVisibleTime += mSimTime;
	}

	bool anyAlive = false;
	bool rigidBodyChange = false;

	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		bool alive = aliveState;
		EffectData* ed = mEffects[i];
		// only emitters can handle a 'repeat' count.
		// others have an initial delay
		bool reset = false;

		if (ed->getDuration() == 0)
		{
			reset = !prevVisible; // if it was not previously visible then force a reset to bring it back to life.
		}
		if (alive)
		{
			alive = ed->simulate(mSimTime, reset);
			if ( alive )
			{
				anyAlive = true;
			}
		}
		if (ed->isDead())   // if it's lifetime has completely expired kill it!
		{
			if (ed->getEffectActor())
			{
				ed->releaseActor();
			}
			else if ( ed->getType() == ET_RIGID_BODY )
			{
				EffectRigidBody *erb = static_cast< EffectRigidBody *>(ed);
				erb->releaseRigidBody();
				rigidBodyChange = true;
			}
		}
		else
		{
			switch (ed->getType())
			{
				case ET_EMITTER:
				{
					EffectEmitter* ee = static_cast< EffectEmitter*>(ed);
					ee->computeVelocity(mSimTime);
					EmitterActor* ea = static_cast< EmitterActor*>(ed->getEffectActor());
					// if there is already an emitter actor...
					if (ea)
					{
						if (alive)   // is it alive?
						{
							if ( ed->getForceRenableEmitterSemaphore() && !ea->isEmitting() )
							{
								reset = true;
							}
							if (reset)   // is it time to reset it's condition?
							{
								ea->startEmit(false);
							}
							if (mData->LODSettings.FadeEmitterRate || mData->LODSettings.RandomizeEmitterRate || mFadeOut || mFadeIn)
							{
								// attenuate the emitter rate range based on the previously computed LOD lerp value
								if (mData->LODSettings.RandomizeEmitterRate && ee->mFirstRate)
								{
									ee->mFirstRate = false;
								}
								if (mData->LODSettings.FadeEmitterRate ||
										mFadeOut ||
										mFadeIn ||
										ee->mData->EmitterVelocityChanges.AdjustEmitterRate.AdjustEnabled ||
										ee->mData->EmitterVelocityChanges.AdjustLifetime.AdjustEnabled)
								{
									float rate = ee->mRate * emitterRate;
									if (ee->mData->EmitterVelocityChanges.AdjustEmitterRate.AdjustEnabled)
									{
										rate *= processVelocityAdjust(ee->mData->EmitterVelocityChanges.AdjustEmitterRate, ee->mEmitterVelocity);
									}
									ea->setRate(rate);
									if (ee->mData->EmitterVelocityChanges.AdjustLifetime.AdjustEnabled)
									{
										float va = processVelocityAdjust(ee->mData->EmitterVelocityChanges.AdjustLifetime, ee->mEmitterVelocity);
										ea->setLifetimeLow(ee->mLifetimeLow * va);
										ea->setLifetimeLow(ee->mLifetimeHigh * va);
									}
								}
							}

							if ( ee->activePath() )
							{
								ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());

								PxTransform pose = mPose;
								ee->getSamplePoseSpline(pose);
								ea->setCurrentPose(pose);
							}

						}
						else
						{
							if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
							{
								if (ed->getEffectActor())
								{
									ed->releaseActor();
								}
							}
							else
							{
								if (ea->isEmitting())
								{
									ea->stopEmit(); // just stop emitting but don't destroy the actor.
								}
							}
						}
					}
					else
					{
						if (alive)   // if it is now alive but was not previously; start the initial instance.
						{
							ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
						}
					}
				}
				break;
			case ET_ATTRACTOR_FS:
			{
				EffectAttractorFS *ee = static_cast< EffectAttractorFS *>(ed);
				AttractorFSActor* ea = static_cast< AttractorFSActor*>(ed->getEffectActor());
				// if there is already an emitter actor...
				if (ea)
				{
					if (alive)   // is it alive?
					{
						if (reset)   // is it time to reset it's condition?
						{
							ea->setEnabled(true);
						}
						if (mData->LODSettings.FadeAttractorFieldStrength)
						{
							// TODO
						}
						if ( ee->activePath() )
						{
							PxTransform pose = mPose;
							ee->getSamplePoseSpline(pose);
							ea->setCurrentPosition(pose.p);
							ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
						}
					}
					else
					{
						if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
						{

							if (ed->getEffectActor())
							{
								ed->releaseActor();
							}
						}
						else
						{
							ea->setEnabled(false);
						}
					}
				}
				else
				{
					if (alive)   // if it is now alive but was not previously; start the initial instance.
					{
						ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
					}
				}
			}
			break;
			case ET_JET_FS:
			{
				EffectJetFS *ee = static_cast< EffectJetFS *>(ed);
				JetFSActor* ea = static_cast< JetFSActor*>(ed->getEffectActor());
				// if there is already an emitter actor...
				if (ea)
				{
					if (alive)   // is it alive?
					{
						if (reset)   // is it time to reset it's condition?
						{
							ea->setEnabled(true);
						}
						if (mData->LODSettings.FadeJetFieldStrength)
						{
							// TODO
						}
						if ( ee->activePath() )
						{
							PxTransform pose = mPose;
							ee->getSamplePoseSpline(pose);
							ea->setCurrentPose(pose);
							ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
						}
					}
					else
					{
						if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
						{
							if (ed->getEffectActor())
							{
								ed->releaseActor();
							}
						}
						else
						{
							ea->setEnabled(false);
						}
					}
				}
				else
				{
					if (alive)   // if it is now alive but was not previously; start the initial instance.
					{
						ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
					}
				}
			}
			break;
			case ET_RIGID_BODY:
				{
					EffectRigidBody *erb = static_cast< EffectRigidBody *>(ed);
					if ( alive )
					{
						if ( ed->refresh(mPose, true, reset, mRenderVolume,mEmitterValidateCallback) )
						{
							rigidBodyChange = true;
						}
					}
					else
					{
						if ( erb->mRigidDynamic )
						{
							erb->releaseRigidBody();
							rigidBodyChange = true;
						}
					}
				}
				break;
			case ET_WIND_FS:
				{
					EffectWindFS *ee = static_cast< EffectWindFS *>(ed);
					WindFSActor* ea = static_cast< WindFSActor*>(ed->getEffectActor());
					// if there is already an emitter actor...
					if (ea)
					{
						if (alive)   // is it alive?
						{
							if (reset)   // is it time to reset it's condition?
							{
								ea->setEnabled(true);
							}
							if ( ee->activePath() )
							{
								PxTransform pose = mPose;
								ee->getSamplePoseSpline(pose);
								ea->setCurrentPose(pose);
								ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
							}
						}
						else
						{
							if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
							{
								if (ed->getEffectActor())
								{
									ed->releaseActor();
								}
							}
							else
							{
								ea->setEnabled(false);
							}
						}
					}
					else
					{
						if (alive)   // if it is now alive but was not previously; start the initial instance.
						{
							ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
						}
					}
				}
				break;
			case ET_NOISE_FS:
			{
				EffectNoiseFS *ee = static_cast< EffectNoiseFS *>(ed);
				NoiseFSActor* ea = static_cast< NoiseFSActor*>(ed->getEffectActor());
				// if there is already an emitter actor...
				if (ea)
				{
					if (alive)   // is it alive?
					{
						if (reset)   // is it time to reset it's condition?
						{
							ea->setEnabled(true);
						}
						if ( ee->activePath() )
						{
							PxTransform pose = mPose;
							ee->getSamplePoseSpline(pose);
							ea->setCurrentPose(pose);
							ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
						}
					}
					else
					{
						if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
						{
							if (ed->getEffectActor())
							{
								ed->releaseActor();
							}
						}
						else
						{
							ea->setEnabled(false);
						}
					}
				}
				else
				{
					if (alive)   // if it is now alive but was not previously; start the initial instance.
					{
						ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
					}
				}
			}
			break;
			case ET_VORTEX_FS:
			{
				EffectVortexFS *ee = static_cast< EffectVortexFS *>(ed);
				VortexFSActor* ea = static_cast< VortexFSActor*>(ed->getEffectActor());
				// if there is already an emitter actor...
				if (ea)
				{
					if (alive)   // is it alive?
					{
						if (reset)   // is it time to reset it's condition?
						{
							ea->setEnabled(true);
						}
						if ( ee->activePath() )
						{
							PxTransform pose = mPose;
							ee->getSamplePoseSpline(pose);
							ea->setCurrentPose(pose);
							ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
						}
					}
					else
					{
						if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
						{
							if (ed->getEffectActor())
							{
								ed->releaseActor();
							}
						}
						else
						{
							ea->setEnabled(false);
						}
					}
				}
				else
				{
					if (alive)   // if it is now alive but was not previously; start the initial instance.
					{
						ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
					}
				}
			}
			break;
			case ET_TURBULENCE_FS:
			{
				EffectTurbulenceFS *ee = static_cast< EffectTurbulenceFS *>(ed);
				TurbulenceFSActor* ea = static_cast< TurbulenceFSActor*>(ed->getEffectActor());
				// if there is already an emitter actor...
				if (ea)
				{
					if (alive)   // is it alive?
					{
						if (reset)   // is it time to reset it's condition?
						{
							ea->setEnabled(true);
						}
						if (mData->LODSettings.FadeTurbulenceNoise)
						{
							// TODO
						}
						if ( ee->activePath() )
						{
							PxTransform pose = mPose;
							ee->getSamplePoseSpline(pose);
							ea->setPose(pose);
							ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
						}
					}
					else
					{
						if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
						{
							if (ed->getEffectActor())
							{
								ed->releaseActor();
							}
						}
						else
						{
							ea->setEnabled(false);
						}
					}
				}
				else
				{
					if (alive)   // if it is now alive but was not previously; start the initial instance.
					{
						ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
					}
				}
			}
			break;
			case ET_FORCE_FIELD:
			{
				EffectForceField *ee = static_cast< EffectForceField *>(ed);
				ForceFieldActor* ea = static_cast< ForceFieldActor*>(ed->getEffectActor());
				// if there is already an emitter actor...
				if (ea)
				{
					if (alive)   // is it alive?
					{
						if (reset)   // is it time to reset it's condition?
						{
							ea->enable();
						}
						if (mData->LODSettings.FadeForceFieldStrength)
						{
							// TODO
						}
						if ( ee->activePath() )
						{
							PxTransform pose = mPose;
							ee->getSamplePoseSpline(pose);
							ea->setPose(pose);
							ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
						}
					}
					else
					{
						if (mNotVisibleTime > mData->LODSettings.NonVisibleDeleteTime)   // if it's been non-visible for a long time; delete it, don't just disable it!
						{
							if (ed->getEffectActor())
							{
								ed->releaseActor();
							}
						}
						else
						{
							if (ea->isEnable())
							{
								ea->disable();
							}
						}
					}
				}
				else
				{
					if (alive)   // if it is now alive but was not previously; start the initial instance.
					{
						ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
					}
				}
			}
			break;
			case ET_HEAT_SOURCE:
			{
				EffectHeatSource* ee = static_cast< EffectHeatSource*>(ed);
				HeatSourceActor* ea = static_cast< HeatSourceActor*>(ed->getEffectActor());
				// if there is already an emitter actor...
				if (ea)
				{
					if (alive)   // is it alive?
					{
						if (reset)   // is it time to reset it's condition?
						{
							ea->setTemperature(ee->mAverageTemperature, ee->mStandardDeviationTemperature);
							//ea->enable();
							//TODO!
						}
						if (mData->LODSettings.FadeHeatSourceTemperature)
						{
							// TODO
						}
						if ( ee->activePath() )
						{
							PxTransform pose = mPose;
							ee->getSamplePoseSpline(pose);
							ea->setPose(pose);
							ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
						}
					}
					else
					{
						ed->releaseActor();
					}
				}
				else
				{
					if (alive)   // if it is now alive but was not previously; start the initial instance.
					{
						ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
					}
				}
			}
			break;
			case ET_SUBSTANCE_SOURCE:
				{
					EffectSubstanceSource* ee = static_cast< EffectSubstanceSource*>(ed);
					SubstanceSourceActor* ea = static_cast< SubstanceSourceActor*>(ed->getEffectActor());
					// if there is already an emitter actor...
					if (ea)
					{
						if (alive)   // is it alive?
						{
							if (reset)   // is it time to reset it's condition?
							{
								ea->setDensity(ee->mAverageDensity, ee->mStandardDeviationDensity);
								//ea->enable();
								//TODO!
							}
							if (mData->LODSettings.FadeHeatSourceTemperature)
							{
								// TODO
							}
							if ( ee->activePath() )
							{
								PxTransform pose = mPose;
								ee->getSamplePoseSpline(pose);
								nvidia::apex::Shape *shape = ea->getShape();
								shape->setPose(pose);
								ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
							}
						}
						else
						{
							ed->releaseActor();
						}
					}
					else
					{
						if (alive)   // if it is now alive but was not previously; start the initial instance.
						{
							ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
						}
					}
				}
				break;
			case ET_VELOCITY_SOURCE:
				{
					EffectVelocitySource* ee = static_cast< EffectVelocitySource*>(ed);
					VelocitySourceActor* ea = static_cast< VelocitySourceActor*>(ed->getEffectActor());
					// if there is already an emitter actor...
					if (ea)
					{
						if (alive)   // is it alive?
						{
							if (reset)   // is it time to reset it's condition?
							{
								ea->setVelocity(ee->mAverageVelocity, ee->mStandardDeviationVelocity);
								//ea->enable();
								//TODO!
							}
							if (mData->LODSettings.FadeHeatSourceTemperature)
							{
								// TODO
							}
							if ( ee->activePath() )
							{
								PxTransform pose = mPose;
								ee->getSamplePoseSpline(pose);
								ea->setPose(pose);
								ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
							}
						}
						else
						{
							ed->releaseActor();
						}
					}
					else
					{
						if (alive)   // if it is now alive but was not previously; start the initial instance.
						{
							ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
						}
					}
				}
				break;
			case ET_FLAME_EMITTER:
				{
					EffectFlameEmitter* ee = static_cast< EffectFlameEmitter*>(ed);
					FlameEmitterActor* ea = static_cast< FlameEmitterActor*>(ed->getEffectActor());
					// if there is already an emitter actor...
					if (ea)
					{
						if (alive)   // is it alive?
						{
							if (reset)   // is it time to reset it's condition?
							{
								ea->setEnabled(true);
							}
							if ( ee->activePath() )
							{
								PxTransform pose = mPose;
								ee->getSamplePoseSpline(pose);
								ea->setPose(pose);
								ea->setCurrentScale(ee->mObjectScale*ee->getSampleScaleSpline());
							}
						}
						else
						{
							ed->releaseActor();
						}
					}
					else
					{
						if (alive)   // if it is now alive but was not previously; start the initial instance.
						{
							ed->refresh(mPose, true, false, mRenderVolume,mEmitterValidateCallback);
						}
					}
				}
				break;
			default:
				PX_ALWAYS_ASSERT(); // effect type not handled!
				break;
			}
		}
	}
	if ( rigidBodyChange )
	{
		mRigidBodyChange = true;
	}
	mAlive = anyAlive;
	mFirstFrame = false;
}


/**
\brief Returns the name of the effect at this index.

\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
*/
const char* EffectPackageActorImpl::getEffectName(uint32_t effectIndex) const
{
	READ_ZONE();
	const char* ret = NULL;
	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];
		ret = ed->getEffectAsset()->getName();
	}
	return ret;
}

/**
\brief Returns true if this sub-effect is currently enabled.

\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
*/
bool EffectPackageActorImpl::isEffectEnabled(uint32_t effectIndex) const
{
	READ_ZONE();
	bool ret = false;
	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];
		ret = ed->isEnabled();
	}
	return ret;
}

/**
\brief Set's the enabled state of this sub-effect

\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
\param [in] state : Whether the effect should be enabled or not.
*/
bool EffectPackageActorImpl::setEffectEnabled(uint32_t effectIndex, bool state)
{
	WRITE_ZONE();
	bool ret = false;

	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];
		ed->setEnabled(state);
		if ( ed->getType() == ET_EMITTER )
		{
			ed->setForceRenableEmitter(state); // set the re-enable semaphore
		}
		ret = true;
	}

	return ret;
}

/**
\brief Returns the pose of this sub-effect; returns as a a bool the active state of this effect.

\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
\param [pose] : Contains the pose requested
\param [worldSpace] : Whether to return the pose in world-space or in parent-relative space.
*/
bool EffectPackageActorImpl::getEffectPose(uint32_t effectIndex, PxTransform& pose, bool worldSpace)
{
	READ_ZONE();
	bool ret = false;

	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];

		if (worldSpace)
		{
			pose = ed->getWorldPose();
		}
		else
		{
			pose = ed->getLocalPose();
		}
		ret = true;
	}

	return ret;
}

void EffectPackageActorImpl::setCurrentScale(float scale)
{
	WRITE_ZONE();
	mObjectScale = scale;
	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		EffectData* ed = mEffects[i];
		ed->setCurrentScale(mObjectScale,mEffectPath);
		ed->refresh(mPose, mEnabled, true, mRenderVolume,mEmitterValidateCallback);
	}
}

/**
\brief Sets the pose of this sub-effect; returns as a a bool the active state of this effect.

\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
\param [pose] : Contains the pose to be set
\param [worldSpace] : Whether to return the pose in world-space or in parent-relative space.
*/
bool EffectPackageActorImpl::setEffectPose(uint32_t effectIndex, const PxTransform& pose, bool worldSpace)
{
	WRITE_ZONE();
	bool ret = false;

	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];
		if (worldSpace)
		{
			PxTransform p = getPose(); // get root pose
			PxTransform i = p.getInverse();
			PxTransform l = i * pose;
			ed->setLocalPose(pose);		// change the local pose
			setPose(p);
		}
		else
		{
			ed->setLocalPose(pose);		// change the local pose
			PxTransform p = getPose();
			setPose(p);
		}
		ret = true;
	}
	return ret;
}

/**
\brief Returns the current lifetime of the particle.
*/
float EffectPackageActorImpl::getCurrentLife(void) const
{
	READ_ZONE();
	return mCurrentLifeTime;
}

float EffectData::getRealDuration(void) const
{
	float ret = 0;

	if (mDuration != 0 && mRepeatCount != 9999)   // if it's not an infinite lifespan...
	{
		ret = mInitialDelayTime + mRepeatCount * mDuration + mRepeatCount * mRepeatDelay;
	}

	return ret;
}

float EffectPackageActorImpl::getDuration(void) const
{
	READ_ZONE();
	float ret = 0;

	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		EffectData* ed = mEffects[i];
		if (ed->getType() == ET_EMITTER)   // if it's an emitter
		{
			float v = ed->getRealDuration();
			if (v > ret)
			{
				ret = v;
			}
		}
	}
	return ret;
}

void	EffectPackageActorImpl::setPreferredRenderVolume(RenderVolume* volume)
{
	WRITE_ZONE();
	mRenderVolume = volume;
	for (uint32_t i = 0; i < mEffects.size(); i++)
	{
		EffectData* ed = mEffects[i];
		if (ed->getType() == ET_EMITTER)   // if it's an emitter
		{
			EffectEmitter* ee = static_cast< EffectEmitter*>(ed);
			if (ee->getEffectActor())
			{
				EmitterActor* ea = static_cast< EmitterActor*>(ee->getEffectActor());
				ea->setPreferredRenderVolume(volume);
			}
		}
	}

}


EffectNoiseFS::EffectNoiseFS(const char* parentName,
                             NoiseFieldSamplerEffect* data,
                             ApexSDK& sdk,
                             Scene& scene,
                             ParticlesScene& dscene,
                             const PxTransform& rootPose,
                             bool parentEnabled) : mData(data), EffectData(ET_NOISE_FS, &sdk, &scene, &dscene, parentName, NOISE_FS_AUTHORING_TYPE_NAME,*(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties)  )
{
}

EffectNoiseFS::~EffectNoiseFS(void)
{
}

void EffectNoiseFS::visualize(RenderDebugInterface* callback, bool solid) const
{

}

bool EffectNoiseFS::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				bool ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				if (mActor)
				{
					NoiseFSActor* fs = static_cast< NoiseFSActor*>(mActor);
					if (fs)
					{
						fs->setCurrentPose(myPose);
						fs->setCurrentScale(mObjectScale*getSampleScaleSpline());
					}
					ret = true;
				}
			}
		}
		else if (mActor)
		{
			NoiseFSActor* a = static_cast< NoiseFSActor*>(mActor);
			a->setCurrentPose(myPose);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}

EffectVortexFS::EffectVortexFS(const char* parentName,
                               VortexFieldSamplerEffect* data,
                               ApexSDK& sdk,
                               Scene& scene,
                               ParticlesScene& dscene,
                               const PxTransform& rootPose,
                               bool parentEnabled) : mData(data), EffectData(ET_VORTEX_FS, &sdk, &scene, &dscene, parentName, VORTEX_FS_AUTHORING_TYPE_NAME, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
}

EffectVortexFS::~EffectVortexFS(void)
{
}

void EffectVortexFS::visualize(RenderDebugInterface* callback, bool solid) const
{

}

bool EffectVortexFS::refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;

	if (parentEnabled && mEnabled && mAsset)
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose(parent * localPose);
		getSamplePoseSpline(myPose);

		if (mActor == NULL && mAsset && !fromSetPose)
		{
			NvParameterized::Interface* descParams = mAsset->getDefaultActorDesc();
			if (descParams)
			{
				bool ok = NvParameterized::setParamF32(*descParams, "initialScale", mObjectScale*getSampleScaleSpline() );
				PX_ASSERT(ok);
				PX_UNUSED(ok);
				mActor = mAsset->createApexActor(*descParams, *mApexScene);
				if (mActor)
				{
					VortexFSActor* fs = static_cast< VortexFSActor*>(mActor);
					if (fs)
					{
						fs->setCurrentPose(myPose);
						fs->setCurrentScale(mObjectScale*getSampleScaleSpline());
					}
					ret = true;
				}
			}
		}
		else if (mActor)
		{
			VortexFSActor* a = static_cast< VortexFSActor*>(mActor);
			a->setCurrentPose(myPose);
			a->setCurrentScale(mObjectScale*getSampleScaleSpline());
		}
	}
	else
	{
		if ( mActor )
		{
			releaseActor();
			ret = true;
		}
	}
	return ret;
}

const char * EffectPackageActorImpl::hasVolumeRenderMaterial(uint32_t &index) const
{
	READ_ZONE();
	const char *ret = NULL;

	for (uint32_t i=0; i<mEffects.size(); i++)
	{
		EffectData *d = mEffects[i];
		if ( d->getType() == ET_TURBULENCE_FS )
		{
			const NvParameterized::Interface *iface = d->getAsset()->getAssetNvParameterized();
			const turbulencefs::TurbulenceFSAssetParams *ap = static_cast< const turbulencefs::TurbulenceFSAssetParams *>(iface);
			if ( ap->volumeRenderMaterialName )
			{
				if ( strlen(ap->volumeRenderMaterialName->name()) > 0 )
				{
					index = i;
					ret = ap->volumeRenderMaterialName->name();
					break;
				}
			}
		}
	}

	return ret;
}

EffectRigidBody::EffectRigidBody(const char* parentName,
	RigidBodyEffect* data,
	ApexSDK& sdk,
	Scene& scene,
	ParticlesScene& dscene,
	const PxTransform& rootPose,
	bool parentEnabled) : mData(data), EffectData(ET_RIGID_BODY, &sdk, &scene, &dscene, parentName, NULL, *(RigidBodyEffectNS::EffectProperties_Type *)(&data->EffectProperties))
{
	mRigidDynamic = NULL;
}

EffectRigidBody::~EffectRigidBody(void)
{
	releaseRigidBody();
}


void EffectRigidBody::visualize(RenderDebugInterface* callback, bool solid) const
{

}

bool EffectRigidBody::refresh(const PxTransform& parent,
							  bool parentEnabled,
							  bool fromSetPose,
							  RenderVolume* renderVolume,
							  EmitterActor::EmitterValidateCallback *callback)
{
	bool ret = false;
	SCOPED_PHYSX_LOCK_WRITE(mApexScene);
	if (parentEnabled && mEnabled )
	{
		PxTransform localPose = mLocalPose;
		localPose.p*=mObjectScale*getSampleScaleSpline();
		PxTransform myPose = parent * localPose;
		getSamplePoseSpline(myPose);

		if (mRigidDynamic == NULL && !fromSetPose)
		{
			PxScene * scene = mApexScene->getPhysXScene();
			PxPhysics &sdk = scene->getPhysics();
			PxMaterial * material = mParticlesScene->getModuleParticles()->getDefaultMaterial();
			RigidBodyEffect *rbe = static_cast< RigidBodyEffect *>(mData);
			physx::PxFilterData data = ApexResourceHelper::resolveCollisionGroup128(rbe->CollisionFilterDataName.buf);
			mRigidDynamic = sdk.createRigidDynamic(myPose);
			ret = true;
			if ( mRigidDynamic )
			{
				mRigidDynamic->setLinearVelocity(rbe->InitialLinearVelocity);
				mRigidDynamic->setAngularVelocity(rbe->InitialAngularVelocity);
				mRigidDynamic->setMass(rbe->Mass);
				mRigidDynamic->setLinearDamping(rbe->LinearDamping);
				mRigidDynamic->setAngularDamping(rbe->AngularDamping);
				mRigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC,!rbe->Dynamic);

				physx::PxShape *shape = NULL;
				if ( nvidia::strcmp(rbe->Type,"SPHERE") == 0 )
				{
					PxSphereGeometry sphere;
					sphere.radius = rbe->Extents.x*getSampleScaleSpline();
					shape = mRigidDynamic->createShape(sphere,*material);
					shape->setLocalPose(localPose);
				}
				else if ( nvidia::strcmp(rbe->Type,"CAPSULE") == 0 )
				{
					PxCapsuleGeometry capsule;
					capsule.radius = rbe->Extents.x*getSampleScaleSpline();
					capsule.halfHeight = rbe->Extents.y*getSampleScaleSpline();
					shape = mRigidDynamic->createShape(capsule,*material);
					shape->setLocalPose(localPose);
				}
				else if ( nvidia::strcmp(rbe->Type,"BOX") == 0 )
				{
					PxBoxGeometry box;
					box.halfExtents.x = rbe->Extents.x*0.5f*getSampleScaleSpline();
					box.halfExtents.y = rbe->Extents.y*0.5f*getSampleScaleSpline();
					box.halfExtents.z = rbe->Extents.z*0.5f*getSampleScaleSpline();
					shape = mRigidDynamic->createShape(box,*material);
					shape->setLocalPose(localPose);
				}
				else
				{
					PX_ALWAYS_ASSERT();
				}
				if ( shape )
				{
					// do stuff here...
					shape->setSimulationFilterData(data);
					shape->setQueryFilterData(data);
					shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE,true);
					shape->setMaterials(&material,1);
				}
				mRigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY,!rbe->Gravity);
				mRigidDynamic->setActorFlag(PxActorFlag::eDISABLE_SIMULATION,false);
				mRigidDynamic->setActorFlag(PxActorFlag::eVISUALIZATION,true);
				scene->addActor(*mRigidDynamic);
			}
		}
		else if (mRigidDynamic && fromSetPose )
		{
			if ( mRigidDynamic->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC )
			{
				mRigidDynamic->setKinematicTarget(myPose);
			}
			else
			{
				mRigidDynamic->setGlobalPose(myPose);
			}
		}
		if ( activePath() && mRigidDynamic )
		{
			// if we are sampling a spline curve to control the scale of the object..
			RigidBodyEffect *rbe = static_cast< RigidBodyEffect *>(mData);
			physx::PxShape *shape = NULL;
			mRigidDynamic->getShapes(&shape,1,0);
			if ( shape )
			{
				if ( nvidia::strcmp(rbe->Type,"SPHERE") == 0 )
				{
					PxSphereGeometry sphere;
					sphere.radius = rbe->Extents.x*getSampleScaleSpline();
					shape->setGeometry(sphere);
				}
				else if ( nvidia::strcmp(rbe->Type,"CAPSULE") == 0 )
				{
					PxCapsuleGeometry capsule;
					capsule.radius = rbe->Extents.x*getSampleScaleSpline();
					capsule.halfHeight = rbe->Extents.y*getSampleScaleSpline();
					shape->setGeometry(capsule);
				}
				else if ( nvidia::strcmp(rbe->Type,"BOX") == 0 )
				{
					PxBoxGeometry box;
					box.halfExtents.x = rbe->Extents.x*0.5f*getSampleScaleSpline();
					box.halfExtents.y = rbe->Extents.y*0.5f*getSampleScaleSpline();
					box.halfExtents.z = rbe->Extents.z*0.5f*getSampleScaleSpline();
					shape->setGeometry(box);
				}
			}
			if ( mRigidDynamic->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC )
			{
				mRigidDynamic->setKinematicTarget(myPose);
			}
			else
			{
				mRigidDynamic->setGlobalPose(myPose);
			}
		}
	}
	else
	{
		//releaseActor();
	}
	return ret;
}

PxRigidDynamic* EffectPackageActorImpl::getEffectRigidDynamic(uint32_t effectIndex) const
{
	READ_ZONE();
	PxRigidDynamic *ret = NULL;

	if (effectIndex < mEffects.size())
	{
		EffectData* ed = mEffects[effectIndex];
		if ( ed->getType() == ET_RIGID_BODY )
		{
			EffectRigidBody *erd = static_cast< EffectRigidBody *>(ed);
			ret = erd->mRigidDynamic;
		}
	}


	return ret;
}

void EffectRigidBody::releaseRigidBody(void)
{
	if ( mRigidDynamic )
	{
		SCOPED_PHYSX_LOCK_WRITE(mApexScene);
		mRigidDynamic->release();
		mRigidDynamic = NULL;
	}
}

EffectPath::EffectPath(void)
{
	mRotations = NULL;
	mPathSpline = NULL;
	mScaleSpline = NULL;
	mSampleScaleSpline = 1.0f;
	mSpeedSpline = NULL;
	mPathDuration = 1;
}

EffectPath::~EffectPath(void)
{
	delete mScaleSpline;
	delete mSpeedSpline;
	delete	mPathSpline;
	PX_FREE(mRotations);
}

bool EffectPath::init(RigidBodyEffectNS::EffectPath_Type &path)
{
	bool ret = path.Scale.arraySizes[0] > 2;

	mPathDuration = path.PathDuration;
	mMode = EM_LOOP;
	if ( nvidia::strcmp(path.PlaybackMode,"LOOP") == 0 )
	{
		mMode = EM_LOOP;
	}
	else if ( nvidia::strcmp(path.PlaybackMode,"PLAY_ONCE") == 0 )
	{
		mMode = EM_PLAY_ONCE;
	}
	else if ( nvidia::strcmp(path.PlaybackMode,"PING_PONG") == 0 )
	{
		mMode = EM_PING_PONG;
	}

	delete mScaleSpline;
	mScaleSpline = NULL;

	if ( path.Scale.arraySizes[0] == 2 )
	{
		if ( path.Scale.buf[0].y != 1 ||
			path.Scale.buf[1].y != 1 )
		{
			ret = true;
		}
	}
	else if ( path.Scale.arraySizes[0] > 2 )
	{
		ret = true;
	}

	if ( ret )
	{
		mScaleSpline = PX_NEW(Spline);
		int32_t scount = path.Scale.arraySizes[0];
		mScaleSpline->Reserve(scount);
		for (int32_t i=0; i<scount; i++)
		{
			float x = path.Scale.buf[i].x;
			float y = path.Scale.buf[i].y;
			mScaleSpline->AddNode(x,y);
		}
		mScaleSpline->ComputeSpline();
		uint32_t index;
		float t;
		mSampleScaleSpline = mScaleSpline->Evaluate(0,index,t);
	}


	bool hasSpeed = false;

	if ( path.Speed.arraySizes[0] == 2 )
	{
		if ( path.Speed.buf[0].y != 1 ||
			path.Speed.buf[1].y != 1 )
		{
			hasSpeed = true;
		}
	}
	else if ( path.Speed.arraySizes[0] > 2 )
	{
		hasSpeed = true;
	}

	delete mSpeedSpline;
	mSpeedSpline = NULL;

	if ( hasSpeed )
	{
		ret = true;

		Spline speed;
		int32_t scount = path.Speed.arraySizes[0];
		speed.Reserve(scount);
		for (int32_t i=0; i<scount; i++)
		{
			float x = path.Speed.buf[i].x;
			float y = path.Speed.buf[i].y;
			speed.AddNode(x,y);
		}
		speed.ComputeSpline();

		float distance = 0;
		uint32_t index;
		for (int32_t i=0; i<32; i++)
		{
			float t = (float)i/32.0f;
			float fraction;
			float dt = speed.Evaluate(t,index,fraction);
			distance+=dt;
		}
		float recipDistance = 1.0f / distance;
		mSpeedSpline = PX_NEW(Spline);
		mSpeedSpline->Reserve(32);
		distance = 0;
		for (int32_t i=0; i<32; i++)
		{
			float t = (float)i/32.0f;
			float fraction;
			float dt = speed.Evaluate(t,index,fraction);
			distance+=dt;
			float d = distance*recipDistance;
			mSpeedSpline->AddNode(t,d);
		}
		mSpeedSpline->ComputeSpline();
		float fraction;
		mSampleSpeedSpline = mSpeedSpline->Evaluate(0,index,fraction);
	}


	PX_FREE(mRotations);
	mRotations = NULL;
	delete mPathSpline;
	mPathSpline = NULL;

	if ( path.ControlPoints.arraySizes[0] >= 3 )
	{
		mPathSpline = PX_NEW(SplineCurve);
		int32_t count = path.ControlPoints.arraySizes[0];
		PxVec3Vector points;
		mRotationCount = (uint32_t)count-1;
		mRotations = (PxQuat *)PX_ALLOC(sizeof(PxQuat)*(count-1),"PathRotations");
		mPathRoot = path.ControlPoints.buf[0];
		for (int32_t i=1; i<count; i++)
		{
			const PxTransform &t = path.ControlPoints.buf[i];
			mRotations[i-1] = t.q;
			points.pushBack(t.p);
		}
		mPathSpline->setControlPoints(points);

		float fraction;
		uint32_t index;
		mSamplePoseSpline.p = mPathSpline->Evaluate(0,index,fraction);
		mSamplePoseSpline.q = PxQuat(PxIdentity);
		mSamplePoseSpline = mPathRoot * mSamplePoseSpline;
		ret = true;
	}



	return ret;
}

float EffectPath::sampleSpline(float x)
{
	if ( mScaleSpline )
	{
		uint32_t index;
		float fraction;
		mSampleScaleSpline = mScaleSpline->Evaluate(x,index,fraction);
		if ( mSampleScaleSpline < 0.001f )
		{
			mSampleScaleSpline = 0.001f;
		}
	}
	if ( mPathSpline )
	{
		uint32_t index;
		float fraction;
		if ( mSpeedSpline )
		{
			x = mSpeedSpline->Evaluate(x,index,fraction);
		}
		float duration = mPathSpline->GetLength();
		duration*=x;
		mSamplePoseSpline.p = mPathSpline->Evaluate(duration,index,fraction);
		PX_ASSERT( index < mRotationCount );
		PxQuat q0 = mRotations[index];
		uint32_t index2 = index+1;
		if ( index2 >= mRotationCount )
		{
			index2 = 0;
		}
		PxQuat q1 = mRotations[index2];

		PxQuat q = physx::shdfnd::slerp(fraction,q0,q1);

		mSamplePoseSpline.q = q;
		mSamplePoseSpline = mPathRoot * mSamplePoseSpline;
	}

	return mSampleScaleSpline;
}

void EffectPath::computeSampleTime(float ctime,float duration)
{
	float sampleTime=0;
	switch ( mMode )
	{
		case EM_PLAY_ONCE:
			if ( ctime >= duration )
			{
				sampleTime = 1;
			}
			else
			{
				sampleTime = ctime / duration;
			}
			break;
		case EM_LOOP:
			sampleTime = fmodf(ctime,duration) / duration;
			break;
		case EM_PING_PONG:
			sampleTime = fmodf(ctime,duration*2) / duration;
			if ( sampleTime > 1 )
			{
				sampleTime = 2.0f - sampleTime;
			}
			break;
		default:
			PX_ALWAYS_ASSERT();
			break;
	}
	sampleSpline(sampleTime);
}
} // end of particles namespace
} // end of nvidia namespace
