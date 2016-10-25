/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PARTICLES_EFFECT_PACKAGE_ACTOR_H

#define PARTICLES_EFFECT_PACKAGE_ACTOR_H

#include "ApexActor.h"
#include "EffectPackageActor.h"
#include "EffectPackageAssetImpl.h"
#include "ParticlesBase.h"
#include "Spline.h"
#include "PxTransform.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace nvidia
{
namespace apex
{
class Scene;
}
namespace turbulencefs
{
class ModuleTurbulenceFS;
}

namespace particles
{

enum VisState
{
	VS_TOO_CLOSE,
	VS_ON_SCREEN,
	VS_BEHIND_SCREEN,
	VS_OFF_SCREEN,
};

class EffectPath : public shdfnd::UserAllocated
{
public:
	enum Mode
	{
		EM_LOOP,
		EM_PLAY_ONCE,
		EM_PING_PONG
	};
	EffectPath(void);
	~EffectPath(void);

	bool init(RigidBodyEffectNS::EffectPath_Type &path);
	float getSampleScaleSpline(void) const { return mSampleScaleSpline; };
	float getSampleSpeedSpline(void) const { return mSampleSpeedSpline; };

	void getSamplePoseSpline(PxTransform &pose)
	{
		if ( mPathSpline )
		{
			pose = pose * mSamplePoseSpline;
		}
	}

	float sampleSpline(float stime);

	Mode getMode(void) const
	{
		return mMode;
	}

	void computeSampleTime(float ctime,float duration);

	float getPathDuration(void)
	{
		return mPathDuration;
	}

private:
	Mode					mMode;
	float					mPathDuration;
	Spline					*mScaleSpline;
	float					mSampleScaleSpline; // the scale value sampled from the spline curve
	float					mSampleSpeedSpline;
	Spline					*mSpeedSpline;
	PxTransform		mPathRoot;
	PxTransform		mSamplePoseSpline;
	uint32_t			mRotationCount;
	PxQuat			*mRotations;
	SplineCurve				*mPathSpline;
};

class EffectData : public shdfnd::UserAllocated
{
public:
	enum EffectState
	{
		ES_INITIAL_DELAY,
		ES_ACTIVE,
		ES_REPEAT_DELAY,
		ES_DONE
	};

	EffectData(EffectType type,
	           ApexSDK* sdk,
	           Scene* scene,
	           ParticlesScene* dscene,
	           const char* assetName,
	           const char* nameSpace,
			   RigidBodyEffectNS::EffectProperties_Type &effectProperties);

	virtual ~EffectData(void);
	void releaseActor(void);

	float getRandomTime(float baseTime);

	EffectType getType(void) const
	{
		return mType;
	}
	virtual void release(void) = 0;
	virtual void visualize(RenderDebugInterface* callback, bool solid) const = 0;
	virtual bool refresh(const PxTransform& parent,
						bool parentEnabled,
						bool fromSetPose,
						RenderVolume* renderVolume,
						EmitterActor::EmitterValidateCallback *callback) = 0;

	bool isDead(void) const
	{
		return mState == ES_DONE;
	}

	Actor* getEffectActor(void) const
	{
		return mActor;
	}
	Asset* getEffectAsset(void) const
	{
		return mAsset;
	}

	bool isEnabled(void) const
	{
		return mEnabled;
	}

	void setEnabled(bool state)
	{
		mEnabled = state;
	}

	bool simulate(float dtime, bool& reset);

	uint32_t getRepeatCount(void) const
	{
		return mRepeatCount;
	};
	float getDuration(void) const
	{
		return mDuration;
	};

	float getRealDuration(void) const;

	void setLocalPose(const PxTransform& p)
	{
		mLocalPose = p;
	}
	const PxTransform& getWorldPose(void) const
	{
		return mPose;
	};
	const PxTransform& getLocalPose(void) const
	{
		return mLocalPose;
	};

	Asset * getAsset(void) const { return mAsset; };

	void setForceRenableEmitter(bool state)
	{
		mForceRenableEmitter = state;
	}

	bool getForceRenableEmitterSemaphore(void)
	{
		bool ret = mForceRenableEmitter;
		mForceRenableEmitter = false;
		return ret;
	}

	void setCurrentScale(float objectScale,EffectPath *parentPath)
	{
		mParentPath = parentPath;
		mObjectScale = objectScale;
	}

	nvidia::apex::Scene *getApexScene(void) const
	{
		return mApexScene;
	}

	void getSamplePoseSpline(PxTransform &pose)
	{
		if ( mParentPath )
		{
			mParentPath->getSamplePoseSpline(pose);
		}
		if ( mEffectPath )
		{
			mEffectPath->getSamplePoseSpline(pose);
		}
	}

	float getSampleScaleSpline(void) const
	{
		float parentScale = mParentPath ? mParentPath->getSampleScaleSpline() : 1;
		float myScale = mEffectPath ? mEffectPath->getSampleScaleSpline() : 1;
		return myScale*parentScale;
	}

	bool activePath(void) const
	{
		bool ret = false;
		if ( mEffectPath || mParentPath )
		{
			ret = true;
		}
		return ret;
	}

	bool					mFirstRate: 1;
	float					mObjectScale;
	EffectPath				*mParentPath;
	EffectPath				*mEffectPath;

protected:
	bool					mUseEmitterPool: 1;
	bool					mEnabled: 1;
	bool					mForceRenableEmitter:1;
	EffectState				mState;
	float					mRandomDeviation;
	float					mSimulationTime;
	float					mStateTime;
	uint32_t					mStateCount;
	const char*				mNameSpace;
	ParticlesScene*			mParticlesScene;
	Scene*			mApexScene;
	ApexSDK*				mApexSDK;
	Asset*			mAsset;
	Actor*			mActor;
	float					mInitialDelayTime;
	float					mDuration;
	uint32_t					mRepeatCount;
	float					mRepeatDelay;
	EffectType				mType;
	PxTransform				mPose;				// world space pose
	PxTransform				mLocalPose;			// local space pose
};

class EffectForceField : public EffectData
{
public:
	EffectForceField(const char* parentName,
	                 ForceFieldEffect* data,
	                 ApexSDK& sdk,
	                 Scene& scene,
	                 ParticlesScene& dscene,
	                 const PxTransform& rootPose,
	                 bool parentEnabled);

	virtual ~EffectForceField(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPos, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	ForceFieldEffect*		mData;
};



class EffectEmitter : public EffectData
{
public:
	EffectEmitter(const char* parentName,
	              const EmitterEffect* data,
	              ApexSDK& sdk,
	              Scene& scene,
	              ParticlesScene& dscene,
	              const PxTransform& rootPose,
	              bool parentEnabled);

	virtual ~EffectEmitter(void);

	virtual void release(void)
	{
		delete this;
	}
	void computeVelocity(float dtime);
	virtual void visualize(RenderDebugInterface* callback, bool solid) const;

	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	float mRate;
	float mLifetimeLow;
	float mLifetimeHigh;
	const EmitterEffect*		mData;
	bool					mFirstVelocityFrame: 1;
	bool					mHaveSetPosition;
	PxVec3			mLastEmitterPosition;
	float			mVelocityTime;
	PxVec3			mEmitterVelocity;
};

class EffectHeatSource : public EffectData
{
public:
	EffectHeatSource(const char* parentName,
	                 HeatSourceEffect* data,
	                 ApexSDK& sdk,
	                 Scene& scene,
	                 ParticlesScene& dscene,
					 ModuleTurbulenceFS* moduleTurbulenceFS,
	                 const PxTransform& rootPose,
	                 bool parentEnabled);

	virtual ~EffectHeatSource(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);


	float			mAverageTemperature;
	float			mStandardDeviationTemperature;

	ModuleTurbulenceFS*	mModuleTurbulenceFS;
	HeatSourceEffect*		mData;
};

class EffectSubstanceSource : public EffectData
{
public:
	EffectSubstanceSource(const char* parentName,
		SubstanceSourceEffect* data,
		ApexSDK& sdk,
		Scene& scene,
		ParticlesScene& dscene,
		ModuleTurbulenceFS* moduleTurbulenceFS,
		const PxTransform& rootPose,
		bool parentEnabled);

	virtual ~EffectSubstanceSource(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);


	float			mAverageDensity;
	float			mStandardDeviationDensity;

	ModuleTurbulenceFS*	mModuleTurbulenceFS;
	SubstanceSourceEffect*		mData;
};

class EffectVelocitySource : public EffectData
{
public:
	EffectVelocitySource(const char* parentName,
		VelocitySourceEffect* data,
		ApexSDK& sdk,
		Scene& scene,
		ParticlesScene& dscene,
		ModuleTurbulenceFS* moduleTurbulenceFS,
		const PxTransform& rootPose,
		bool parentEnabled);

	virtual ~EffectVelocitySource(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);


	float			mAverageVelocity;
	float			mStandardDeviationVelocity;
	ModuleTurbulenceFS*	mModuleTurbulenceFS;
	VelocitySourceEffect*		mData;
};

class EffectFlameEmitter : public EffectData
{
public:
	EffectFlameEmitter(const char* parentName,
		FlameEmitterEffect* data,
		ApexSDK& sdk,
		Scene& scene,
		ParticlesScene& dscene,
		ModuleTurbulenceFS* moduleTurbulenceFS,
		const PxTransform& rootPose,
		bool parentEnabled);

	virtual ~EffectFlameEmitter(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);


	ModuleTurbulenceFS*	mModuleTurbulenceFS;
	FlameEmitterEffect*		mData;
};


class EffectTurbulenceFS : public EffectData
{
public:
	EffectTurbulenceFS(const char* parentName,
	                   TurbulenceFieldSamplerEffect* data,
	                   ApexSDK& sdk,
	                   Scene& scene,
	                   ParticlesScene& dscene,
					   ModuleTurbulenceFS* moduleTurbulenceFS,
	                   const PxTransform& rootPose,
	                   bool parentEnabled);

	virtual ~EffectTurbulenceFS(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	ModuleTurbulenceFS*	mModuleTurbulenceFS;
	TurbulenceFieldSamplerEffect*	mData;
};

class EffectJetFS : public EffectData
{
public:
	EffectJetFS(const char* parentName,
	            JetFieldSamplerEffect* data,
	            ApexSDK& sdk,
	            Scene& scene,
	            ParticlesScene& dscene,
	            const PxTransform& rootPose,
	            bool parentEnabled);
	virtual ~EffectJetFS(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	JetFieldSamplerEffect*	mData;
};


class EffectWindFS : public EffectData
{
public:
	EffectWindFS(const char* parentName,
		WindFieldSamplerEffect* data,
		ApexSDK& sdk,
		Scene& scene,
		ParticlesScene& dscene,
		const PxTransform& rootPose,
		bool parentEnabled);
	virtual ~EffectWindFS(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	WindFieldSamplerEffect*	mData;
};

class EffectRigidBody : public EffectData
{
public:
	EffectRigidBody(const char* parentName,
		RigidBodyEffect* data,
		ApexSDK& sdk,
		Scene& scene,
		ParticlesScene& dscene,
		const PxTransform& rootPose,
		bool parentEnabled);
	virtual ~EffectRigidBody(void);

	virtual void release(void)
	{
		delete this;
	}

	void releaseRigidBody(void);

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	RigidBodyEffect*	mData;
	PxRigidDynamic		*mRigidDynamic;
};

class EffectNoiseFS : public EffectData
{
public:
	EffectNoiseFS(const char* parentName,
	              NoiseFieldSamplerEffect* data,
	              ApexSDK& sdk,
	              Scene& scene,
	              ParticlesScene& dscene,
	              const PxTransform& rootPose,
	              bool parentEnabled);
	virtual ~EffectNoiseFS(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	NoiseFieldSamplerEffect*	mData;
};


class EffectVortexFS : public EffectData
{
public:
	EffectVortexFS(const char* parentName,
	               VortexFieldSamplerEffect* data,
	               ApexSDK& sdk,
	               Scene& scene,
	               ParticlesScene& dscene,
	               const PxTransform& rootPose,
	               bool parentEnabled);
	virtual ~EffectVortexFS(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	VortexFieldSamplerEffect*	mData;
};



class EffectAttractorFS : public EffectData
{
public:
	EffectAttractorFS(const char* parentName,
	                  AttractorFieldSamplerEffect* data,
	                  ApexSDK& sdk,
	                  Scene& scene,
	                  ParticlesScene& dscene,
	                  const PxTransform& rootPose,
	                  bool parentEnabled);

	virtual ~EffectAttractorFS(void);

	virtual void release(void)
	{
		delete this;
	}

	virtual void visualize(RenderDebugInterface* callback, bool solid) const;
	virtual bool refresh(const PxTransform& parent, bool parentEnabled, bool fromSetPose, RenderVolume* renderVolume,EmitterActor::EmitterValidateCallback *callback);

	AttractorFieldSamplerEffect*		mData;
};


class EffectPackageActorImpl : public EffectPackageActor, public shdfnd::UserAllocated, public ParticlesBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	EffectPackageActorImpl(EffectPackageAsset* asset,
	                   const EffectPackageAssetParams* assetParams,
	                   const EffectPackageActorParams* actorParams,
	                   nvidia::apex::ApexSDK& sdk,
	                   nvidia::apex::Scene& scene,
	                   ParticlesScene& dynamicSystemScene,
					   ModuleTurbulenceFS* moduleTurbulenceFS);

	virtual ~EffectPackageActorImpl(void);

	virtual ParticlesType getParticlesType(void) const
	{
		return ParticlesBase::DST_EFFECT_PACKAGE_ACTOR;
	}

	void updateParticles(float dtime);
	void updatePoseAndBounds(bool screenCulling, bool znegative);

	virtual void setPose(const PxTransform& pose);
	virtual const PxTransform& getPose(void) const;
	virtual void visualize(RenderDebugInterface* callback, bool solid) const;

	virtual void refresh(void);
	virtual void release(void);

	virtual const char* getName(void) const;

	virtual uint32_t getEffectCount(void) const; // returns the number of effects in the effect package
	virtual EffectType getEffectType(uint32_t effectIndex) const; // return the type of effect.
	virtual Actor* getEffectActor(uint32_t effectIndex) const; // return the base Actor pointer
	virtual void setEmitterState(bool state); // set the state for all emitters in this effect package.
	virtual uint32_t getActiveParticleCount(void) const; // return the total number of particles still active in this effect package.
	virtual bool isStillEmitting(void) const; // return true if any emitters are still actively emitting particles.


	/**
	\brief Returns the name of the effect at this index.

	\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
	*/
	virtual const char* getEffectName(uint32_t effectIndex) const;

	/**
	\brief Returns true if this sub-effect is currently enabled.

	\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
	*/
	virtual bool isEffectEnabled(uint32_t effectIndex) const;

	/**
	\brief Set's the enabled state of this sub-effect

	\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
	\param [in] state : Whether the effect should be enabled or not.
	*/
	virtual bool setEffectEnabled(uint32_t effectIndex, bool state);

	/**
	\brief Returns the pose of this sub-effect; returns as a a bool the active state of this effect.

	\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
	\param [pose] : Contains the pose requested
	\param [worldSpace] : Whether to return the pose in world-space or in parent-relative space.
	*/
	virtual bool getEffectPose(uint32_t effectIndex, PxTransform& pose, bool worldSpace);

	/**
	\brief Sets the pose of this sub-effect; returns as a a bool the active state of this effect.

	\param [in] effectIndex : The effect number to refer to; must be less than the result of getEffectCount
	\param [pose] : Contains the pose to be set
	\param [worldSpace] : Whether to return the pose in world-space or in parent-relative space.
	*/
	virtual bool setEffectPose(uint32_t effectIndex, const PxTransform& pose, bool worldSpace);

	virtual void setCurrentScale(float scale);

	virtual float getCurrentScale(void) const
	{
		return mObjectScale;
	}

	virtual PxRigidDynamic* getEffectRigidDynamic(uint32_t effectIndex) const;

	/**
	\brief Returns the current lifetime of the particle.
	*/
	virtual float getCurrentLife(void) const;


	virtual float getDuration(void) const;

	/**
	\brief Returns the owning asset
	*/
	virtual Asset* getOwner() const
	{
		READ_ZONE();
		return mAsset;
	}

	/**
	\brief Returns the range of possible values for physical Lod overwrite

	\param [out] min		The minimum lod value
	\param [out] max		The maximum lod value
	\param [out] intOnly	Only integers are allowed if this is true, gets rounded to nearest

	\note The max value can change with different graphical Lods
	\see Actor::forceLod()
	*/
	virtual void getLodRange(float& min, float& max, bool& intOnly) const
	{
		READ_ZONE();
		min = 0;
		max = 100000;
		intOnly = false;
	}

	/**
	\brief Get current physical lod.
	*/
	virtual float getActiveLod() const
	{
		READ_ZONE();
		return 0;
	}

	/**
	\brief Force an APEX Actor to use a certian physical Lod

	\param [in] lod	Overwrite the Lod system to use this Lod.

	\note Setting the lod value to a negative number will turn off the overwrite and proceed with regular Lod computations
	\see Actor::getLodRange()
	*/
	virtual void forceLod(float lod)
	{
		WRITE_ZONE();
		PX_UNUSED(lod);
	}

	/**
	\brief Selectively enables/disables debug visualization of a specific APEX actor.  Default value it true.
	*/
	virtual void setEnableDebugVisualization(bool state)
	{
		WRITE_ZONE();
		ApexActor::setEnableDebugVisualization(state);
	}

	/**
	\brief Ensure that all module-cached data is cached.
	*/
	virtual void cacheModuleData() const
	{

	}

	virtual void setEnabled(bool state)
	{
		WRITE_ZONE();
		mEnabled = state;
		refresh();
	}

	bool getEnabled(void) const
	{
		READ_ZONE();
		return mEnabled;
	}

	virtual void		setPhysXScene(PxScene* s)
	{
		mPhysXScene = s;
	}
	virtual PxScene*	getPhysXScene() const
	{
		return mPhysXScene;
	}

	float internalGetDuration(void);

	virtual bool isAlive(void) const
	{
		READ_ZONE();
		return mAlive;
	}

	virtual void fadeOut(float fadeTime)
	{
		WRITE_ZONE();
		if (!mFadeOut)
		{
			mFadeOutTime = fadeTime;
			mFadeOutDuration = 0;

			if (mFadeIn)
			{
				float fadeLerp = mFadeInDuration / mFadeInTime;
				if (fadeLerp > 1)
				{
					fadeLerp = 1;
				}
				mFadeOutDuration = 1 - (fadeLerp * mFadeOutTime);
				mFadeIn = false;
			}

			mFadeOut = true;
		}
	}

	virtual void fadeIn(float fadeTime)
	{
		WRITE_ZONE();
		if (!mFadeIn)
		{
			mFadeInTime = fadeTime;
			mFadeInDuration = 0;
			mFadeIn = true;
			if (mFadeOut)
			{
				float fadeLerp = mFadeOutDuration / mFadeOutTime;
				if (fadeLerp > 1)
				{
					fadeLerp = 1;
				}
				mFadeInDuration = 1 - (fadeLerp * mFadeInTime);
				mFadeOut = false;
			}
		}
	}

	virtual void                 setPreferredRenderVolume(RenderVolume* volume);

	virtual const char * hasVolumeRenderMaterial(uint32_t &index) const;

	virtual void setApexEmitterValidateCallback(EmitterActor::EmitterValidateCallback *callback)
	{
		WRITE_ZONE();
		mEmitterValidateCallback = callback;
	}

	float getSampleScaleSpline(void) const
	{
		return mEffectPath ? mEffectPath->getSampleScaleSpline() : 1;
	}

	void getSamplePoseSpline(PxTransform &pose)
	{
		if ( mEffectPath )
		{
			mEffectPath->getSamplePoseSpline(pose);
		}
	}

private:

	physx::PxScene*			mPhysXScene;

	EffectType getEffectType(const NvParameterized::Interface* iface);

	bool						mAlive:1;
	bool						mEnabled: 1;
	bool						mVisible: 1;
	bool						mEverVisible: 1;
	bool						mFirstFrame: 1;
	bool						mFadeOut: 1;
	float						mFadeOutTime;
	float						mFadeOutDuration;

	bool						mFadeIn: 1;
	float						mFadeInTime;
	float						mFadeInDuration;

	EmitterActor::EmitterValidateCallback *mEmitterValidateCallback;

	float						mFadeTime;
	float                       mNotVisibleTime;
	VisState					mVisState;
	float						mOffScreenTime;

	PxTransform					mPose;
	float						mObjectScale;
	const EffectPackageAssetParams*	mData;
	Array< EffectData* >		mEffects;
	float                       mSimTime;
	float						mCurrentLifeTime;
	EffectPath					*mEffectPath;

	nvidia::apex::Scene	*mScene;
	ModuleTurbulenceFS		*mModuleTurbulenceFS;
	EffectPackageAsset		*mAsset;
	RenderVolume			*mRenderVolume;
	bool						mRigidBodyChange;
};

} // end of particles namespace
} // end of nvidia namespace

#endif
