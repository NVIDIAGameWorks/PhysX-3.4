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


#ifndef PX_PHYSICS_SCP_BODYCORE
#define PX_PHYSICS_SCP_BODYCORE

#include "foundation/PxTransform.h"
#include "ScRigidCore.h"
#include "PxRigidDynamic.h"
#include "PxvDynamics.h"
#include "PxvConfig.h"
#include "PsPool.h"

namespace physx
{

class PxRigidBodyDesc;

namespace Sc
{
	class BodySim;
	struct SimStateData;

	struct KinematicTransform
	{
		PxTransform		targetPose;		// The body will move to this pose over the superstep following this getting set.
		PxU8			targetValid;	// User set a kinematic target.
		PxU8			pad[2];
		PxU8			type;		
	};

	class BodyCore : public RigidCore
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		//---------------------------------------------------------------------------------
		// Construction, destruction & initialization
		//---------------------------------------------------------------------------------
	public:
// PX_SERIALIZATION
											BodyCore(const PxEMPTY) : RigidCore(PxEmpty), mCore(PxEmpty), mSimStateData(NULL)	{}
		static			void				getBinaryMetaData(PxOutputStream& stream);
						void				disableInternalCaching(bool disable);
                        size_t				getSerialCore(PxsBodyCore& serialCore);
//~PX_SERIALIZATION
											BodyCore(PxActorType::Enum type, const PxTransform& bodyPose);
		/*virtual*/							~BodyCore();

		//---------------------------------------------------------------------------------
		// External API
		//---------------------------------------------------------------------------------
		PX_FORCE_INLINE	const PxTransform&	getBody2World()				const	{ return mCore.body2World;			}
						void				setBody2World(const PxTransform& p);

		PX_FORCE_INLINE	const PxVec3&		getLinearVelocity()			const	{ return mCore.linearVelocity;		}
						void				setLinearVelocity(const PxVec3& v); 
	
		PX_FORCE_INLINE	const PxVec3&		getAngularVelocity()		const	{ return mCore.angularVelocity;		}
						void				setAngularVelocity(const PxVec3& v);
	

		PX_FORCE_INLINE	void				updateVelocities(const PxVec3& linearVelModPerStep, const PxVec3& angularVelModPerStep)
											{
												mCore.linearVelocity += linearVelModPerStep;
												mCore.angularVelocity += angularVelModPerStep;
											}

		PX_FORCE_INLINE	const PxTransform&	getBody2Actor()				const	{ return mCore.getBody2Actor();			}
						void				setBody2Actor(const PxTransform& p);

						void				addSpatialAcceleration(Ps::Pool<SimStateData>* simStateDataPool, const PxVec3* linAcc, const PxVec3* angAcc);
						void				clearSpatialAcceleration(bool force, bool torque);
						void				addSpatialVelocity(Ps::Pool<SimStateData>* simStateDataPool, const PxVec3* linVelDelta, const PxVec3* angVelDelta);
						void				clearSpatialVelocity(bool force, bool torque);

		PX_FORCE_INLINE PxReal				getMaxPenetrationBias() const		{ return mCore.maxPenBias; }
		PX_FORCE_INLINE void				setMaxPenetrationBias(PxReal p)		{ mCore.maxPenBias = p; }

						PxReal				getInverseMass()			const;
						void				setInverseMass(PxReal m);
						const PxVec3&		getInverseInertia()			const;
						void				setInverseInertia(const PxVec3& i);

						PxReal				getLinearDamping()			const;
						void				setLinearDamping(PxReal d);

						PxReal				getAngularDamping()			const;
						void				setAngularDamping(PxReal d);

		PX_FORCE_INLINE	PxRigidBodyFlags	getFlags()					const	{ return mCore.mFlags;		}
						void				setFlags(Ps::Pool<SimStateData>* simStateDataPool, PxRigidBodyFlags f);

		PX_FORCE_INLINE	PxRigidDynamicLockFlags	getRigidDynamicLockFlags()					const	{ return mCore.lockFlags; }

		PX_FORCE_INLINE void				setRigidDynamicLockFlags(PxRigidDynamicLockFlags flags) { mCore.lockFlags = flags; }

		PX_FORCE_INLINE	PxReal				getSleepThreshold()	const	{ return mCore.sleepThreshold;	}
						void				setSleepThreshold(PxReal t);		

		PX_FORCE_INLINE	PxReal				getFreezeThreshold()	const	{ return mCore.freezeThreshold;	}
						void				setFreezeThreshold(PxReal t);

		PX_FORCE_INLINE PxReal				getMaxContactImpulse() const	{ return mCore.maxContactImpulse;  }
						void				setMaxContactImpulse(PxReal m);

		PX_FORCE_INLINE PxReal				getWakeCounter() const { return mCore.wakeCounter; }
						void				setWakeCounter(PxReal wakeCounter, bool forceWakeUp=false);

						bool				isSleeping() const;
		PX_FORCE_INLINE	void				wakeUp(PxReal wakeCounter)			{ setWakeCounter(wakeCounter, true);	}
						void				putToSleep();

						PxReal				getMaxAngVelSq() const;
						void				setMaxAngVelSq(PxReal v);


						PxU32				getSolverIterationCounts()	const	{ return mCore.solverIterationCounts;	}
						void				setSolverIterationCounts(PxU16 c);

						bool				getKinematicTarget(PxTransform& p) const;
						bool				getHasValidKinematicTarget() const;
						void				setKinematicTarget(Ps::Pool<SimStateData>* simStateDataPool, const PxTransform& p, PxReal wakeCounter);
						void				invalidateKinematicTarget();	

		PX_FORCE_INLINE	PxReal				getContactReportThreshold()	const	{ return mCore.contactReportThreshold;	}
						void				setContactReportThreshold(PxReal t)	{ mCore.contactReportThreshold = t;		}

						void				onOriginShift(const PxVec3& shift);

		//---------------------------------------------------------------------------------
		// Internal API
		//---------------------------------------------------------------------------------

		PX_FORCE_INLINE void				setLinearVelocityInternal(const PxVec3& v)	{ mCore.linearVelocity = v; }
		PX_FORCE_INLINE void				setAngularVelocityInternal(const PxVec3& v)	{ mCore.angularVelocity = v; }
		PX_FORCE_INLINE	void				setWakeCounterFromSim(PxReal c)		{ mCore.wakeCounter = c;					}

						BodySim*			getSim() const;

		PX_FORCE_INLINE	PxsBodyCore&		getCore()							{ return mCore;						}
		PX_FORCE_INLINE	const PxsBodyCore&	getCore()			const			{ return mCore;						}

		PX_FORCE_INLINE	PxReal				getCCDAdvanceCoefficient() const	{ return mCore.ccdAdvanceCoefficient;	}
		PX_FORCE_INLINE	void				setCCDAdvanceCoefficient(PxReal c)	{ mCore.ccdAdvanceCoefficient = c;		}

						bool				setupSimStateData(Ps::Pool<SimStateData>* simStateDataPool, const bool isKinematic, const bool targetValid = false);
						void				tearDownSimStateData(Ps::Pool<SimStateData>* simStateDataPool, const bool isKinematic);

						bool				checkSimStateKinematicStatus(bool) const;

						Ps::IntBool			isFrozen()							const;
						void				setFrozen();
						void				clearFrozen();

		PX_FORCE_INLINE const SimStateData*	getSimStateData(bool isKinematic)	const	{ return (mSimStateData && (checkSimStateKinematicStatus(isKinematic)) ? mSimStateData : NULL); }
		PX_FORCE_INLINE SimStateData*		getSimStateData(bool isKinematic)			{ return (mSimStateData && (checkSimStateKinematicStatus(isKinematic)) ? mSimStateData : NULL); }

		static PX_FORCE_INLINE BodyCore&	getCore(PxsBodyCore& core)
		{ 
			size_t offset = PX_OFFSET_OF_RT(BodyCore, mCore);
			return *reinterpret_cast<BodyCore*>(reinterpret_cast<PxU8*>(&core) - offset); 
		}		

	private:
						void				backup(SimStateData&);
						void				restore();

						PX_ALIGN_PREFIX(16) PxsBodyCore mCore PX_ALIGN_SUFFIX(16);
						/*PxReal				mSleepThreshold;
						PxReal				mFreezeThreshold;
						PxReal				mWakeCounter;*/
						SimStateData*		mSimStateData;
	};

	PxActor* getPxActorFromBodyCore(Sc::BodyCore* bodyCore, PxActorType::Enum& type);

} // namespace Sc

}

#endif
