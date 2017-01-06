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


#ifndef PX_PHYSICS_PARTICLESHAPE
#define PX_PHYSICS_PARTICLESHAPE

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScElementSim.h"
#include "ScParticleSystemSim.h"
#include "PtParticleShape.h"

namespace physx
{
namespace Sc
{
	class ParticleElementRbElementInteraction;


	/**
	A collision detection primitive for particle systems.
	*/
	class ParticlePacketShape : public ElementSim
	{
		public:
													ParticlePacketShape(ParticleSystemSim& particleSystem, PxU32 uid, Pt::ParticleShape* llParticleShape);
													~ParticlePacketShape();
		
		// ElementSim implementation
		virtual			void						getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const;
		// ~ElementSim

		virtual			void						createLowLevelVolume();
		virtual			void						destroyLowLevelVolume();

		public:
						void						setIndex(PxU32 index) { PX_ASSERT(index < ((1 << 16) - 1)); mIndex = static_cast<PxU16>(index); }
		PX_FORCE_INLINE	PxU16						getIndex()	const { return mIndex; }

		PX_FORCE_INLINE	PxBounds3					getBounds()	const	{ return mLLParticleShape->getBoundsV(); }

						class ParticleSystemSim&	getParticleSystem()	const;

						void						computeWorldBounds(PxBounds3&)	const;

		PX_FORCE_INLINE	ParticleElementRbElementInteraction**	getInteractions()		const { return mInteractions.begin();	}
		PX_FORCE_INLINE	PxU32									getInteractionsCount()	const { return mInteractions.size();	}

		PX_FORCE_INLINE	Pt::ParticleShape*			getLowLevelParticleShape() const { return mLLParticleShape; }

						void						setInteractionsDirty(InteractionDirtyFlag::Enum flag);

		PX_INLINE		PxU16						addPacketShapeInteraction(ParticleElementRbElementInteraction* interaction);
		PX_INLINE		void						removePacketShapeInteraction(PxU16 id);
		PX_INLINE		ParticleElementRbElementInteraction*	getPacketShapeInteraction(PxU16 id) const;

		private:
						void						reallocInteractions(ParticleElementRbElementInteraction**& mem, PxU16& capacity, PxU16 size, PxU16 requiredMinCapacity);


		static const PxU32 INLINE_INTERACTION_CAPACITY = 4;
						ParticleElementRbElementInteraction* mInlineInteractionMem[INLINE_INTERACTION_CAPACITY];

		Cm::OwnedArray<ParticleElementRbElementInteraction*, ParticlePacketShape, PxU16, &ParticlePacketShape::reallocInteractions>
													mInteractions;

						Pt::ParticleShape*			mLLParticleShape;		// Low level handle of particle packet
						PxU16						mIndex;
	};

} // namespace Sc


//These are called from interaction creation/destruction
PX_INLINE PxU16 Sc::ParticlePacketShape::addPacketShapeInteraction(ParticleElementRbElementInteraction* interaction)
{
	mInteractions.pushBack(interaction, *this);
	return PxU16(mInteractions.size()-1);
}

PX_INLINE void Sc::ParticlePacketShape::removePacketShapeInteraction(PxU16 id)
{
	mInteractions.replaceWithLast(id);
}

PX_INLINE Sc::ParticleElementRbElementInteraction* Sc::ParticlePacketShape::getPacketShapeInteraction(PxU16 id) const
{
	PX_ASSERT(id<mInteractions.size());
	return mInteractions[id];
}


}

#endif	// PX_USE_PARTICLE_SYSTEM_API

#endif
