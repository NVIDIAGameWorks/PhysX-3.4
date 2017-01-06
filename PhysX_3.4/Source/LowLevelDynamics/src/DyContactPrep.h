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


#ifndef DY_CONTACTPREP_H
#define DY_CONTACTPREP_H

#include "DySolverConstraintDesc.h"
#include "PxSceneDesc.h"
#include "DySolverContact4.h"


namespace physx
{

struct PxcNpWorkUnit;
class PxsConstraintBlockManager;
struct PxsContactManagerOutput;
struct PxSolverBody;
struct PxSolverBodyData;
struct PxSolverConstraintDesc;

namespace Dy
{
	class ThreadContext;
	struct CorrelationBuffer;

#define CREATE_FINALIZE_SOLVER_CONTACT_METHOD_ARGS			\
	PxSolverContactDesc& contactDesc,						\
	PxsContactManagerOutput& output,						\
	ThreadContext& threadContext,							\
	const PxReal invDtF32,									\
	PxReal bounceThresholdF32,								\
	PxReal frictionOffsetThreshold,							\
	PxReal	correlationDistance,							\
	PxConstraintAllocator& constraintAllocator				

#define CREATE_FINALIZE_SOVLER_CONTACT_METHOD_ARGS_4									\
								 PxsContactManagerOutput** outputs,						\
								 ThreadContext& threadContext,							\
								 PxSolverContactDesc* blockDescs,						\
								 const PxReal invDtF32,									\
								 PxReal bounceThresholdF32,								\
								 PxReal	frictionThresholdF32,							\
								 PxReal	correlationDistanceF32,							\
								 PxConstraintAllocator& constraintAllocator				

	
/*!
Method prototype for create finalize solver contact
*/

typedef	bool (*PxcCreateFinalizeSolverContactMethod)(CREATE_FINALIZE_SOLVER_CONTACT_METHOD_ARGS);

extern PxcCreateFinalizeSolverContactMethod createFinalizeMethods[3];

typedef	SolverConstraintPrepState::Enum (*PxcCreateFinalizeSolverContactMethod4)(CREATE_FINALIZE_SOVLER_CONTACT_METHOD_ARGS_4);

extern PxcCreateFinalizeSolverContactMethod4 createFinalizeMethods4[3];


bool createFinalizeSolverContacts(	PxSolverContactDesc& contactDesc,
									PxsContactManagerOutput& output,
									ThreadContext& threadContext,
									const PxReal invDtF32,
									PxReal bounceThresholdF32,
									PxReal frictionOffsetThreshold,
									PxReal correlationDistance,
									PxConstraintAllocator& constraintAllocator);

bool createFinalizeSolverContacts(	PxSolverContactDesc& contactDesc,
									CorrelationBuffer& c,
									const PxReal invDtF32,
									PxReal bounceThresholdF32,
									PxReal frictionOffsetThreshold,
									PxReal correlationDistance,
									PxConstraintAllocator& constraintAllocator);

SolverConstraintPrepState::Enum createFinalizeSolverContacts4(	PxsContactManagerOutput** outputs,
																 ThreadContext& threadContext,
																 PxSolverContactDesc* blockDescs,
																 const PxReal invDtF32,
																 PxReal bounceThresholdF32,
																 PxReal frictionOffsetThreshold,
																 PxReal correlationDistance,
																 PxConstraintAllocator& constraintAllocator);

SolverConstraintPrepState::Enum createFinalizeSolverContacts4(	Dy::CorrelationBuffer& c,
																PxSolverContactDesc* blockDescs,
																const PxReal invDtF32,
																PxReal bounceThresholdF32,
																PxReal	frictionOffsetThreshold,
																PxReal correlationDistance,
																PxConstraintAllocator& constraintAllocator);



bool createFinalizeSolverContactsCoulomb1D(PxSolverContactDesc& contactDesc,
											 PxsContactManagerOutput& output,
											 ThreadContext& threadContext,
											 const PxReal invDtF32,
											 PxReal bounceThresholdF32,
											 PxReal frictionOffsetThreshold,
											 PxReal correlationDistance,
											 PxConstraintAllocator& constraintAllocator);

bool createFinalizeSolverContactsCoulomb2D(PxSolverContactDesc& contactDesc,
											PxsContactManagerOutput& output,
											ThreadContext& threadContext,
											const PxReal invDtF32,
											PxReal bounceThresholdF32,
											PxReal frictionOffsetThreshold,
											PxReal correlationDistance,
											PxConstraintAllocator& constraintAllocator);


SolverConstraintPrepState::Enum createFinalizeSolverContacts4Coulomb1D(	PxsContactManagerOutput** outputs,
																		ThreadContext& threadContext,
																		 PxSolverContactDesc* blockDescs,
																		 const PxReal invDtF32,
																		 PxReal bounceThresholdF32,
																		 PxReal frictionOffsetThreshold,
																		 PxReal correlationDistance,
																		 PxConstraintAllocator& constraintAllocator);

SolverConstraintPrepState::Enum createFinalizeSolverContacts4Coulomb2D(PxsContactManagerOutput** outputs,
																		ThreadContext& threadContext,
																		PxSolverContactDesc* blockDescs,
																		const PxReal invDtF32,
																		PxReal bounceThresholdF32,
																		PxReal frictionOffsetThreshold,
																		PxReal correlationDistance,
																		PxConstraintAllocator& constraintAllocator);


PxU32 getContactManagerConstraintDesc(const PxsContactManagerOutput& cmOutput, const PxsContactManager& cm, PxSolverConstraintDesc& desc);

}

}

#endif //DY_CONTACTPREP_H
