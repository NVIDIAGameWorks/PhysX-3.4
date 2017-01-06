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


#include "ScParticleBodyInteraction.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScParticleSystemSim.h"
#include "ScScene.h"
#include "PxsContext.h"
#include "ScParticleSystemCore.h"
#include "ScBodySim.h"
#include "ScNPhaseCore.h"

using namespace physx;


Sc::ParticleElementRbElementInteraction::ParticleElementRbElementInteraction(ParticlePacketShape &particleShape, ShapeSim& rbShape, ActorElementPair& actorElementPair, const PxU32 ccdPass) :
	ElementSimInteraction	(particleShape, rbShape, InteractionType::ePARTICLE_BODY, InteractionFlag::eFILTERABLE | InteractionFlag::eELEMENT_ELEMENT),
	mActorElementPair		(actorElementPair),
	mPacketShapeIndex		(PX_INVALID_PACKET_SHAPE_INDEX),
	mIsActiveForLowLevel	(false)
{
	registerInActors();

	getScene().getNPhaseCore()->registerInteraction(this);

	mPacketShapeIndex = getParticleShape().addPacketShapeInteraction(this);

	if (!isDisabled())
		activateForLowLevel(ccdPass);	// Collision with rigid body
}


Sc::ParticleElementRbElementInteraction::~ParticleElementRbElementInteraction()
{
	unregisterFromActors();
	getScene().getNPhaseCore()->unregisterInteraction(this);
}


void Sc::ParticleElementRbElementInteraction::destroy(bool isDyingRb, const PxU32 ccdPass)
{
	ParticlePacketShape& ps = getParticleShape();

	if (mIsActiveForLowLevel)
		deactivateForLowLevel(isDyingRb, ccdPass);

	const PxU16 idx = mPacketShapeIndex;
	ps.removePacketShapeInteraction(idx);
	if (idx < ps.getInteractionsCount())
		ps.getPacketShapeInteraction(idx)->setPacketShapeIndex(idx);
	mPacketShapeIndex = PX_INVALID_PACKET_SHAPE_INDEX;
}


bool Sc::ParticleElementRbElementInteraction::onActivate(void*)
{
	return false;
}


bool Sc::ParticleElementRbElementInteraction::onDeactivate(PxU32)
{
	return true;
}


void Sc::ParticleElementRbElementInteraction::activateForLowLevel(const PxU32 ccdPass)
{
	//update active cm count and update transform hash/mirroring
	getParticleShape().getParticleSystem().addInteraction(getParticleShape(), getRbShape(), ccdPass);
	mIsActiveForLowLevel = true;
}


void Sc::ParticleElementRbElementInteraction::deactivateForLowLevel(bool isDyingRb, const PxU32 ccdPass)
{
	//update active cm count and update transform hash/mirroring
	getParticleShape().getParticleSystem().removeInteraction(getParticleShape(), getRbShape(), isDyingRb, ccdPass);
	mIsActiveForLowLevel = false;
}



#endif	// PX_USE_PARTICLE_SYSTEM_API
