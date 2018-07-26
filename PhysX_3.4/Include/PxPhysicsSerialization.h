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


#ifndef PX_PHYSICS_PX_PHYSICS_SERIALIZATION
#define PX_PHYSICS_PX_PHYSICS_SERIALIZATION

#include "PxPhysXConfig.h"
#include "common/PxSerialFramework.h"

#if !PX_DOXYGEN
/**
\brief Retrieves the PhysX SDK metadata.
This function is used to implement PxSerialization.dumpBinaryMetaData() and is not intended to be needed otherwise.
@see PxSerialization.dumpBinaryMetaData()
*/
PX_C_EXPORT PX_PHYSX_CORE_API void PX_CALL_CONV PxGetPhysicsBinaryMetaData(physx::PxOutputStream& stream);

/**
\brief Registers physics classes for serialization.
This function is used to implement PxSerialization.createSerializationRegistry() and is not intended to be needed otherwise.
@see PxSerializationRegistry
*/
PX_C_EXPORT PX_PHYSX_CORE_API void PX_CALL_CONV PxRegisterPhysicsSerializers(physx::PxSerializationRegistry& sr);

/**
\brief Unregisters physics classes for serialization.
This function is used in the release implementation of PxSerializationRegistry and in not intended to be used otherwise.
@see PxSerializationRegistry
*/
PX_C_EXPORT PX_PHYSX_CORE_API void PX_CALL_CONV PxUnregisterPhysicsSerializers(physx::PxSerializationRegistry& sr);


/**
\brief Adds collected objects to PxPhysics.

This function adds all objects contained in the input collection to the PxPhysics instance. This is used after deserializing
the collection, to populate the physics with inplace deserialized objects. This function is used in the implementation of 
PxSerialization.createCollectionFromBinary and is not intended to be needed otherwise.
\param[in] collection Objects to add to the PxPhysics instance.

@see PxCollection, PxSerialization.createCollectionFromBinary
*/
PX_C_EXPORT PX_PHYSX_CORE_API void PX_CALL_CONV PxAddCollectionToPhysics(const physx::PxCollection& collection);

#endif // !PX_DOXYGEN

#endif // PX_PHYSICS_PX_PHYSICS_SERIALIZATION
