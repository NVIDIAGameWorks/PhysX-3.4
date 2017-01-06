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
#include "SampleAllocator.h"
#include <vector>

class SampleSubmarine;

namespace physx
{
	class PxRigidDynamic;
	class PxRevoluteJoint;
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
	PxReal a;	// half x-dim leg
	PxReal b;	// half height upper triangle
	PxReal c;	// half height distance joints (square)
	PxReal d;	// half height lower triangle
	PxReal e;	// x distance from main body centre
	PxReal m;	// half extent of the motor box
	PxReal n;	// y offset of the motor from the main body centre
};

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
struct SqRayBuffer: public SampleAllocateable
{
	SqRayBuffer(SampleSubmarine& sampleSubmarine, PxU32 numRays, PxU32 numHits);
	~SqRayBuffer();
	
	SampleSubmarine&				mSampleSubmarine;
	PxBatchQuery*					mBatchQuery;
	
	PxRaycastQueryResult*			mRayCastResults;
	PxRaycastHit*					mRayCastHits;

	PxU32 							mQueryResultSize; 
	PxU32 							mHitSize;
	void*							mOrigAddresses[2];
private:
	SqRayBuffer& operator=(const SqRayBuffer&);
};

class Crab: public ClassType, public physx::PxLightCpuTask, public SampleAllocateable
{
public:
	Crab(SampleSubmarine& sample, const PxVec3& crabPos, RenderMaterial* material);
	Crab(SampleSubmarine& sample, const char* filename,  RenderMaterial* material);
	~Crab();

	void	update(PxReal dt);
	void	setAcceleration(PxReal leftAcc, PxReal rightAcc);
	void	flushAccelerationBuffer();

	PX_INLINE const PxRigidDynamic*		getCrabBody() const		{ return mCrabBody; }
	PX_INLINE PxRigidDynamic*			getCrabBody()			{ return mCrabBody; }
	PX_INLINE bool						needsRespawn()			{ return mRespawnMe; }
	void	save(const char* filename);

	// Implements LightCpuTask
	virtual  const char*				getName() const			{ return "Crab AI Task"; }
	virtual  void						run();

private:
	void	initMembers();
	void	create(const PxVec3& _crabPos);
	void	load(const char* filename);

	PxVec3	getPlaceOnFloor(PxVec3 pos);
	void	createLeg(PxRigidDynamic* mainBody, PxVec3 localPos, PxReal mass, const LegParameters& params, PxReal scale, PxRigidDynamic* motor, PxVec3 motorAttachmentPos);
	void	scanForObstacles();
	void	updateState();
	void	initState(CrabState::Enum state);
	
private:
	SampleSubmarine*					mSampleSubmarine;

	PxRigidDynamic*						mCrabBody;
	PxRevoluteJoint*					mMotorJoint[2];
	std::vector<PxRigidDynamic*>		mActors;
	std::vector<PxJoint*>				mJoints;


	RenderMaterial*					mMaterial;
	PxReal							mAcceleration[2];
	SqRayBuffer*					mSqRayBuffer;
	PxReal							mLegHeight;
	bool							mRespawnMe;

	CrabState::Enum					mCrabState;
	PxReal							mStateTime;
	PxReal							mDistances[10];
	PxReal							mAccumTime;
	PxReal							mElapsedTime;
	PxVec3							mSubmarinePos;
	PxReal							mAccelerationBuffer[2];
	
	volatile PxU32					mRunning;
	void*							mMemory;
};

#endif
