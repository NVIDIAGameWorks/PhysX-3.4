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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef PX_SOLVER_DEFS_H
#define PX_SOLVER_DEFS_H


#include "PxPhysXConfig.h"
#include "foundation/PxVec3.h"
#include "foundation/PxMat33.h"
#include "foundation/PxTransform.h"
#include "PxConstraintDesc.h"
#include "GeomUtils/GuContactPoint.h"

#if PX_VC
#pragma warning(push)
#pragma warning(disable : 4324) // structure was padded due to alignment
#endif

#if !PX_DOXYGEN
namespace physx
{
#endif

namespace Dy
{
	struct FsData;
}

namespace Sc
{
	class ShapeInteraction;
}

struct PxSolverBody
{
	PxVec3				linearVelocity;					//!< Delta linear velocity computed by the solver
	PxU16				maxSolverNormalProgress;		//!< Progress counter used by constraint batching and parallel island solver. 
	PxU16				maxSolverFrictionProgress;		//!< Progress counter used by constraint batching and parallel island solver.

	PxVec3				angularState;					//!< Delta angular velocity state computed by the solver.

	PxU32				solverProgress;					//!< Progress counter used by constraint batching and parallel island solver

	PxSolverBody() : linearVelocity(0.f), maxSolverNormalProgress(0), maxSolverFrictionProgress(0), angularState(0), solverProgress(0)
	{
	}
};

PX_COMPILE_TIME_ASSERT(sizeof(PxSolverBody) == 32);

struct PxSolverBodyData
{
	PX_ALIGN(16, PxVec3 linearVelocity);	//!< 12 Pre-solver linear velocity
	PxReal			invMass;				//!< 16 inverse mass
	PxVec3			angularVelocity;		//!< 28 Pre-solver angular velocity
	PxReal			reportThreshold;		//!< 32 contact force threshold
	PxMat33			sqrtInvInertia;			//!< 68 inverse inertia in world space
	PxReal			penBiasClamp;			//!< 72 the penetration bias clamp
	PxU32			nodeIndex;				//!< 76 the node idx of this solverBodyData. Used by solver to reference between solver bodies and island bodies. Not required by immediate mode
	PxReal			maxContactImpulse;		//!< 80 the max contact impulse
	PxTransform		body2World;				//!< 108 the body's transform
	PxU32			lockFlags;				//!< 112 lock flags

	PX_FORCE_INLINE PxReal projectVelocity(const PxVec3& lin, const PxVec3& ang)	const
	{
		return linearVelocity.dot(lin) + angularVelocity.dot(ang);
	}
};

//----------------------------------
/*
* A header that defines the size of a specific batch of constraints (of same type and without dependencies)
*/
struct PxConstraintBatchHeader
{
	PxU32 mStartIndex;			//!< Start index for this batch
	PxU16 mStride;				//!< Number of constraints in this batch (range: 1-4)
	PxU16 mConstraintType;		//!< The type of constraint this batch references
};


struct PxSolverConstraintDesc
{
	static const PxU16 NO_LINK = 0xffff;

	enum ConstraintType
	{
		eCONTACT_CONSTRAINT,				//!< Defines this pair is a contact constraint
		eJOINT_CONSTRAINT					//!< Defines this pair is a joint constraint
	};

	union
	{
		PxSolverBody*		bodyA;			//!< bodyA pointer
		Dy::FsData*			articulationA;	//!< Articulation pointer for body A
	};

	union
	{
		PxSolverBody*		bodyB;			//!< BodyB pointer
		Dy::FsData*			articulationB;	//!< Articulation pointer for body B
	};
	PxU16				linkIndexA;			//!< Link index defining which link in Articulation A this constraint affects. If not an articulation, must be NO_LINK
	PxU16				linkIndexB;			//!< Link index defining which link in Articulation B this constraint affects. If not an articulation, must be NO_LINK.
	union
	{
		PxU16				articulationALength;	//!< The total length of articulation A in multiples of 16 bytes
		PxU32				bodyADataIndex;			//!< Body A's index into the SolverBodyData array
	};

	union
	{
		PxU16				articulationBLength;	//!< The total lengh of articulation B in multiples of 16 bytes.
		PxU32				bodyBDataIndex;			//!< Body B's index into the SolverBodyData array
	};

	PxU16					writeBackLengthOver4;	//!< writeBackLength/4, max writeback length is 256K, allows PxSolverConstraintDesc to fit in 32 bytes
	PxU16					constraintLengthOver16;	//!< constraintLength/16, max constraint length is 1MB, allows PxSolverConstraintDesc to fit in 32 bytes

	PxU8*					constraint;				//!< Pointer to the constraint rows to be solved
	void*					writeBack;				//!< Pointer to the writeback structure results for this given constraint are to be written to
};

struct PxSolverConstraintPrepDescBase
{
	enum BodyState
	{
		eDYNAMIC_BODY = 1 << 0,
		eSTATIC_BODY = 1 << 1,
		eKINEMATIC_BODY = 1 << 2,
		eARTICULATION = 1 << 3
	};

	PxConstraintInvMassScale mInvMassScales;	//!< In: The local mass scaling for this pair.

	PxSolverConstraintDesc* desc;				//!< Output: The PxSolverConstraintDesc filled in by contact prep

	const PxSolverBody* body0;					//!< In: The first body. Stores velocity information. Unused unless contact involves articulations.
	const PxSolverBody* body1;					//!< In: The second body. Stores velocity information. Unused unless contact involves articulations.

	const PxSolverBodyData* data0;				//!< In: The first PxSolverBodyData. Stores mass and miscellaneous information for the first body. 
	const PxSolverBodyData* data1;				//!< In: The second PxSolverBodyData. Stores mass and miscellaneous information for the second body

	PxTransform bodyFrame0;						//!< In: The world-space transform of the first body.
	PxTransform bodyFrame1;						//!< In: The world-space transform of the second body.

	BodyState bodyState0;						//!< In: Defines what kind of actor the first body is
	BodyState bodyState1;						//!< In: Defines what kind of actor the second body is

};

struct PxSolverConstraintPrepDesc : public PxSolverConstraintPrepDescBase
{
	PX_ALIGN(16, Px1DConstraint* rows);			//!< The start of the constraint rows
	PxU32 numRows;								//!< The number of rows

	PxReal linBreakForce, angBreakForce;		//!< Break forces
	PxReal minResponseThreshold;				//!< The minimum response threshold
	void* writeback;							//!< Pointer to constraint writeback structure. Reports back joint breaking. If not required, set to NULL.
	bool disablePreprocessing;					//!< Disable joint pre-processing. Pre-processing can improve stability but under certain circumstances, e.g. when some invInertia rows are zero/almost zero, can cause instabilities.	
	bool improvedSlerp;							//!< Use improved slerp model
	bool driveLimitsAreForces;					//!< Indicates whether drive limits are forces

	PxVec3 body0WorldOffset;					//!< Body0 world offset
};


struct PxSolverContactDesc : public PxSolverConstraintPrepDescBase
{

	PX_ALIGN(16, Sc::ShapeInteraction* shapeInteraction); //!< Pointer to share interaction. Used for force threshold reports in solver. Set to NULL if using immediate mode.
	Gu::ContactPoint* contacts;				//!< The start of the contacts for this pair
	PxU32 numContacts;						//!< The total number of contacs this pair references.

	bool hasMaxImpulse;						//!< Defines whether this pairs has maxImpulses clamping enabled
	bool disableStrongFriction;				//!< Defines whether this pair disables strong friction (sticky friction correlation)
	bool hasForceThresholds;				//!< Defines whether this pair requires force thresholds	

	PxReal restDistance;					//!< A distance at which the solver should aim to hold the bodies separated. Default is 0
	PxReal maxCCDSeparation;				//!< A distance used to configure speculative CCD behavior. Default is PX_MAX_F32. Set internally in PhysX for bodies with eENABLE_SPECULATIVE_CCD on. Do not set directly!

	PxU8* frictionPtr;						//!< InOut: Friction patch correlation data. Set each frame by solver. Can be retained for improved behaviour or discarded each frame.
	PxU8 frictionCount;						//!< The total number of friction patches in this pair

	PxReal* contactForces;					//!< Out: A buffer for the solver to write applied contact forces to.

	PxU32 startFrictionPatchIndex;			//!< Start index of friction patch in the correlation buffer. Set by friction correlation
	PxU32 numFrictionPatches;				//!< Total number of friction patches in this pair. Set by friction correlation

	PxU32 startContactPatchIndex;			//!< The start index of this pair's contact patches in the correlation buffer. For internal use only
	PxU16 numContactPatches;				//!< Total number of contact patches.
	PxU16 axisConstraintCount;				//!< Axis constraint count. Defines how many constraint rows this pair has produced. Useful for statistical purposes.

	PxU8 pad[16 - sizeof(void*)];
};

class PxConstraintAllocator
{
public:
	/**
	\brief Allocates constraint data. It is the application's responsibility to release this memory after PxSolveConstraints has completed.
	\param[in] byteSize Allocation size in bytes
	\return the allocated memory. This address must be 16-byte aligned.
	*/
	virtual PxU8* reserveConstraintData(const PxU32 byteSize) = 0;
	/**
	\brief Allocates friction data. Friction data can be retained by the application for a given pair and provided as an input to PxSolverContactDesc to improve simulation stability.
	It is the application's responsibility to release this memory. If this memory is released, the application should ensure it does not pass pointers to this memory to PxSolverContactDesc.
	\param[in] byteSize Allocation size in bytes
	\return the allocated memory. This address must be 4-byte aligned.
	*/
	virtual PxU8* reserveFrictionData(const PxU32 byteSize) = 0;

	virtual ~PxConstraintAllocator() {}
};

#if !PX_DOXYGEN
}
#endif

#if PX_VC
#pragma warning(pop)
#endif

#endif

