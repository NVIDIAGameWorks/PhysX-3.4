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


#ifndef PICKING_H
#define PICKING_H

#include "foundation/PxSimpleTypes.h"
#include "foundation/PxVec3.h"
#include "extensions/PxD6Joint.h"
#include "extensions/PxSphericalJoint.h"

#include "PhysXSample.h"
#include "RendererProjection.h"

#define USE_D6_JOINT_FOR_MOUSE 1			// PT: please keep it 0 for interactive tests where one needs to rotate objects
#define USE_SPHERICAL_JOINT_FOR_MOUSE 0
//#define VISUALIZE_PICKING_RAYS

namespace physx
{
	class PxActor;
	class PxDistanceJoint;
	class PxRigidDynamic;
	class PxScene;
	class PxPhysics;
}

namespace physx {

	struct PickingCommands
	{
		enum Enum
		{
			PICK_START, //Bound to mouse 1 down
			PICK_STOP,  //bound to mouse 1 up
			SCREEN_MOTION_CURSOR, //See DefaultMovementStrategy
			SCREEN_MOTION_CURSOR_DEPTH, //See DefaultMovementStrategy
		};
	};
	
	class Picking 
	{
	public:
													Picking(PhysXSample& frame);
													~Picking();

		PX_FORCE_INLINE	void						lazyPick() { pick(mMouseScreenX, mMouseScreenY); }
						bool						isPicked() const;
						bool						pick(int x, int y);
						void						moveCursor(PxI32 x, PxI32 y);
//						void						moveCursor(PxReal deltaDepth);

						void						computeCameraRay(PxVec3& orig, PxVec3& dir, PxI32 x, PxI32 y)	const;

		// returns picked actor
						PxActor*					letGo();
						void						tick();
						void						project(const physx::PxVec3& v, int& xi, int& yi, float& depth)	const;

#ifdef VISUALIZE_PICKING_RAYS
		struct Ray
		{
			PxVec3	origin;
			PxVec3	dir;
		};
		PX_FORCE_INLINE	const std::vector<Ray>&		getRays()	const	{ return mRays;	}
#endif

	private:
						void						grabActor(const PxVec3& worldImpact, const PxVec3& rayOrigin);
						void						moveActor(int x, int y);
						PxVec3						unProject( int x, int y, float depth)	const;

						PxActor*					mSelectedActor;

#if USE_D6_JOINT_FOR_MOUSE
						PxD6Joint*					mMouseJoint; // was PxDistanceJoint, PxSphericalJoint, PxD6Joint
#elif USE_SPHERICAL_JOINT_FOR_MOUSE
						PxSphericalJoint*			mMouseJoint;
#else
						PxDistanceJoint*			mMouseJoint;
#endif
						PxRigidDynamic*				mMouseActor;
						PxRigidDynamic*				mMouseActorToDelete;
						PxReal						mDistanceToPicked;
						int							mMouseScreenX, mMouseScreenY;
						PhysXSample&				mFrame;
#ifdef VISUALIZE_PICKING_RAYS
						std::vector<Ray>			mRays;
#endif
	};
}

#endif
