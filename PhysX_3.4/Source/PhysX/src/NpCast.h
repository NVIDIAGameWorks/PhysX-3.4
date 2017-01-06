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

#ifndef PX_PHYSICS_NP_CAST
#define PX_PHYSICS_NP_CAST

#include "PxPhysXConfig.h"
#include "NpScene.h"
#include "NpRigidDynamic.h"
#include "NpRigidStatic.h"
#include "NpArticulation.h"
#include "NpArticulationLink.h"
#include "NpArticulationJoint.h"
#include "NpAggregate.h"
#if PX_USE_PARTICLE_SYSTEM_API
	#include "NpParticleFluid.h"
#endif
#if PX_USE_CLOTH_API
	#include "NpCloth.h"
#endif

namespace physx
{
	// PT: Scb-to-Np casts

	PX_FORCE_INLINE const NpScene* getNpScene(const Scb::Scene* scene)
	{
		const size_t scbOffset = reinterpret_cast<size_t>(&(reinterpret_cast<NpScene*>(0)->getScene()));
		return reinterpret_cast<const NpScene*>(reinterpret_cast<const char*>(scene) - scbOffset);
	}

	PX_FORCE_INLINE const NpRigidDynamic* getNpRigidDynamic(const Scb::Body* scbBody)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpRigidDynamic*>(0)->getScbActorFast()));
		return reinterpret_cast<const NpRigidDynamic*>(reinterpret_cast<const char*>(scbBody) - offset);
	}

	PX_FORCE_INLINE const NpRigidStatic* getNpRigidStatic(const Scb::RigidStatic* scbRigidStatic)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpRigidStatic*>(0)->getScbActorFast()));
		return reinterpret_cast<const NpRigidStatic*>(reinterpret_cast<const char*>(scbRigidStatic) - offset);
	}

	PX_FORCE_INLINE const NpShape* getNpShape(const Scb::Shape* scbShape)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpShape*>(0)->getScbShape()));
		return reinterpret_cast<const NpShape*>(reinterpret_cast<const char*>(scbShape) - offset);
	}

	PX_FORCE_INLINE const NpArticulationLink* getNpArticulationLink(const Scb::Body* scbArticulationLink)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpArticulationLink*>(0)->getScbActorFast()));
		return reinterpret_cast<const NpArticulationLink*>(reinterpret_cast<const char*>(scbArticulationLink) - offset);
	}

	PX_FORCE_INLINE const NpArticulation* getNpArticulation(const Scb::Articulation* scbArticulation)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpArticulation*>(0)->getArticulation()));
		return reinterpret_cast<const NpArticulation*>(reinterpret_cast<const char*>(scbArticulation) - offset);
	}

	PX_FORCE_INLINE const NpArticulationJoint* getNpArticulationJoint(const Scb::ArticulationJoint* scbArticulationJoint)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpArticulationJoint*>(0)->getScbArticulationJoint()));
		return reinterpret_cast<const NpArticulationJoint*>(reinterpret_cast<const char*>(scbArticulationJoint) - offset);
	}

	PX_FORCE_INLINE const NpAggregate* getNpAggregate(const Scb::Aggregate* aggregate)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpAggregate*>(0)->getScbAggregate()));
		return reinterpret_cast<const NpAggregate*>(reinterpret_cast<const char*>(aggregate) - offset);
	}

#if PX_USE_PARTICLE_SYSTEM_API
	PX_FORCE_INLINE const NpParticleSystem* getNpParticleSystem(const Scb::ParticleSystem* scbParticleSystem)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpParticleSystem*>(0)->getScbParticleSystem()));
		return reinterpret_cast<const NpParticleSystem*>(reinterpret_cast<const char*>(scbParticleSystem) - offset);
	}

	PX_FORCE_INLINE const NpParticleFluid* getNpParticleFluid(const Scb::ParticleSystem* scbParticleSystem)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpParticleFluid*>(0)->getScbParticleSystem()));
		return reinterpret_cast<const NpParticleFluid*>(reinterpret_cast<const char*>(scbParticleSystem) - offset);
	}
#endif

#if PX_USE_CLOTH_API
	PX_FORCE_INLINE const NpCloth* getNpCloth(const Scb::Cloth* cloth)
	{
		const size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<NpCloth*>(0)->getScbCloth()));
		return reinterpret_cast<const NpCloth*>(reinterpret_cast<const char*>(cloth) - offset);
	}
#endif

}

#endif
