// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef SAMPLE_BRIDGES_H
#define SAMPLE_BRIDGES_H

#include "PhysXSample.h"
#include "PxSimulationEventCallback.h"
#include "SampleBridgesSettings.h"
#include "KinematicPlatform.h"

#ifdef CCT_ON_BRIDGES
	#include "characterkinematic/PxController.h"
	#include "characterkinematic/PxControllerBehavior.h"
#endif
#ifdef PLATFORMS_AS_OBSTACLES
	#include "characterkinematic/PxControllerObstacles.h"
#endif

namespace physx
{
	class PxJoint;
	class PxParticleBase;

#ifdef CCT_ON_BRIDGES
	class PxControllerManager;
#endif
}

#ifdef CCT_ON_BRIDGES
	class SampleCCTCameraController;
	class ControlledActor;
#endif

	class RenderParticleSystemActor;

	class SampleBridges : public PhysXSample
#ifdef CCT_ON_BRIDGES
		, public PxUserControllerHitReport, public PxControllerBehaviorCallback, public PxQueryFilterCallback
#endif
	{
		public:
												SampleBridges(PhysXSampleApplication& app);
		virtual									~SampleBridges();

		///////////////////////////////////////////////////////////////////////////////

		// Implements RAWImportCallback
		virtual	void							newMesh(const RAWMesh&);
		virtual	void							newShape(const RAWShape&);

		///////////////////////////////////////////////////////////////////////////////

		// Implements SampleApplication
		virtual	void							onInit();
        virtual	void						    onInit(bool restart) { onInit(); }
		virtual	void							onShutdown();
		virtual	void							onTickPreRender(float dtime);
		virtual void							onTickPostRender(float dtime);
		virtual void							onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);			

		///////////////////////////////////////////////////////////////////////////////

		// Implements PhysXSampleApplication
		virtual	void							helpRender(PxU32 x, PxU32 y, PxU8 textAlpha);
		virtual	void							descriptionRender(PxU32 x, PxU32 y, PxU8 textAlpha);
		virtual	void							customizeSample(SampleSetup&);
		virtual	void							customizeSceneDesc(PxSceneDesc&);
		virtual	void							customizeRender();
		virtual	void							onSubstep(float dtime);		
		virtual void							collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);
		virtual PxU32							getDebugObjectTypes() const;

		///////////////////////////////////////////////////////////////////////////////

#ifdef CCT_ON_BRIDGES
		// Implements PxUserControllerHitReport
		virtual void							onShapeHit(const PxControllerShapeHit& hit);
		virtual void							onControllerHit(const PxControllersHit& hit)		{}
		virtual void							onObstacleHit(const PxControllerObstacleHit& hit)	{}

		// Implements PxControllerBehaviorCallback
		virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxShape& shape, const PxActor& actor);
		virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxController& controller);
		virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxObstacle& obstacle);

		// Implements PxQueryFilterCallback
		virtual PxQueryHitType::Enum			preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags);
		virtual	PxQueryHitType::Enum			postFilter(const PxFilterData& filterData, const PxQueryHit& hit);
#endif

		private:
#ifdef CCT_ON_BRIDGES
				SampleCCTCameraController*		mCCTCamera;
				ControlledActor*				mActor;
				PxControllerManager*			mControllerManager;
				PxExtendedVec3					mControllerInitialPosition;
#endif
#ifdef PLATFORMS_AS_OBSTACLES
				PxObstacleContext*				mObstacleContext;
				PxBoxObstacle*					mBoxObstacles;
#endif
				PxReal							mGlobalScale;
				std::vector<PxJoint*>			mJoints;
				KinematicPlatformManager		mPlatformManager;
				PxU32							mCurrentPlatform;
				PxF32							mElapsedRenderTime;

				RenderMaterial*					mRockMaterial;
				RenderMaterial*					mWoodMaterial;
				RenderMaterial*					mPlatformMaterial;

				PxRigidActor*					mBomb;
				PxReal							mBombTimer;

				PxReal							mWaterY;
				bool							mMustResync;
				bool							mEnableCCTDebugRender;

#ifdef PLATFORMS_AS_OBSTACLES
				void							updateRenderPlatforms(float dtime);
#endif
	};

#endif
