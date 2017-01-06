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

#ifndef SAMPLE_CHARACTER_CLOTH_H
#define SAMPLE_CHARACTER_CLOTH_H

#include "PhysXSample.h"
#include "PxSimulationEventCallback.h"
#include "characterkinematic/PxController.h"
#include "characterkinematic/PxControllerBehavior.h"
#include "PxShape.h"

#include "SampleCharacterHelpers.h"
#include "SampleCharacterClothJump.h"
#include "SampleCharacterClothPlatform.h"

namespace physx
{
	class PxCapsuleController;
	class PxControllerManager;
}

	class SampleCharacterClothFlag;
	class SampleCharacterClothCameraController;
	class ControlledActor;

	class SampleCharacterCloth : public PhysXSample, public PxUserControllerHitReport, public PxControllerBehaviorCallback
	{
		public:
												SampleCharacterCloth(PhysXSampleApplication& app);
		virtual									~SampleCharacterCloth();

		///////////////////////////////////////////////////////////////////////////////

		// Implements SampleApplication
		virtual	void							onInit();
        virtual	void						    onInit(bool restart) { onInit(); }
		virtual	void							onShutdown();
		virtual void                            onSubstep(float dtime);
		virtual	void							onTickPreRender(float dtime);
		virtual	void							onTickPostRender(float dtime);
		virtual void							onPointerInputEvent(const SampleFramework::InputEvent& ie, physx::PxU32 x, physx::PxU32 y, physx::PxReal dx, physx::PxReal dy, bool val);
		virtual void							onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);
		
		///////////////////////////////////////////////////////////////////////////////

		// Implements PhysXSampleApplication
		virtual	void							helpRender(PxU32 x, PxU32 y, PxU8 textAlpha);
		virtual	void							descriptionRender(PxU32 x, PxU32 y, PxU8 textAlpha);
		virtual	void							customizeRender();				
		virtual	void							customizeSample(SampleSetup&);
		virtual	void							customizeSceneDesc(PxSceneDesc&);	
		virtual void							collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);

		///////////////////////////////////////////////////////////////////////////////

		// Implements PxUserControllerHitReport
		virtual void							onShapeHit(const PxControllerShapeHit& hit);
		virtual void							onControllerHit(const PxControllersHit& hit);
		virtual void							onObstacleHit(const PxControllerObstacleHit& hit);

		// Implements PxControllerBehaviorCallback
		virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxShape&, const PxActor&);
		virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxController&);
		virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxObstacle&);

		///////////////////////////////////////////////////////////////////////////////
	protected:
		// non-dynamic environment
				PxRigidStatic*					buildHeightField();
				void							createLandscape(PxReal* heightmap, PxU32 size, PxReal scale, PxReal* outVerts, PxReal* outNorms, PxU32* outTris);
				PxRigidStatic*					createHeightField(PxReal* heightmap, PxReal scale, PxU32 size);
				SampleCharacterClothPlatform*	createPlatform(const PxTransform &pose, const PxGeometry &geom, PxReal travelTime, const PxVec3 &offset, RenderMaterial *renderMaterial);
				void							createPlatforms();
				void                            updatePlatforms(float dtime);

		// cct control
				void                            bufferCCTMotion(const PxVec3& disp, PxReal dtime);
				void                            createCCTController();
				void                            updateCCT(float dtime);

		// cloth and character
				void                            createCharacter();
				void                            createCape();
				void                            createFlags();
				void							releaseFlags();

				void                            resetCape();
				void                            resetCharacter();
				void							resetScene();
				
				void                            updateCape(float dtime);
				void                            updateCharacter(float dtime);
				void                            updateFlags(float dtime);

				void						    createRenderMaterials();
#if PX_SUPPORT_GPU_PHYSX || PX_XBOXONE
				void                            toggleGPU();
#endif

	private:
		///////////////////////////////////////////////////////////////////////////////
		// Landscape
				PxTransform						mHFPose;
				PxU32							mNbVerts;
				PxU32							mNbTris;
				PxReal*							mVerts;
				PxU32*							mTris;

		///////////////////////////////////////////////////////////////////////////////
		// Camera and Controller
				SampleCharacterClothCameraController*		mCCTCamera;
				PxCapsuleController*			mController;
				PxControllerManager*			mControllerManager;
				PxVec3							mControllerInitialPosition;
				bool                            mCCTActive;
				PxVec3                          mCCTDisplacement;
				PxVec3                          mCCTDisplacementPrevStep;
				PxReal                          mCCTTimeStep;
				SampleCharacterClothJump		mJump;
	
		///////////////////////////////////////////////////////////////////////////////
		// rendering
				RenderMaterial*					mSnowMaterial;
				RenderMaterial*					mPlatformMaterial;
				RenderMaterial*					mRockMaterial;
				RenderMaterial*					mWoodMaterial;
				RenderMaterial*					mFlagMaterial;

		///////////////////////////////////////////////////////////////////////////////
		// cloth
				PxCloth*                        mCape;
		SampleArray<SampleCharacterClothFlag*>  mFlags;
#if PX_SUPPORT_GPU_PHYSX || PX_XBOXONE
				bool                            mUseGPU;
#endif
				PxU32							mClothFlagCountIndex;
				PxU32							mClothFlagCountIndexMax;

		///////////////////////////////////////////////////////////////////////////////
		// character motion
				Character						mCharacter;
				Skin							mSkin;
				int								mMotionHandleWalk;
				int								mMotionHandleRun;
				int								mMotionHandleJump;

		///////////////////////////////////////////////////////////////////////////////
		// simulation stats
				shdfnd::Time					mTimer;
				PxReal							mAccumulatedSimulationTime;
				PxReal							mAverageSimulationTime;
				PxU32							mFrameCount;								

		///////////////////////////////////////////////////////////////////////////////
		// platform levels
		SampleArray<SampleCharacterClothPlatform*>		mPlatforms;

		friend class SampleCharacterClothCameraController;
		friend class SampleCharacterClothFlag;
	};

#endif
