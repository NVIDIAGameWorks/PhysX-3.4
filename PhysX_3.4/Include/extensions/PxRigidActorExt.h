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


#ifndef PX_PHYSICS_EXTENSIONS_RIGIDACTOR_H
#define PX_PHYSICS_EXTENSIONS_RIGIDACTOR_H
/** \addtogroup extensions
  @{
*/

#include "PxPhysXConfig.h"
#include "PxPhysics.h"
#include "PxRigidActor.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

/**
\brief utility functions for use with PxRigidActor and subclasses

@see PxRigidActor PxRigidStatic PxRigidBody PxRigidDynamic PxArticulationLink
*/

class PxRigidActorExt
{
public:

	/**
	\brief Creates a new shape with default properties and a list of materials and adds it to the list of shapes of this actor.
	
	This is equivalent to the following

	PxShape* shape(...) = PxGetPhysics().createShape(...);	// reference count is 1
	actor->attachShape(shape);								// increments reference count
	shape->release();										// releases user reference, leaving reference count at 1

	As a consequence, detachShape() will result in the release of the last reference, and the shape will be deleted.

	\note The default shape flags to be set are: eVISUALIZATION, eSIMULATION_SHAPE, eSCENE_QUERY_SHAPE (see #PxShapeFlag).
	Triangle mesh, heightfield or plane geometry shapes configured as eSIMULATION_SHAPE are not supported for 
	non-kinematic PxRigidDynamic instances.

	\note Creating compounds with a very large number of shapes may adversely affect performance and stability.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] actor the actor to which to attach the shape
	\param[in] geometry	the geometry of the shape
	\param[in] materials a pointer to an array of material pointers
	\param[in] materialCount the count of materials
	\param[in] shapeFlags optional PxShapeFlags

	\return The newly created shape.

	@see PxShape PxShape::release(), PxPhysics::createShape(), PxRigidActor::attachShape()
	*/

	static PxShape* createExclusiveShape(PxRigidActor& actor, const PxGeometry& geometry, PxMaterial*const* materials, PxU16 materialCount, 
								         PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE)
	{
		PxShape* shape = PxGetPhysics().createShape(geometry, materials, materialCount, true, shapeFlags);
		if(shape)
		{
			actor.attachShape(*shape);	// attach can fail, if e.g. we try and attach a trimesh simulation shape to a dynamic actor
			shape->release();		// if attach fails, we hold the only counted reference, and so this cleans up properly
		}
		return shape;
	}
	
	/**
	\brief Creates a new shape with default properties and a single material adds it to the list of shapes of this actor.

	This is equivalent to the following

	PxShape* shape(...) = PxGetPhysics().createShape(...);	// reference count is 1
	actor->attachShape(shape);								// increments reference count
	shape->release();										// releases user reference, leaving reference count at 1

	As a consequence, detachShape() will result in the release of the last reference, and the shape will be deleted.

	\note The default shape flags to be set are: eVISUALIZATION, eSIMULATION_SHAPE, eSCENE_QUERY_SHAPE (see #PxShapeFlag).
	Triangle mesh, heightfield or plane geometry shapes configured as eSIMULATION_SHAPE are not supported for 
	non-kinematic PxRigidDynamic instances.

	\note Creating compounds with a very large number of shapes may adversely affect performance and stability.

	<b>Sleeping:</b> Does <b>NOT</b> wake the actor up automatically.

	\param[in] actor the actor to which to attach the shape
	\param[in] geometry	the geometry of the shape
	\param[in] material	the material for the shape
	\param[in] shapeFlags optional PxShapeFlags

	\return The newly created shape.

	@see PxShape PxShape::release(), PxPhysics::createShape(), PxRigidActor::attachShape()
	*/

	static PX_FORCE_INLINE	PxShape*	createExclusiveShape(PxRigidActor& actor, const PxGeometry& geometry, const PxMaterial& material, 
													         PxShapeFlags shapeFlags = PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSCENE_QUERY_SHAPE | PxShapeFlag::eSIMULATION_SHAPE)
	{
		PxMaterial* materialPtr = const_cast<PxMaterial*>(&material);
		return createExclusiveShape(actor, geometry, &materialPtr, 1, shapeFlags);
	}

};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
