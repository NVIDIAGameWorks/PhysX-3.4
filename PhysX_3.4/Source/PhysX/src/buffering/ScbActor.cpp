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

#include "ScbBase.h"

using namespace physx;
using namespace Scb;

#include "ScbActor.h"
#include "ScbRigidStatic.h"
#include "ScbBody.h"
#include "ScbParticleSystem.h"
#include "ScbCloth.h"

Actor::Offsets::Offsets()
{
	const size_t staticOffset	= reinterpret_cast<size_t>(&(reinterpret_cast<Scb::RigidStatic*>(0)->getScStatic()));
	const size_t bodyOffset		= reinterpret_cast<size_t>(&(reinterpret_cast<Scb::Body*>(0)->getScBody()));

	scToScb[PxActorType::eRIGID_STATIC] = staticOffset;
	scToScb[PxActorType::eRIGID_DYNAMIC] = bodyOffset;
	scToScb[PxActorType::eARTICULATION_LINK] = bodyOffset;

	scbToSc[ScbType::eRIGID_STATIC] = staticOffset;
	scbToSc[ScbType::eBODY] = bodyOffset;
	scbToSc[ScbType::eBODY_FROM_ARTICULATION_LINK] = bodyOffset;
	
#if PX_USE_PARTICLE_SYSTEM_API
	const size_t particleOffset = reinterpret_cast<size_t>(&(reinterpret_cast<Scb::ParticleSystem*>(0)->getScParticleSystem()));
	scToScb[PxActorType::ePARTICLE_FLUID] = particleOffset;
	scToScb[PxActorType::ePARTICLE_SYSTEM] = particleOffset;
	scbToSc[ScbType::ePARTICLE_SYSTEM] = particleOffset;
#endif

#if PX_USE_CLOTH_API
	const size_t clothOffset = reinterpret_cast<size_t>(&(reinterpret_cast<Scb::Cloth*>(0)->getScCloth()));
	scToScb[PxActorType::eCLOTH] = clothOffset;
	scbToSc[ScbType::eCLOTH] = clothOffset;
#endif
}

const Actor::Offsets Actor::sOffsets;
