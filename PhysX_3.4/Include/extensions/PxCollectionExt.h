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


#ifndef PX_COLLECTION_EXT_H
#define PX_COLLECTION_EXT_H
/** \addtogroup extensions
@{
*/

#include "PxPhysXConfig.h"
#include "common/PxCollection.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

	class PxCollectionExt
	{
	public:
		/**
		\brief Removes and releases all object from a collection.
		
		The Collection itself is not released.

		If the releaseExclusiveShapes flag is not set to true, release() will not be called on exclusive shapes.

		It is assumed that the application holds a reference to each of the objects in the collection, with the exception of objects that are not releasable
		(PxBase::isReleasable()). In general, objects that violate this assumption need to be removed from the collection prior to calling releaseObjects.
		
		\note when a shape is created with PxRigidActor::createShape() or PxRigidActorExt::createExclusiveShape(), the only counted reference is held by the actor. 
		If such a shape and its actor are present in the collection, the reference count will be decremented once when the actor is released, and once when the 
		shape is released, resulting in undefined behavior. Shape reference counts can be incremented with PxShape::acquireReference().

		\param[in] collection to remove and release all object from.
		\param[in] releaseExclusiveShapes if this parameter is set to false, release() will not be called on exclusive shapes.
		*/
		static void	releaseObjects(PxCollection& collection, bool releaseExclusiveShapes = true);

		/**
		\brief Removes objects of a given type from a collection, potentially adding them to another collection.

		\param[in,out] collection Collection from which objects are removed
		\param[in] concreteType PxConcreteType of sdk objects that should be removed	
		\param[in,out] to Optional collection to which the removed objects are added

		@see PxCollection, PxConcreteType
		*/	
		static void remove(PxCollection& collection, PxType concreteType, PxCollection* to = NULL);


		/**
		\brief Collects all objects in PxPhysics that are shareable across multiple scenes.

		This function creates a new collection from all objects that are shareable across multiple 
		scenes. Instances of the following types are included: PxConvexMesh, PxTriangleMesh, 
		PxHeightField, PxShape, PxMaterial and PxClothFabric.

		This is a helper function to ease the creation of collections for serialization. 

		\param[in] physics The physics SDK instance from which objects are collected. See #PxPhysics
		\return Collection to which objects are added. See #PxCollection

		@see PxCollection, PxPhysics
		*/
		static  PxCollection*	createCollection(PxPhysics& physics);
	
		/**
		\brief Collects all objects from a PxScene.

		This function creates a new collection from all objects that where added to the specified 
		PxScene. Instances of the following types are included: PxActor, PxAggregate, 
		PxArticulation and PxJoint (other PxConstraint types are not included).
	
		This is a helper function to ease the creation of collections for serialization. 
		The function PxSerialization.complete() can be used to complete the collection with required objects prior to 
		serialization.

		\param[in] scene The PxScene instance from which objects are collected. See #PxScene
		\return Collection to which objects are added. See #PxCollection

		@see PxCollection, PxScene, PxSerialization.complete()
		*/
		static	PxCollection*	createCollection(PxScene& scene);
	};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
