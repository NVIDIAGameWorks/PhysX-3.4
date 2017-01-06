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

#ifndef PT_PARTICLE_SYSTEM_FLAGS_H
#define PT_PARTICLE_SYSTEM_FLAGS_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "CmPhysXCommon.h"
#include "particles/PxParticleBaseFlag.h"

namespace physx
{

namespace Pt
{

/*
ParticleSystems related constants
*/
// Maximum number of particles per particle system
#define PT_PARTICLE_SYSTEM_PARTICLE_LIMIT 0xfffffffe

/*!
PxParticleBaseFlag extension.
*/
struct InternalParticleSystemFlag
{
	enum Enum
	{
		// flags need to go into the unused bits of PxParticleBaseFlag
		eSPH                               = (1 << 16),
		eDISABLE_POSITION_UPDATE_ON_CREATE = (1 << 17),
		eDISABLE_POSITION_UPDATE_ON_SETPOS = (1 << 18)
	};
};

struct InternalParticleFlag
{
	enum Enum
	{
		// constraint info
		eCONSTRAINT_0_VALID          = (1 << 0),
		eCONSTRAINT_1_VALID          = (1 << 1),
		eANY_CONSTRAINT_VALID        = (eCONSTRAINT_0_VALID | eCONSTRAINT_1_VALID),
		eCONSTRAINT_0_DYNAMIC        = (1 << 2),
		eCONSTRAINT_1_DYNAMIC        = (1 << 3),
		eALL_CONSTRAINT_MASK         = (eANY_CONSTRAINT_VALID | eCONSTRAINT_0_DYNAMIC | eCONSTRAINT_1_DYNAMIC),

		// static geometry cache: 00 (cache invalid), 11 (cache valid and refreshed), 01 (cache valid, but aged by one
		// step).
		eGEOM_CACHE_BIT_0            = (1 << 4),
		eGEOM_CACHE_BIT_1            = (1 << 5),
		eGEOM_CACHE_MASK             = (eGEOM_CACHE_BIT_0 | eGEOM_CACHE_BIT_1),

		// cuda update info
		eCUDA_NOTIFY_CREATE          = (1 << 6),
		eCUDA_NOTIFY_SET_POSITION    = (1 << 7),
		eCUDA_NOTIFY_POSITION_CHANGE = (eCUDA_NOTIFY_CREATE | eCUDA_NOTIFY_SET_POSITION)
	};
};

struct ParticleFlags
{
	PxU16 api; // this is PxParticleFlag
	PxU16 low; // this is InternalParticleFlag
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_PARTICLE_SYSTEM_FLAGS_H
