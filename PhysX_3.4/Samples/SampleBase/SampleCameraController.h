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

#ifndef SAMPLE_CAMERA_CONTROLLER_H
#define SAMPLE_CAMERA_CONTROLLER_H

#include "SampleAllocator.h"
#include "RendererWindow.h"
#include <SampleUserInput.h>
#include "foundation/PxVec3.h"

namespace SampleFramework {
	class SamplePlatform;
}

class Camera;

class CameraController : public SampleAllocateable
{
	public:
	virtual ~CameraController() {}

	virtual void		onPointerInputEvent(const SampleFramework::InputEvent& ie, physx::PxU32 x, physx::PxU32 y, physx::PxReal dx, physx::PxReal dy, bool val) {}

	virtual void		onAnalogInputEvent(const SampleFramework::InputEvent& , float val) {}
	virtual void		onDigitalInputEvent(const SampleFramework::InputEvent& , bool val) {}

	virtual	void		update(Camera& camera, PxReal dtime)												{}
	virtual void		collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents)	{}
	virtual PxReal		getCameraSpeed() { return 0; }
};

class DefaultCameraController : public CameraController
{
	public:
						DefaultCameraController();
	virtual				~DefaultCameraController();

			void		init(const PxVec3& pos, const PxVec3& rot);
			void		init(const PxTransform& pose);
			void		setCameraSpeed(const PxReal speed) { mCameraSpeed = speed; }
			PxReal		getCameraSpeed() { return mCameraSpeed; }
			void		setMouseLookOnMouseButton(bool mouseLookOnMB) { mMouseLookOnMB = mouseLookOnMB; }
			void		setMouseSensitivity(PxReal mouseSensitivity) { mMouseSensitivity = mouseSensitivity; }

	// Implements CameraController
			void		onMouseDelta(PxI32 dx, PxI32 dy);

	virtual void		onAnalogInputEvent(const SampleFramework::InputEvent& , float val);
	virtual void		onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);
	virtual void		onPointerInputEvent(const SampleFramework::InputEvent& ie, physx::PxU32 x, physx::PxU32 y, physx::PxReal dx, physx::PxReal dy, bool val);

	virtual void		collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);

	virtual	void		update(Camera& camera, PxReal dtime);

	protected:
			PxVec3								mTargetEyePos;
			PxVec3								mTargetEyeRot;
			PxVec3								mEyePos;
			PxVec3								mEyeRot;

			bool								mMouseButtonDown;
			bool								mKeyFWDown;
			bool								mKeyBKDown;
			bool								mKeyRTDown;
			bool								mKeyLTDown;
			bool								mKeyUpDown;
			bool								mKeyDownDown;
			bool								mKeyShiftDown;
			PxReal								mCameraSpeed;
			PxReal								mCameraSpeedMultiplier;
			bool								mMouseLookOnMB;
			PxReal								mMouseSensitivity;
	};

#endif
