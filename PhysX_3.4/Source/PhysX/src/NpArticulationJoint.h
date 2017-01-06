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


#ifndef PX_PHYSICS_NP_ARTICULATION_JOINT
#define PX_PHYSICS_NP_ARTICULATION_JOINT

#include "PxArticulationJoint.h"
#include "ScbArticulationJoint.h"

#if PX_ENABLE_DEBUG_VISUALIZATION
#include "CmRenderOutput.h"
#endif

namespace physx
{

class NpScene;
class NpArticulationLink;

class NpArticulationJoint : public PxArticulationJoint, public Ps::UserAllocated
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
									NpArticulationJoint(PxBaseFlags baseFlags) : PxArticulationJoint(baseFlags), mJoint(PxEmpty)	{}
	virtual		void				resolveReferences(PxDeserializationContext& context);
	static		NpArticulationJoint* createObject(PxU8*& address, PxDeserializationContext& context);
	static		void				getBinaryMetaData(PxOutputStream& stream);
				void				exportExtraData(PxSerializationContext&)	{}
				void				importExtraData(PxDeserializationContext&)	{}
	virtual		void				requires(PxProcessPxBaseCallback&){}	
	virtual		bool			    isSubordinate()  const	 { return true; }           
//~PX_SERIALIZATION
									NpArticulationJoint(NpArticulationLink& parent, 
														const PxTransform& parentFrame,
														NpArticulationLink& child,
														const PxTransform& childFrame);

	virtual							~NpArticulationJoint();

	//---------------------------------------------------------------------------------
	// PxArticulationJoint implementation
	//---------------------------------------------------------------------------------
	// Save

	virtual		PxTransform			getParentPose() const;
	virtual		void				setParentPose(const PxTransform&);

	virtual		PxTransform			getChildPose() const;
	virtual		void				setChildPose(const PxTransform&);

	virtual		void				setTargetOrientation(const PxQuat&);
	virtual		PxQuat				getTargetOrientation() const;

	virtual		void				setTargetVelocity(const PxVec3&);
	virtual		PxVec3				getTargetVelocity() const;

	virtual		void				setDriveType(PxArticulationJointDriveType::Enum driveType);
	virtual		PxArticulationJointDriveType::Enum
									getDriveType() const;


	virtual		void				setStiffness(PxReal);
	virtual		PxReal				getStiffness() const;

	virtual		void				setDamping(PxReal);
	virtual		PxReal				getDamping() const;

	virtual		void				setInternalCompliance(PxReal);
	virtual		PxReal				getInternalCompliance() const;

	virtual		void				setExternalCompliance(PxReal);
	virtual		PxReal				getExternalCompliance() const;

	virtual		void				setSwingLimit(PxReal yLimit, PxReal zLimit);
	virtual		void				getSwingLimit(PxReal &yLimit, PxReal &zLimit) const;

	virtual		void				setTangentialStiffness(PxReal spring);
	virtual		PxReal				getTangentialStiffness() const;

	virtual		void				setTangentialDamping(PxReal damping);
	virtual		PxReal				getTangentialDamping() const;

	virtual		void				setSwingLimitEnabled(bool);
	virtual		bool				getSwingLimitEnabled() const;

	virtual		void				setSwingLimitContactDistance(PxReal contactDistance);
	virtual		PxReal				getSwingLimitContactDistance() const;

	virtual		void				setTwistLimit(PxReal lower, PxReal upper);
	virtual		void				getTwistLimit(PxReal &lower, PxReal &upper) const;

	virtual		void				setTwistLimitEnabled(bool);
	virtual		bool				getTwistLimitEnabled() const;

	virtual		void				setTwistLimitContactDistance(PxReal contactDistance);
	virtual		PxReal				getTwistLimitContactDistance() const;


	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
public:
				void				release();

	PX_INLINE	const Scb::ArticulationJoint&	getScbArticulationJoint() const { return mJoint; }
	PX_INLINE	Scb::ArticulationJoint&			getScbArticulationJoint() { return mJoint; }

	PX_INLINE	const NpArticulationLink&		getParent() const	{ return *mParent; }
	PX_INLINE	NpArticulationLink&				getParent()			{ return *mParent; }

	PX_INLINE	const NpArticulationLink&		getChild() const	{ return *mChild; }
	PX_INLINE	NpArticulationLink&				getChild()			{ return *mChild; }

private:
				NpScene*						getOwnerScene() const; // the scene the user thinks the actor is in, or from which the actor is pending removal


private:
				Scb::ArticulationJoint			mJoint;
				NpArticulationLink*				mParent;
				NpArticulationLink*				mChild;
};

}

#endif
