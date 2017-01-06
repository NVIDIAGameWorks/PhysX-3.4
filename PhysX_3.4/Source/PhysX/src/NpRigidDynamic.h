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


#ifndef PX_PHYSICS_NP_RIGIDDYNAMIC
#define PX_PHYSICS_NP_RIGIDDYNAMIC

#include "NpRigidBodyTemplate.h"
#include "PxRigidDynamic.h"
#include "ScbBody.h"
#include "PxMetaData.h"

namespace physx
{

class NpRigidDynamic;
typedef NpRigidBodyTemplate<PxRigidDynamic> NpRigidDynamicT;

class NpRigidDynamic : public NpRigidDynamicT
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
									NpRigidDynamic(PxBaseFlags baseFlags) : NpRigidDynamicT(baseFlags) {}

	virtual		void				requires(PxProcessPxBaseCallback& c);

	static		NpRigidDynamic*		createObject(PxU8*& address, PxDeserializationContext& context);
	static		void				getBinaryMetaData(PxOutputStream& stream);	
//~PX_SERIALIZATION
	virtual							~NpRigidDynamic();

	//---------------------------------------------------------------------------------
	// PxActor implementation
	//---------------------------------------------------------------------------------

	virtual		void				release();

	//---------------------------------------------------------------------------------
	// PxRigidDynamic implementation
	//---------------------------------------------------------------------------------

	virtual		PxActorType::Enum	getType() const { return PxActorType::eRIGID_DYNAMIC; }

	// Pose
	virtual		void 				setGlobalPose(const PxTransform& pose, bool autowake);
	PX_FORCE_INLINE		PxTransform			getGlobalPoseFast() const
	{
		const Scb::Body& body=getScbBodyFast();
		return body.getBody2World() * body.getBody2Actor().getInverse();
	}
	virtual		PxTransform			getGlobalPose() const
	{
		NP_READ_CHECK(NpActor::getOwnerScene(*this));
		return getGlobalPoseFast();
	}

	virtual		void				setKinematicTarget(const PxTransform& destination);
	virtual		bool				getKinematicTarget(PxTransform& target)	const;

	// Center of mass pose
	virtual		void				setCMassLocalPose(const PxTransform&);

	// Damping
	virtual		void				setLinearDamping(PxReal);
	virtual		PxReal				getLinearDamping() const;
	virtual		void				setAngularDamping(PxReal);
	virtual		PxReal				getAngularDamping() const;

	// Velocity
	virtual		void				setLinearVelocity(const PxVec3&, bool autowake);
	virtual		void				setAngularVelocity(const PxVec3&, bool autowake);
	virtual		void				setMaxAngularVelocity(PxReal);
	virtual		PxReal				getMaxAngularVelocity() const;

	// Force/Torque modifiers
	virtual		void				addForce(const PxVec3&, PxForceMode::Enum mode, bool autowake);
	virtual		void				clearForce(PxForceMode::Enum mode);
	virtual		void				addTorque(const PxVec3&, PxForceMode::Enum mode, bool autowake);
	virtual		void				clearTorque(PxForceMode::Enum mode);

	// Sleeping
	virtual		bool				isSleeping() const;
	virtual		PxReal				getSleepThreshold() const;
	virtual		void				setSleepThreshold(PxReal threshold);
	virtual		PxReal				getStabilizationThreshold() const;
	virtual		void				setStabilizationThreshold(PxReal threshold);
	virtual		void				setWakeCounter(PxReal wakeCounterValue);
	virtual		PxReal				getWakeCounter() const;
	virtual		void				wakeUp();
	virtual		void				putToSleep();

	virtual		void				setSolverIterationCounts(PxU32 positionIters, PxU32 velocityIters);
	virtual		void				getSolverIterationCounts(PxU32 & positionIters, PxU32 & velocityIters) const;

	virtual		void				setContactReportThreshold(PxReal threshold);
	virtual		PxReal				getContactReportThreshold() const;

	virtual		PxRigidDynamicLockFlags getRigidDynamicLockFlags() const;
	virtual		void				setRigidDynamicLockFlags(PxRigidDynamicLockFlags flags);
	virtual		void				setRigidDynamicLockFlag(PxRigidDynamicLockFlag::Enum flag, bool value);

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
									NpRigidDynamic(const PxTransform& bodyPose);

	virtual		void				switchToNoSim();
	virtual		void				switchFromNoSim();

	PX_FORCE_INLINE void			wakeUpInternal();
					void			wakeUpInternalNoKinematicTest(Scb::Body& body, bool forceWakeUp, bool autowake);

private:
	PX_FORCE_INLINE	void			setKinematicTargetInternal(const PxTransform& destination);

#if PX_ENABLE_DEBUG_VISUALIZATION
public:
				void				visualize(Cm::RenderOutput& out, NpScene* scene);
#endif
};




PX_FORCE_INLINE void NpRigidDynamic::wakeUpInternal()
{
	PX_ASSERT(NpActor::getOwnerScene(*this));

	Scb::Body& body = getScbBodyFast();
	const PxRigidBodyFlags currentFlags = body.getFlags();

	if (!(currentFlags & PxRigidBodyFlag::eKINEMATIC))  // kinematics are only awake when a target is set, else they are asleep
		wakeUpInternalNoKinematicTest(body, false, true);
}


}

#endif
