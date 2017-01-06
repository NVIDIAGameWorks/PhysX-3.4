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


#include "NpParticleSystem.h"

#if PX_USE_PARTICLE_SYSTEM_API

#include "PsFoundation.h"
#include "NpPhysics.h"

#include "NpScene.h"
#include "NpWriteCheck.h"

#include "PsArray.h"

using namespace physx;

NpParticleSystem::NpParticleSystem(PxU32 maxParticles, bool perParticleRestOffset)
: ParticleSystemTemplateClass(PxConcreteType::ePARTICLE_SYSTEM, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, PxActorType::ePARTICLE_SYSTEM, maxParticles, perParticleRestOffset)
{}

NpParticleSystem::~NpParticleSystem()
{}

// PX_SERIALIZATION
NpParticleSystem* NpParticleSystem::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpParticleSystem* obj = new (address) NpParticleSystem(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpParticleSystem);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

void NpParticleSystem::setParticleReadDataFlag(PxParticleReadDataFlag::Enum flag, bool val)
{
	NP_WRITE_CHECK(getNpScene());		
	PX_CHECK_AND_RETURN( flag != PxParticleReadDataFlag::eDENSITY_BUFFER, 
		"ParticleSystem has unsupported PxParticleReadDataFlag::eDENSITY_BUFFER set.");	
	NpParticleSystemT::setParticleReadDataFlag(flag, val);
}
#endif
