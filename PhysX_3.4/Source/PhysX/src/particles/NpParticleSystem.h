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


#ifndef PX_PHYSICS_NP_PARTICLESYSTEM
#define PX_PHYSICS_NP_PARTICLESYSTEM

#include "PxPhysXConfig.h"

#if PX_USE_PARTICLE_SYSTEM_API

#include "particles/PxParticleSystem.h"
#include "NpParticleBaseTemplate.h"

#include "ScbParticleSystem.h"

// PX_SERIALIZATION
#include "PxSerialFramework.h"
//~PX_SERIALIZATION

namespace physx
{

class NpScene;
class NpParticleSystem;

namespace Scb
{
	class ParticleSystem;
}

typedef NpParticleBaseTemplate<PxParticleSystem, NpParticleSystem> NpParticleSystemT;
class NpParticleSystem : public NpParticleSystemT
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
									NpParticleSystem(PxBaseFlags baseFlags) : NpParticleSystemT(baseFlags) {}
	virtual		void				requires(PxProcessPxBaseCallback&) {}	
	virtual		void				exportExtraData(PxSerializationContext& stream) { mParticleSystem.exportExtraData(stream); }
				void				importExtraData(PxDeserializationContext& context) { mParticleSystem.importExtraData(context); }
	static		NpParticleSystem*	createObject(PxU8*& address, PxDeserializationContext& context);
	static		void				getBinaryMetaData(PxOutputStream& stream);

//~PX_SERIALIZATION
									NpParticleSystem(PxU32 maxParticles, bool perParticleRestOffset);
	virtual							~NpParticleSystem();

	virtual		void 				setParticleReadDataFlag(PxParticleReadDataFlag::Enum flag, bool val);

	//---------------------------------------------------------------------------------
	// PxParticleSystem implementation
	//---------------------------------------------------------------------------------
	virtual		PxActorType::Enum	getType() const { return PxActorType::ePARTICLE_SYSTEM; }

private:
	typedef NpParticleBaseTemplate<PxParticleSystem, NpParticleSystem> ParticleSystemTemplateClass;
};

}

#endif // PX_USE_PARTICLE_SYSTEM_API

#endif
