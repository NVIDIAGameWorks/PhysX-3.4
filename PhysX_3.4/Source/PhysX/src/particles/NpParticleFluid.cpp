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

#include "PxPhysXConfig.h"

#if PX_USE_PARTICLE_SYSTEM_API

#include "NpParticleFluid.h"
#include "ScbParticleSystem.h"
#include "NpWriteCheck.h"
#include "NpReadCheck.h"

using namespace physx;

NpParticleFluid::NpParticleFluid(PxU32 maxParticles, bool perParticleRestOffset)
: ParticleSystemTemplateClass(PxConcreteType::ePARTICLE_FLUID, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, PxActorType::ePARTICLE_FLUID, maxParticles, perParticleRestOffset)
{}

NpParticleFluid::~NpParticleFluid()
{
}

// PX_SERIALIZATION
NpParticleFluid* NpParticleFluid::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpParticleFluid* obj = new (address) NpParticleFluid(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpParticleFluid);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

PxParticleFluidReadData* NpParticleFluid::lockParticleFluidReadData(PxDataAccessFlags flags)
{
	return static_cast<PxParticleFluidReadData*>(lockParticleReadData(flags));
}

PxParticleFluidReadData* NpParticleFluid::lockParticleFluidReadData()
{
	return static_cast<PxParticleFluidReadData*>(lockParticleReadData());
}

void* NpParticleFluid::is(PxActorType::Enum type)
{
	if (type == PxActorType::ePARTICLE_FLUID)
		return reinterpret_cast<void*>(static_cast<PxParticleFluid*>(this));
	else
		return NULL;
}


const void* NpParticleFluid::is(PxActorType::Enum type) const
{
	if (type == PxActorType::ePARTICLE_FLUID)
		return reinterpret_cast<const void*>(static_cast<const PxParticleFluid*>(this));
	else
		return NULL;
}


PxReal NpParticleFluid::getStiffness() const
{
	NP_READ_CHECK(getNpScene());
	return getScbParticleSystem().getStiffness();
}


void NpParticleFluid::setStiffness(PxReal s)
{
	NP_WRITE_CHECK(getNpScene());		
	PX_CHECK_AND_RETURN(s > 0.0f,"Stiffness needs to be positive, PxParticleFluid::setStiffness() ignored.");
	getScbParticleSystem().setStiffness(s);
}


PxReal NpParticleFluid::getViscosity() const
{
	NP_READ_CHECK(getNpScene());
	return getScbParticleSystem().getViscosity();
}


void NpParticleFluid::setViscosity(PxReal v)
{
	NP_WRITE_CHECK(getNpScene());		
	PX_CHECK_AND_RETURN(v > 0.0f,"Viscosity needs to be positive, PxParticleFluid::setViscosity() ignored.");
	getScbParticleSystem().setViscosity(v);
}


PxReal NpParticleFluid::getRestParticleDistance() const
{
	NP_READ_CHECK(getNpScene());
	return getScbParticleSystem().getRestParticleDistance();
}

void NpParticleFluid::setRestParticleDistance(PxReal r)
{
	NP_WRITE_CHECK(getNpScene());		
	PX_CHECK_AND_RETURN(r > 0.0f,"RestParticleDistance needs to be positive, PxParticleFluid::setRestParticleDistance() ignored.");
	PX_CHECK_AND_RETURN(!getScbParticleSystem().getScParticleSystem().getSim(),"RestParticleDistance immutable when the particle system is part of a scene.");	
	getScbParticleSystem().setRestParticleDistance(r);
}

#endif // PX_USE_PARTICLE_SYSTEM_API
