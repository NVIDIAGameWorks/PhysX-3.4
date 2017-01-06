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


#include "ScTriggerInteraction.h"
#include "ScBodySim.h"
#include "ScNPhaseCore.h"

using namespace physx;

Sc::TriggerInteraction::TriggerInteraction(ShapeSim& tShape, ShapeSim& oShape) :
											RbElementInteraction(tShape, oShape, InteractionType::eTRIGGER, InteractionFlag::eRB_ELEMENT | InteractionFlag::eFILTERABLE),
											mFlags(PROCESS_THIS_FRAME),
											mLastFrameHadContacts(false)
{
	// The PxPairFlags eNOTIFY_TOUCH_FOUND and eNOTIFY_TOUCH_LOST get stored and mixed up with internal flags. Make sure any breaking change gets noticed.
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_FOUND < PxPairFlag::eNOTIFY_TOUCH_LOST);
	PX_COMPILE_TIME_ASSERT((PAIR_FLAGS_MASK & PxPairFlag::eNOTIFY_TOUCH_FOUND) == PxPairFlag::eNOTIFY_TOUCH_FOUND);
	PX_COMPILE_TIME_ASSERT((PAIR_FLAGS_MASK & PxPairFlag::eNOTIFY_TOUCH_LOST) == PxPairFlag::eNOTIFY_TOUCH_LOST);
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_FOUND < 0xffff);
	PX_COMPILE_TIME_ASSERT(PxPairFlag::eNOTIFY_TOUCH_LOST < 0xffff);
	PX_COMPILE_TIME_ASSERT(LAST < 0xffff);

	bool active = registerInActors();
	getScene().registerInteraction(this, active);
	getScene().getNPhaseCore()->registerInteraction(this);

	PX_ASSERT(getShape0().getFlags() & PxShapeFlag::eTRIGGER_SHAPE);
	mTriggerCache.state = Gu::TRIGGER_DISJOINT;
}

Sc::TriggerInteraction::~TriggerInteraction()
{
	getScene().unregisterInteraction(this);
	getScene().getNPhaseCore()->unregisterInteraction(this);
	unregisterFromActors();
}


bool Sc::TriggerInteraction::isOneActorActive() const
{
	const BodySim* bodySim0 = getTriggerShape().getBodySim();
	if (bodySim0 && bodySim0->isActive())
	{
		PX_ASSERT(!bodySim0->isKinematic() || bodySim0->readInternalFlag(BodySim::BF_KINEMATIC_MOVED) || 
					bodySim0->readInternalFlag(BodySim::InternalFlags(BodySim::BF_KINEMATIC_SETTLING | BodySim::BF_KINEMATIC_SETTLING_2)));
		return true;
	}

	const BodySim* bodySim1 = getOtherShape().getBodySim();
	if (bodySim1 && bodySim1->isActive())
	{
		PX_ASSERT(!bodySim1->isKinematic() || bodySim1->readInternalFlag(BodySim::BF_KINEMATIC_MOVED) || 
			bodySim1->readInternalFlag(BodySim::InternalFlags(BodySim::BF_KINEMATIC_SETTLING | BodySim::BF_KINEMATIC_SETTLING_2)));
		return true;
	}

	return false;
}


//
// Some general information about triggers and sleeping
//
// The goal is to avoid running overlap tests if both objects are sleeping.
// This is an optimization for eNOTIFY_TOUCH_LOST events since the overlap state 
// can not change if both objects are sleeping. eNOTIFY_TOUCH_FOUND should be sent nonetheless.
// For this to work the following assumptions are made:
// - On creation or if the pose of an actor is set, the pair will always be checked.
// - If the scenario above does not apply, then a trigger pair can only be deactivated, if both actors are sleeping.
// - If an overlapping actor is activated/deactivated, the trigger interaction gets notified
//
bool Sc::TriggerInteraction::onActivate(void*)
{
	// IMPORTANT: this method can get called concurrently from multiple threads -> make sure shared resources
	//            are protected (note: there are none at the moment but it might change)

	if (!(readFlag(PROCESS_THIS_FRAME)))
	{
		if (isOneActorActive())
		{
			raiseInteractionFlag(InteractionFlag::eIS_ACTIVE);
			return true;
		}
		else
			return false;
	}
	else
	{
		raiseInteractionFlag(InteractionFlag::eIS_ACTIVE);
		return true;  // newly created trigger pairs should always test for overlap, no matter the sleep state
	}
}


bool Sc::TriggerInteraction::onDeactivate(PxU32)
{
	if (!readFlag(PROCESS_THIS_FRAME))
	{
		if (!isOneActorActive())
		{
			clearInteractionFlag(InteractionFlag::eIS_ACTIVE);
			return true;
		}
		else
			return false;
	}
	else
		return false;
}
