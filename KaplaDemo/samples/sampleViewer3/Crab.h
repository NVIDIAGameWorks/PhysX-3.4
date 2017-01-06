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

#ifndef CRAB_H
#define CRAB_H

#include "foundation/Px.h"
#include "foundation/PxSimpleTypes.h"
#include "common/PxPhysXCommonConfig.h"
#include "task/PxTask.h"
#include <vector>

#define UPDATE_FREQUENCY_RESET 10


struct CrabState
{
	enum Enum
	{
		eWAITING,
		eMOVE_FWD,
		eMOVE_BKWD,
		eROTATE_LEFT,
		eROTATE_RIGHT,
		ePANIC,
		eNUM_STATES,
	};
};



namespace physx
{
	class PxRigidDynamic;
	class PxRevoluteJoint;
	class PxJoint;
	class PxScene;
}

//  Edge Labels for Theo Jansen's Mechanism
//		       _.
//		     ,' |       m
//		   ,'   |2b   .....
//		 ,' 2a  |    e    |n
//		+-------+---------+
//		|       |
//		|       |2c
//		|       |
//		+-------+
//		 `.     |
//		   `.   |2d
//		     `. |
//		       `|

struct LegParameters
{
	physx::PxReal a;	// half x-dim leg
	physx::PxReal b;	// half height upper triangle
	physx::PxReal c;	// half height distance joints (square)
	physx::PxReal d;	// half height lower triangle
	physx::PxReal e;	// x distance from main body centre
	physx::PxReal m;	// half extent of the motor box
	physx::PxReal n;	// y offset of the motor from the main body centre
};

class Crab;
class CrabManager;

struct SqRayBuffer
{
	SqRayBuffer(Crab* crab, physx::PxU32 numRays, physx::PxU32 numHits);
	~SqRayBuffer();

	Crab*						mCrab;
	physx::PxBatchQuery*					mBatchQuery;

	physx::PxRaycastQueryResult*			mRayCastResults;
	physx::PxRaycastHit*					mRayCastHits;

	physx::PxU32 							mQueryResultSize;
	physx::PxU32 							mHitSize;
	void*							mOrigAddresses[2];

	void setScene(physx::PxScene* scene);
private:
	SqRayBuffer& operator=(const SqRayBuffer&);
};

struct SqSweepBuffer
{
	SqSweepBuffer(Crab* crab, physx::PxU32 numRays, physx::PxU32 numHits);
	~SqSweepBuffer();

	Crab*									mCrab;
	physx::PxBatchQuery*					mBatchQuery;

	physx::PxSweepQueryResult*				mSweepResults;
	physx::PxSweepHit*						mSweepHits;

	physx::PxU32 							mQueryResultSize;
	physx::PxU32 							mHitSize;
	void*									mOrigAddresses[2];

	void setScene(physx::PxScene* scene);
private:
	SqSweepBuffer& operator=(const SqSweepBuffer&);
};

class Crab
{
public:

	Crab(CrabManager* crabManager, physx::PxU32 updateFrequency);
	~Crab();

	void	update(physx::PxReal dt);
	void	setAcceleration(physx::PxReal leftAcc, physx::PxReal rightAcc);
	void	flushAccelerationBuffer();

	PX_INLINE const physx::PxRigidDynamic*		getCrabBody() const		{ return mCrabBody; }
	PX_INLINE physx::PxRigidDynamic*			getCrabBody()			{ return mCrabBody; }


	void								run();

	void setScene(physx::PxScene* scene);



	void	initMembers();
	Crab*	create(const physx::PxVec3& _crabPos, const physx::PxReal crabDepth, const physx::PxReal scale, const physx::PxReal legMass, const physx::PxU32 numLegs);

	physx::PxVec3	getPlaceOnFloor(physx::PxVec3 pos);
	void	createLeg(physx::PxRigidDynamic* mainBody, physx::PxVec3 localPos, physx::PxReal mass, const LegParameters& params, physx::PxReal scale, physx::PxRigidDynamic* motor, physx::PxVec3 motorAttachmentPos);
	void	scanForObstacles();
	void	updateState();
	void	initState(CrabState::Enum state);

	void setManager(CrabManager* crabManager) { mManager = crabManager;  }
	CrabManager* getManager(){ return mManager;  }

private:
	CrabManager*								mManager;
	physx::PxRigidDynamic*						mCrabBody;
	//physx::PxRevoluteJoint*						mMotorJoint[2];
	physx::PxD6Joint*							mMotorJoint[2];
	physx::PxMaterial*							mMaterial;

	physx::PxReal								mAcceleration[2];
	//SqRayBuffer*								mSqRayBuffer;
	SqSweepBuffer*								mSqSweepBuffer;

	physx::PxReal								mLegHeight;

	CrabState::Enum								mCrabState;
	physx::PxReal								mStateTime;
	physx::PxReal								mDistances[10];
	physx::PxReal								mAccumTime;
	physx::PxReal								mElapsedTime;
	physx::PxReal								mAccelerationBuffer[2];
	physx::PxU32								mUpdateFrequency;
};

#endif
