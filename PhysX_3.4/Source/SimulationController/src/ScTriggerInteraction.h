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


#ifndef PX_COLLISION_TRIGGERINTERACTION
#define PX_COLLISION_TRIGGERINTERACTION

#include "ScRbElementInteraction.h"
#include "GuOverlapTests.h"

namespace physx
{
namespace Sc
{
	class TriggerInteraction : public RbElementInteraction
	{
	public:
		enum TriggerFlag
		{
			PAIR_FLAGS_MASK					= ((PxPairFlag::eNOTIFY_TOUCH_LOST << 1) - 1),	// Bits where the PxPairFlags eNOTIFY_TOUCH_FOUND and eNOTIFY_TOUCH_LOST get stored
			NEXT_FREE						= ((PAIR_FLAGS_MASK << 1) & ~PAIR_FLAGS_MASK),

			PROCESS_THIS_FRAME				= (NEXT_FREE << 0), // the trigger pair is new or the pose of an actor was set -> initial processing required.
																// This is important to cover cases where a static or kinematic
																// (non-moving) trigger is created and overlaps with a sleeping
																// object. Or for the case where a static/kinematic is teleported to a new
																// location. TOUCH_FOUND should still get sent in that case.
			LAST							= (NEXT_FREE << 1)
		};

											TriggerInteraction(ShapeSim& triggerShape, ShapeSim& otherShape);
		virtual								~TriggerInteraction();

		PX_FORCE_INLINE	Gu::TriggerCache&	getTriggerCache()									{ return mTriggerCache;					}
		PX_FORCE_INLINE	ShapeSim&			getTriggerShape()							const	{ return getShape0();					}
		PX_FORCE_INLINE	ShapeSim&			getOtherShape()								const	{ return getShape1();					}

		PX_FORCE_INLINE bool				lastFrameHadContacts()						const	{ return mLastFrameHadContacts;			}
		PX_FORCE_INLINE void				updateLastFrameHadContacts(bool hasContact)			{ mLastFrameHadContacts = hasContact;	}

		PX_FORCE_INLINE PxPairFlags			getTriggerFlags()							const	{ return PxPairFlags(PxU32(mFlags) & PAIR_FLAGS_MASK);		}
		PX_FORCE_INLINE void				setTriggerFlags(PxPairFlags triggerFlags);

		PX_FORCE_INLINE void				raiseFlag(TriggerFlag flag)				{ mFlags |= flag; }
		PX_FORCE_INLINE void				clearFlag(TriggerFlag flag)				{ mFlags &= ~flag; }
		PX_FORCE_INLINE	Ps::IntBool			readFlag(TriggerFlag flag)		const	{ return Ps::IntBool(mFlags & flag); }

		PX_FORCE_INLINE void				forceProcessingThisFrame(Sc::Scene& scene);

		//////////////////////// interaction ////////////////////////
		virtual			bool				onActivate(void*);
		virtual			bool				onDeactivate(PxU32 infoFlag);

	private:
						bool				isOneActorActive() const;

	protected:
						Gu::TriggerCache	mTriggerCache;
						PxU16				mFlags;
						bool				mLastFrameHadContacts;
	};

} // namespace Sc


PX_FORCE_INLINE void Sc::TriggerInteraction::setTriggerFlags(PxPairFlags triggerFlags)
{
	PX_ASSERT(PxU32(triggerFlags) < (PxPairFlag::eDETECT_CCD_CONTACT << 1));  // to find out if a new PxPairFlag has been added in which case PAIR_FLAGS_MASK needs to get adjusted

#if PX_CHECKED
	if (triggerFlags & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
	{
		PX_WARN_ONCE("Trigger pairs do not support PxPairFlag::eNOTIFY_TOUCH_PERSISTS events any longer.");
	}
#endif

	PxU32 newFlags = mFlags;
	PxU32 fl = PxU32(triggerFlags) & PxU32(PxPairFlag::eNOTIFY_TOUCH_FOUND|PxPairFlag::eNOTIFY_TOUCH_LOST);
	newFlags &= (~PAIR_FLAGS_MASK);  // clear old flags
	newFlags |= fl;

	mFlags = PxU16(newFlags);
}


PX_FORCE_INLINE void Sc::TriggerInteraction::forceProcessingThisFrame(Sc::Scene& scene)
{
	raiseFlag(PROCESS_THIS_FRAME);

	if (!readInteractionFlag(InteractionFlag::eIS_ACTIVE))
	{
		raiseInteractionFlag(InteractionFlag::eIS_ACTIVE);
		scene.notifyInteractionActivated(this);
	}
}

}

#endif
