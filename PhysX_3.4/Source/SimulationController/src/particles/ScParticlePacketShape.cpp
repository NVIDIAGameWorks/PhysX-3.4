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


#include "ScParticlePacketShape.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScParticleBodyInteraction.h"
#include "ScNPhaseCore.h"
#include "ScScene.h"
#include "PtParticleSystemSim.h"
#include "ScParticleSystemCore.h"
#include "ScSimStats.h"
#include "ScSqBoundsManager.h"
#include "ScScene.h"
#include "PxsContext.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ParticlePacketShape::ParticlePacketShape(ParticleSystemSim& particleSystem, PxU32 index, Pt::ParticleShape* llParticleShape) :
	ElementSim(particleSystem, ElementType::ePARTICLE_PACKET),
	mLLParticleShape(llParticleShape)
{
	// Initialize LL shape.
	PX_ASSERT(mLLParticleShape);
	mLLParticleShape->setUserDataV(this);

	setIndex(index);

	// Add particle actor element to broadphase
	createLowLevelVolume();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ParticlePacketShape::~ParticlePacketShape()
{
	getParticleSystem().unlinkParticleShape(this);		// Let the particle system remove this shape from the list.

	// Remove particle actor element from broadphase and cleanup interactions
	destroyLowLevelVolume();

	// Destroy LowLevel shape
	if (mLLParticleShape)
	{
		mLLParticleShape->destroyV();
		mLLParticleShape = 0;
	}

	PX_ASSERT(mInteractions.size()==0);
	mInteractions.releaseMem(*this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::computeWorldBounds(PxBounds3& b) const
{
	 b = getBounds();
	 PX_ASSERT(b.isFinite());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const
{
	filterAttr = 0;
	if (getParticleSystem().getInternalFlags() & Pt::InternalParticleSystemFlag::eSPH)
		ElementSim::setFilterObjectAttributeType(filterAttr, PxFilterObjectType::ePARTICLE_FLUID);
	else
		ElementSim::setFilterObjectAttributeType(filterAttr, PxFilterObjectType::ePARTICLE_SYSTEM);

	filterData = getParticleSystem().getSimulationFilterData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ParticleSystemSim& Sc::ParticlePacketShape::getParticleSystem() const
{ 
	return static_cast<ParticleSystemSim&>(getActor()); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::setInteractionsDirty(InteractionDirtyFlag::Enum flag)
{
	ParticleElementRbElementInteraction** interactions = getInteractions();
	PxU32 nbInteractions = getInteractionsCount();

	while(nbInteractions--)
	{
		ParticleElementRbElementInteraction* interaction = *interactions++;

		PX_ASSERT(interaction->readInteractionFlag(InteractionFlag::eFILTERABLE));
		PX_ASSERT(interaction->readInteractionFlag(InteractionFlag::eELEMENT_ELEMENT));

		interaction->setDirty(flag);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// PT: TODO: refactor with Sc::ActorSim::reallocInteractions
void Sc::ParticlePacketShape::reallocInteractions(Sc::ParticleElementRbElementInteraction**& mem, PxU16& capacity, PxU16 size, PxU16 requiredMinCapacity)
{
	ParticleElementRbElementInteraction** newMem;
	PxU16 newCapacity;

	if(requiredMinCapacity==0)
	{
		newCapacity = 0;
		newMem = 0;
	}
	else if(requiredMinCapacity<=INLINE_INTERACTION_CAPACITY)
	{
		newCapacity = INLINE_INTERACTION_CAPACITY;
		newMem = mInlineInteractionMem;
	}
	else
	{
		const PxU32 desiredCapacity = Ps::nextPowerOfTwo(PxU32(requiredMinCapacity-1));
		PX_ASSERT(desiredCapacity<=65536);

		const PxU32 limit = 0xffff;
		newCapacity = Ps::to16(PxMin(limit, desiredCapacity));
		newMem = reinterpret_cast<ParticleElementRbElementInteraction**>(getScene().allocatePointerBlock(newCapacity));
	}

	PX_ASSERT(newCapacity >= requiredMinCapacity && requiredMinCapacity>=size);

	PxMemCopy(newMem, mem, size*sizeof(ParticleElementRbElementInteraction*));

	if(mem && mem!=mInlineInteractionMem)
		getScene().deallocatePointerBlock(reinterpret_cast<void**>(mem), capacity);
	
	capacity = newCapacity;
	mem = newMem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::createLowLevelVolume()
{
	PX_ASSERT(getBounds().isFinite());

	getScene().getBoundsArray().setBounds(getBounds(), getElementID());
	addToAABBMgr(0, Bp::FilterGroup::ePARTICLES, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ParticlePacketShape::destroyLowLevelVolume()
{
	if (!isInBroadPhase())
		return;

	Sc::Scene& scene = getScene();
	PxsContactManagerOutputIterator outputs = scene.getLowLevelContext()->getNphaseImplementationContext()->getContactManagerOutputs();

	scene.getNPhaseCore()->onVolumeRemoved(this, 0, outputs, scene.getPublicFlags() & PxSceneFlag::eADAPTIVE_FORCE);
	removeFromAABBMgr();
}

#endif	// PX_USE_PARTICLE_SYSTEM_API
