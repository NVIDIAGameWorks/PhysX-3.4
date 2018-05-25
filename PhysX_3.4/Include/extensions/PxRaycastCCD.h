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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PX_RAYCAST_CCD_H
#define PX_RAYCAST_CCD_H
/** \addtogroup extensions
@{
*/

#include "common/PxPhysXCommonConfig.h"
#include "foundation/PxVec3.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	class PxScene;
	class PxShape;
	class PxRigidDynamic;
	class RaycastCCDManagerInternal;

	/**
	\brief Raycast-CCD manager.

	Raycast-CCD is a simple and potentially cheaper alternative to the SDK's built-in continuous collision detection algorithm.

	This implementation has some limitations:
	- it is only implemented for PxRigidDynamic objects (not for PxArticulationLink)
	- it is only implemented for simple actors with 1 shape (not for "compounds")

	Also, since it is raycast-based, the solution is not perfect. In particular:
	- small dynamic objects can still go through the static world if the ray goes through a crack between edges, or a small
	hole in the world (like the keyhole from a door).
	- dynamic-vs-dynamic CCD is very approximate. It only works well for fast-moving dynamic objects colliding against
	slow-moving dynamic objects.

	Finally, since it is using the SDK's scene queries under the hood, it only works provided the simulation shapes also have
	scene-query shapes associated with them. That is, if the objects in the scene only use PxShapeFlag::eSIMULATION_SHAPE
	(and no PxShapeFlag::eSCENE_QUERY_SHAPE), then the raycast-CCD system will not work.
	*/
	class RaycastCCDManager
	{
		public:
					RaycastCCDManager(PxScene* scene);
					~RaycastCCDManager();

			/**
			\brief Register dynamic object for raycast CCD.

			\param[in] actor	object's actor
			\param[in] shape	object's shape

			\return True if success
			*/
			bool	registerRaycastCCDObject(PxRigidDynamic* actor, PxShape* shape);

			/**
			\brief Perform raycast CCD. Call this after your simulate/fetchResults calls.

			\param[in] doDynamicDynamicCCD	True to enable dynamic-vs-dynamic CCD (more expensive, not always needed)
			*/
			void	doRaycastCCD(bool doDynamicDynamicCCD);

		private:
			RaycastCCDManagerInternal*	mImpl;
	};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
