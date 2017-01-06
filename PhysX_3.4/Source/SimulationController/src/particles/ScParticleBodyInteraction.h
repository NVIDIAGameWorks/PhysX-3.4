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


#ifndef PX_PHYSICS_SCP_PARTICLEBODYINTERACTION
#define PX_PHYSICS_SCP_PARTICLEBODYINTERACTION

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScElementSimInteraction.h"
#include "ScActorElementPair.h"
#include "ScParticlePacketShape.h"
#include "ScShapeSim.h"


#define PX_INVALID_PACKET_SHAPE_INDEX 0xffff


namespace physx
{
namespace Sc
{

	class ParticleElementRbElementInteraction : public ElementSimInteraction
	{
	public:

		// The ccdPass parameter is needed to avoid concurrent interaction updates while the gpu particle pipeline is running.
		ParticleElementRbElementInteraction(ParticlePacketShape &particleShape, ShapeSim& rbShape, ActorElementPair& actorElementPair, const PxU32 ccdPass);
		virtual ~ParticleElementRbElementInteraction();
		PX_INLINE void* operator new(size_t s, void* memory);
		
		void destroy(bool isDyingRb, const PxU32 ccdPass);

		//---------- Interaction ----------
	protected:
		virtual bool onActivate(void*);
		virtual bool onDeactivate(PxU32 infoFlag);
		//-----------------------------------

	public:
		//----- ElementSimInteraction ------
		virtual bool isLastFilterInteraction() const { return (mActorElementPair.getRefCount() == 1); }
		//----------------------------------

		PX_INLINE ParticlePacketShape& getParticleShape() const;
		PX_INLINE ShapeSim& getRbShape() const;
		
		PX_INLINE void onRbShapeChange();

		PX_FORCE_INLINE ActorElementPair* getActorElementPair() const { return &mActorElementPair; }
		PX_FORCE_INLINE bool isDisabled() const { return (getActorElementPair()->isSuppressed() || isRbTrigger()); }

		PX_INLINE void setPacketShapeIndex(PxU16 idx);

		PX_INLINE void checkLowLevelActivationState();
	
	private:
		ParticleElementRbElementInteraction& operator=(const ParticleElementRbElementInteraction&);
		void activateForLowLevel(const PxU32 ccdPass);
		void deactivateForLowLevel(bool isDyingRb, const PxU32 ccdPass);
		static void operator delete(void*) {}

		PX_FORCE_INLINE PxU32 isRbTrigger() const { return (getRbShape().getFlags() & PxShapeFlag::eTRIGGER_SHAPE); }


				ActorElementPair&			mActorElementPair;
				PxU16						mPacketShapeIndex;
				bool						mIsActiveForLowLevel; 
	};

} // namespace Sc

PX_INLINE void* Sc::ParticleElementRbElementInteraction::operator new(size_t, void* memory)
{
	return memory;
}


PX_INLINE Sc::ParticlePacketShape& Sc::ParticleElementRbElementInteraction::getParticleShape() const
{
	PX_ASSERT(getElement0().getElementType() == ElementType::ePARTICLE_PACKET);
	return static_cast<ParticlePacketShape&>(getElement0());
}


PX_INLINE Sc::ShapeSim& Sc::ParticleElementRbElementInteraction::getRbShape() const
{
	PX_ASSERT(getElement1().getElementType() == ElementType::eSHAPE);
	PX_ASSERT(static_cast<ShapeSim&>(getElement1()).getActor().isDynamicRigid() || (static_cast<ShapeSim&>(getElement1()).getActor().getActorType() == PxActorType::eRIGID_STATIC));
	return static_cast<ShapeSim&>(getElement1());
}

PX_INLINE void Sc::ParticleElementRbElementInteraction::checkLowLevelActivationState()
{
	if (!isDisabled() && !mIsActiveForLowLevel)
	{
		// The interaction is now valid --> Create low level contact manager
		activateForLowLevel(false);
	}
	else if (isDisabled() && mIsActiveForLowLevel)
	{
		// The interaction is not valid anymore --> Release low level contact manager
		deactivateForLowLevel(false, false);
	}
}

PX_INLINE void Sc::ParticleElementRbElementInteraction::onRbShapeChange()
{
	getParticleShape().getParticleSystem().onRbShapeChange(getParticleShape(), getRbShape());
}

PX_INLINE void Sc::ParticleElementRbElementInteraction::setPacketShapeIndex(PxU16 idx) 
{ 
	PX_ASSERT(idx != PX_INVALID_PACKET_SHAPE_INDEX); 
	mPacketShapeIndex = idx;
}


}

#endif	// PX_USE_PARTICLE_SYSTEM_API

#endif
