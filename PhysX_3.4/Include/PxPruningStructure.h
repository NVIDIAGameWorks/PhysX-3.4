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


#ifndef PX_PHYSICS_NX_PRUNING_STRUCTURE
#define PX_PHYSICS_NX_PRUNING_STRUCTURE
/** \addtogroup physics
@{ */

#include "PxPhysXConfig.h"
#include "common/PxBase.h"

#if !PX_DOXYGEN
namespace physx
{
#endif


/**
\brief A precomputed pruning structure to accelerate scene queries against newly added actors.

The pruning structure can be provided to #PxScene:: addActors() in which case it will get merged
directly into the scene query optimization AABB tree, thus leading to improved performance when
doing queries against the newly added actors. This applies to both static and dynamic actors.

\note PxPruningStructure objects can be added to a collection and get serialized.
\note Adding a PxPruningStructure object to a collection will also add the actors that were used to build the pruning structure.

\note PxPruningStructure must be released before its rigid actors.
\note PxRigidBody objects can be in one PxPruningStructure only.
\note Changing the bounds of PxRigidBody objects assigned to a pruning structure that has not been added to a scene yet will 
invalidate the pruning structure. Same happens if shape scene query flags change or shape gets removed from an actor.

@see PxScene::addActors PxCollection
*/	
class PxPruningStructure : public PxBase
{
public:
	/**
	\brief Release this object.
	*/
	virtual void				release() = 0;

	/**
	\brief Retrieve rigid actors in the pruning structure.

	You can retrieve the number of rigid actor pointers by calling #getNbRigidActors()

	\param[out] userBuffer The buffer to store the actor pointers.
	\param[in] bufferSize Size of provided user buffer.
	\param[in] startIndex Index of first actor pointer to be retrieved
	\return Number of rigid actor pointers written to the buffer.

	@see PxRigidActor
	*/
	virtual PxU32				getRigidActors(PxRigidActor** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const = 0;

	/**
	\brief Returns the number of rigid actors in the pruning structure.

	You can use #getRigidActors() to retrieve the rigid actor pointers.

	\return Number of rigid actors in the pruning structure.

	@see PxRigidActor
	*/
	virtual PxU32				getNbRigidActors() const = 0;

	virtual	const char*			getConcreteTypeName() const	{ return "PxPruningStructure";	}
protected:
	PX_INLINE					PxPruningStructure(PxType concreteType, PxBaseFlags baseFlags) : PxBase(concreteType, baseFlags) {}
	PX_INLINE					PxPruningStructure(PxBaseFlags baseFlags) : PxBase(baseFlags) {}
	virtual						~PxPruningStructure()	{}
	virtual		bool			isKindOf(const char* name)	const		{ return !::strcmp("PxPruningStructure", name) || PxBase::isKindOf(name); }
};


#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */ 
#endif // PX_PHYSICS_NX_PRUNING_STRUCTURE
