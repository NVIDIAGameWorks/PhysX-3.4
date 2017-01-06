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

#ifndef PULLEY_JOINT_H
#define PULLEY_JOINT_H

#include "PxPhysicsAPI.h"

// a pulley joint constrains two actors such that the sum of their distances from their respective anchor points at their attachment points 
// is a fixed value (the parameter 'distance'). Only dynamic actors are supported.
//
// The constraint equation is as follows:
//
// |anchor0 - attachment0| + |anchor1 - attachment1| * ratio = distance
// 
// where 'ratio' provides mechanical advantage.
//
// The above equation results in a singularity when the anchor point is coincident with the attachment point; for simplicity
// the constraint does not attempt to handle this case robustly.



class PulleyJoint : public physx::PxConstraintConnector
{
public:

	static const physx::PxU32 TYPE_ID = physx::PxConcreteType::eFIRST_USER_EXTENSION;

	PulleyJoint(physx::PxPhysics& physics, 
				physx::PxRigidBody& body0, const physx::PxTransform& localFrame0, const physx::PxVec3& attachment0,
			    physx::PxRigidBody& body1, const physx::PxTransform& localFrame1, const physx::PxVec3& attachment1);

	void release();

	// attribute accessor and mutators

	void			setAttachment0(const physx::PxVec3& pos);
	physx::PxVec3	getAttachment0() const;

	void			setAttachment1(const physx::PxVec3& pos);
	physx::PxVec3	getAttachment1() const;

	void			setDistance(physx::PxReal totalDistance);
	physx::PxReal	getDistance() const;
	
	void			setRatio(physx::PxReal ratio);
	physx::PxReal	getRatio() const;

	// PxConstraintConnector boilerplate

	void*			prepareData();
	void			onConstraintRelease();
	void			onComShift(physx::PxU32 actor);
	void			onOriginShift(const physx::PxVec3& shift);
	void*			getExternalReference(physx::PxU32& typeID);


	bool			updatePvdProperties(physx::pvdsdk::PvdDataStream&,
										const physx::PxConstraint*,
										physx::PxPvdUpdateType::Enum) const { return true; }
	physx::PxBase*	getSerializable() { return NULL; }

	virtual physx::PxConstraintSolverPrep getPrep() const { return sShaderTable.solverPrep; }

	virtual const void* getConstantBlock() const { return &mData; }

private:

	static physx::PxU32 solverPrep(physx::Px1DConstraint* constraints,
								   physx::PxVec3& body0WorldOffset,
								   physx::PxU32 maxConstraints,
								   physx::PxConstraintInvMassScale&,
								   const void* constantBlock,
								   const physx::PxTransform& bA2w,
								   const physx::PxTransform& bB2w);


	static void	visualize(physx::PxConstraintVisualizer& viz,
						  const void* constantBlock,
						  const physx::PxTransform&	body0Transform,
						  const physx::PxTransform&	body1Transform,
						  physx::PxU32 flags);

	static void project(const void* constantBlock,
						physx::PxTransform& bodyAToWorld,
						physx::PxTransform& bodyBToWorld,
						bool projectToA);


	struct PulleyJointData
	{
		physx::PxTransform c2b[2];
	
		physx::PxVec3 attachment0;
		physx::PxVec3 attachment1;

		physx::PxReal distance;
		physx::PxReal ratio;
		physx::PxReal tolerance;
	};

	physx::PxRigidBody*		mBody[2];
	physx::PxTransform		mLocalPose[2];

	physx::PxConstraint*	mConstraint;
	PulleyJointData			mData;
	static physx::PxConstraintShaderTable
							sShaderTable;

	~PulleyJoint() {}
};

#endif
