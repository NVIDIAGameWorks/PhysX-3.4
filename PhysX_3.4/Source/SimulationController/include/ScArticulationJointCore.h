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


#ifndef PX_PHYSICS_SCP_ARTICULATION_JOINT_CORE
#define PX_PHYSICS_SCP_ARTICULATION_JOINT_CORE

#include "foundation/PxTransform.h"
#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "DyArticulation.h"
#include "PxMetaData.h"

namespace physx
{
namespace Sc
{

	class BodyCore;
	class ArticulationJointSim;

	class ArticulationJointDesc
	{
	public:
		BodyCore*			parent;
		BodyCore*			child;
		PxTransform			parentPose;
		PxTransform			childPose;
	};

	class ArticulationJointCore : public Ps::UserAllocated
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
							ArticulationJointCore(const PxEMPTY) : mSim(NULL), mCore(PxEmpty)	{}
		static	void		getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
							ArticulationJointCore(const PxTransform& parentFrame,
												  const PxTransform& childFrame);

							~ArticulationJointCore();

		//---------------------------------------------------------------------------------
		// External API
		//---------------------------------------------------------------------------------

		const PxTransform&	getParentPose() const { return mCore.parentPose; }
		void				setParentPose(const PxTransform&);

		const PxTransform&	getChildPose() const { return mCore.childPose; }
		void				setChildPose(const PxTransform&);

		const PxQuat&		getTargetOrientation() const { return mCore.targetPosition; }
		void				setTargetOrientation(const PxQuat&);

		const PxVec3&		getTargetVelocity() const { return mCore.targetVelocity; }
		void				setTargetVelocity(const PxVec3&);

		PxReal				getStiffness() const { return mCore.spring; }
		void				setStiffness(PxReal);

		PxReal				getDamping() const { return mCore.damping; }
		void				setDamping(PxReal);

		PxReal				getInternalCompliance() const { return mCore.internalCompliance; }
		void				setInternalCompliance(PxReal);

		PxReal				getExternalCompliance() const { return mCore.externalCompliance; }
		void				setExternalCompliance(PxReal);

		void				getSwingLimit(PxReal& yLimit, PxReal& zLimit) const { yLimit = mCore.swingYLimit; zLimit = mCore.swingZLimit; }
		void				setSwingLimit(PxReal yLimit, PxReal zLimit);

		PxReal				getTangentialStiffness() const { return mCore.tangentialStiffness; }
		void				setTangentialStiffness(PxReal);

		PxReal				getTangentialDamping() const { return mCore.tangentialDamping; }
		void				setTangentialDamping(PxReal);

		bool				getSwingLimitEnabled() const { return mCore.swingLimited; }
		void				setSwingLimitEnabled(bool);

		PxReal				getSwingLimitContactDistance() const { return mCore.swingLimitContactDistance; }
		void				setSwingLimitContactDistance(PxReal);

		void				getTwistLimit(PxReal& lower, PxReal& upper) const { lower = mCore.twistLimitLow; upper = mCore.twistLimitHigh; }
		void				setTwistLimit(PxReal lower, PxReal upper);

		bool				getTwistLimitEnabled() const { return mCore.twistLimited; }
		void				setTwistLimitEnabled(bool);

		PxReal				getTwistLimitContactDistance() const { return mCore.twistLimitContactDistance; }
		void				setTwistLimitContactDistance(PxReal);

		void				setDriveType(PxArticulationJointDriveType::Enum type);
		PxArticulationJointDriveType::Enum 
							getDriveType() const					{ return PxArticulationJointDriveType::Enum(mCore.driveType); }

		//---------------------------------------------------------------------------------
		// Low Level data access - some wouldn't be needed if the interface wasn't virtual
		//---------------------------------------------------------------------------------

		PX_FORCE_INLINE	ArticulationJointSim*	getSim() const	{ return mSim;	}
		PX_FORCE_INLINE	void					setSim(ArticulationJointSim* sim)
												{
													PX_ASSERT((sim==0) ^ (mSim == 0));
													mSim = sim;
												}

		PX_FORCE_INLINE	const Dy::ArticulationJointCore&	getCore() { return mCore; }

	private:
		ArticulationJointSim*		mSim;
		Dy::ArticulationJointCore	mCore;
	};

} // namespace Sc

}

#endif
