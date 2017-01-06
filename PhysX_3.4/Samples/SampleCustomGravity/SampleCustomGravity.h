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


#ifndef SAMPLE_CUSTOM_GRAVITY_H
#define SAMPLE_CUSTOM_GRAVITY_H

#include "PhysXSample.h"
#include "characterkinematic/PxController.h" // for PxUserControllerHitReport
#include "PxShape.h"
#include "KinematicPlatform.h"

namespace physx
{
	class PxBoxController;
	class PxCapsuleController;
	class PxControllerManager;
}

class SampleCustomGravityCameraController;

//#define CONTACT_OFFSET			0.01f
#define CONTACT_OFFSET			0.1f
#define STEP_OFFSET				0.1f
#define SLOPE_LIMIT				0.0f
#define INVISIBLE_WALLS_HEIGHT	0.0f
#define MAX_JUMP_HEIGHT			0.0f
//#define USE_BOX_CONTROLLER

	struct Planet
	{
		PxVec3	mPos;
		float	mRadius;

		PX_FORCE_INLINE	void	getUpVector(PxVec3& up, const PxVec3& pos)	const
		{
			up = pos - mPos;
			up.normalize();
		}
	};

	PxMat33 fromTo(const PxVec3& from, const PxVec3& to);

	class SampleCustomGravity : public PhysXSample, public PxUserControllerHitReport
	{
		public:
												SampleCustomGravity(PhysXSampleApplication& app);
		virtual									~SampleCustomGravity();

		///////////////////////////////////////////////////////////////////////////////

		// Implements SampleApplication
		virtual	void							onInit();
        virtual	void						    onInit(bool restart) { onInit(); }
		virtual	void							onShutdown();
		virtual	void							onTickPreRender(PxReal dtime);
		virtual	void							onTickPostRender(PxF32 dtime);
		virtual void							onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);		
		virtual void							onPointerInputEvent(const SampleFramework::InputEvent& ie, physx::PxU32 x, physx::PxU32 y, physx::PxReal dx, physx::PxReal dy, bool val);
		virtual void							collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);

		///////////////////////////////////////////////////////////////////////////////

		// Implements PhysXSampleApplication
		virtual	void							helpRender(PxU32 x, PxU32 y, PxU8 textAlpha);
		virtual	void							descriptionRender(PxU32 x, PxU32 y, PxU8 textAlpha);
		virtual	void							customizeSample(SampleSetup&);
		virtual	void							customizeSceneDesc(PxSceneDesc&);
		virtual	void							onSubstep(float dtime);
		virtual	void							onDebugObjectCreation(PxRigidDynamic* actor);
		virtual PxU32							getDebugObjectTypes() const;
//		virtual	PxReal							getDebugObjectsVelocity()			const	{ return 0;}

		///////////////////////////////////////////////////////////////////////////////

		// Implements PxUserControllerHitReport
		virtual void							onShapeHit(const PxControllerShapeHit& hit);
		virtual void							onControllerHit(const PxControllersHit& hit)		{}
		virtual void							onObstacleHit(const PxControllerObstacleHit& hit)	{}

		///////////////////////////////////////////////////////////////////////////////

		// dynamics
				void							resetScene();

		///////////////////////////////////////////////////////////////////////////////

				void							buildScene();

		///////////////////////////////////////////////////////////////////////////////

				PxVec3							mTouchedTriangle[3];
				bool							mValidTouchedTriangle;

		///////////////////////////////////////////////////////////////////////////////

				std::vector<PxRigidDynamic*>	mDebugActors;
				KinematicPlatformManager		mPlatformManager;

				Planet							mPlanet;

		// Camera and Controller
				SampleCustomGravityCameraController*		mCCTCamera;
#ifdef USE_BOX_CONTROLLER
				PxBoxController*				mBoxController;
#else
				PxCapsuleController*			mCapsuleController;
#endif
				PxControllerManager*			mControllerManager;
				RenderBaseActor*				mRenderActor;
				PxExtendedVec3					mControllerInitialPosition;
				PxReal							mControllerRadius;
#ifdef USE_BOX_CONTROLLER
				PxBoxController*				createCharacter(const PxExtendedVec3& position);
#else
				PxCapsuleController*			createCharacter(const PxExtendedVec3& position);
#endif

		///////////////////////////////////////////////////////////////////////////////
		
		// rendering
				RenderMaterial*					mSnowMaterial;
				RenderMaterial*					mPlatformMaterial;
				bool							mDrawInvisibleWalls;
				bool							mEnableCCTDebugRender;

		// Platforms
				KinematicPlatform*				createPlatform(PxU32 nbPts, const PxVec3* pts, const PxQuat& localRot, const PxVec3& extents, PxF32 platformSpeed, PxF32 rotationSpeed);
	};

#endif
