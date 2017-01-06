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


#ifndef PX_PHYSICS_SCP_RB_SIM
#define PX_PHYSICS_SCP_RB_SIM

#include "ScActorSim.h"
#include "ScRigidCore.h"
#include "BpSimpleAABBManager.h"

namespace physx
{

class Interaction;

namespace Sc
{
	class ShapeCore;
	class ShapeSim;
	class Scene;

	class RigidSim : public ActorSim
	{
	public:
										RigidSim(Scene&, RigidCore&);
		virtual							~RigidSim();

		PX_FORCE_INLINE	RigidCore&		getRigidCore()			const	{ return static_cast<RigidCore&>(mCore);	}

		PX_FORCE_INLINE	PxU32			getID()					const	{ return mRigidId;	}

		PX_FORCE_INLINE	PxU32			getBroadphaseGroupId()	const	{ return (getActorType()!=PxActorType::eRIGID_STATIC ? mRigidId + PxU32(Bp::FilterGroup::eDYNAMICS_BASE) : PxU32(Bp::FilterGroup::eSTATICS));}

		void							notifyShapesOfTransformChange();

						Sc::ShapeSim&	getSimForShape(Sc::ShapeCore& shape) const;


						PxActor*		getPxActor() const;
	private:
						PxU32			mRigidId;
	};

} // namespace Sc

}

#endif
