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


#ifndef PX_PHYSICS_SCP_ELEMENT_SIM_INTERACTION
#define PX_PHYSICS_SCP_ELEMENT_SIM_INTERACTION

#include "ScInteraction.h"
#include "ScElementSim.h"

namespace physx
{
namespace Sc
{
	class ElementSim;

	class ElementSimInteraction : public Interaction
	{
	public:
		PX_FORCE_INLINE ElementSim& getElement0() const { return mElement0; }
		PX_FORCE_INLINE ElementSim& getElement1() const { return mElement1; }

		// Method to check if this interaction is the last filter relevant interaction between the two elements,
		// i.e., if this interaction gets deleted, the pair is considered lost
		virtual bool isLastFilterInteraction() const { return true; }

		PX_INLINE void setFilterPairIndex(PxU32 filterPairIndex) { mFilterPairIndex = filterPairIndex; }

		PX_INLINE PxU32 getFilterPairIndex() const { return mFilterPairIndex; }

	protected:
		PX_INLINE ElementSimInteraction(ElementSim& element0, ElementSim& element1, InteractionType::Enum type, PxU8 flags);
		virtual ~ElementSimInteraction() {}
		ElementSimInteraction& operator=(const ElementSimInteraction&);

	private:
		ElementSim& mElement0;
		ElementSim& mElement1;
		PxU32		mFilterPairIndex;
	};

} // namespace Sc

//////////////////////////////////////////////////////////////////////////

PX_INLINE Sc::ElementSimInteraction::ElementSimInteraction(ElementSim& element0, ElementSim& element1, InteractionType::Enum type, PxU8 flags) :
	Interaction	(element0.getActor(), element1.getActor(), type, flags),
	mElement0	(element0),
	mElement1	(element1),
	mFilterPairIndex(INVALID_FILTER_PAIR_INDEX)
{
}


}

#endif
