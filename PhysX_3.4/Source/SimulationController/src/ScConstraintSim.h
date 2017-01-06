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


#ifndef PX_PHYSICS_CONSTRAINT_SIM
#define PX_PHYSICS_CONSTRAINT_SIM

#include "PxSimulationEventCallback.h"
#include "DyConstraint.h"

namespace physx
{
namespace Sc
{

	class Scene;
	class ConstraintInteraction;
	class ConstraintCore;
	class RigidCore;
	class BodySim;
	class RigidSim;

	class ConstraintSim : public Ps::UserAllocated 
	{
	public:
		enum Enum
		{
			ePENDING_GROUP_UPDATE		=	(1<<0),	// For constraint projection an island of the bodies connected by constraints is generated.
													// Schedule generation/update of the island this constraint is a part of.
			eBREAKABLE					=	(1<<1),	// The constraint can break
			eCHECK_MAX_FORCE_EXCEEDED	=	(1<<2),	// This constraint will get tested for breakage at the end of the sim step
			eBROKEN						=	(1<<3)
		};

												ConstraintSim(ConstraintCore& core, 
													RigidCore* r0,
													RigidCore* r1,
													Scene& scene);

												~ConstraintSim();

						void					preBodiesChange();
						void					postBodiesChange(RigidCore* r0, RigidCore* r1);

						void					checkMaxForceExceeded();
						void					updateRelatedSIPs();

						void					setBreakForceLL(PxReal linear, PxReal angular);
		PX_INLINE		void					setMinResponseThresholdLL(PxReal threshold);
						void					setConstantsLL(void* addr);
		PX_INLINE		const void*				getConstantsLL() const;

						void					postFlagChange(PxConstraintFlags oldFlags, PxConstraintFlags newFlags);

		PX_FORCE_INLINE	const Dy::Constraint&	getLowLevelConstraint()	const	{ return mLowLevelConstraint;	}
		PX_FORCE_INLINE	Dy::Constraint&			getLowLevelConstraint()			{ return mLowLevelConstraint;	}
		PX_FORCE_INLINE	ConstraintCore&			getCore()				const	{ return mCore;					}
		PX_FORCE_INLINE	BodySim*				getBody(PxU32 i) const  // for static actors or world attached constraints NULL is returned
												{
													return mBodies[i];
												}

						RigidSim&				getRigid(PxU32 i);

						void					getForce(PxVec3& force, PxVec3& torque);
						bool					isBroken() const;

		PX_FORCE_INLINE	PxU8					readFlag(PxU8 flag) const { return PxU8(mFlags & flag); }
		PX_FORCE_INLINE	void					setFlag(PxU8 flag) { mFlags |= flag; }
		PX_FORCE_INLINE	void					clearFlag(PxU8 flag) { mFlags &= ~flag; }


		//------------------------------------ Projection trees -----------------------------------------
	private:
		PX_INLINE		BodySim*				getConstraintGroupBody();

	public:
						bool					hasDynamicBody();

						void					projectPose(BodySim* childBody, Ps::Array<BodySim*>& projectedBodies);
		PX_INLINE		BodySim*				getOtherBody(BodySim*);
		PX_INLINE		BodySim*				getAnyBody();

						bool					needsProjection();
		//-----------------------------------------------------------------------------------------------

						void					visualize(PxRenderBuffer &out);
	private:
						ConstraintSim&			operator=(const ConstraintSim&);
						bool					createLLConstraint();
						void					destroyLLConstraint();
	private:
						Dy::Constraint			mLowLevelConstraint;
						Scene&					mScene;
						ConstraintCore&			mCore;
						ConstraintInteraction*	mInteraction;
						BodySim*				mBodies[2];
						PxU8					mFlags;
	};
} // namespace Sc


PX_INLINE void Sc::ConstraintSim::setMinResponseThresholdLL(PxReal threshold)
{
	mLowLevelConstraint.minResponseThreshold = threshold;
}

PX_INLINE const void* Sc::ConstraintSim::getConstantsLL()	const
{
	return mLowLevelConstraint.constantBlock;
}


PX_INLINE Sc::BodySim* Sc::ConstraintSim::getOtherBody(BodySim* b)
{
	return (b == mBodies[0]) ? mBodies[1] : mBodies[0];
}


PX_INLINE Sc::BodySim* Sc::ConstraintSim::getAnyBody()
{
	if (mBodies[0]) 
		return mBodies[0];
	else 
		return mBodies[1];
}

}

#endif
